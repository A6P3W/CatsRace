from __future__ import annotations

from collections.abc import Iterable
from typing import Any

from google.cloud import firestore
from google.cloud.firestore_v1.base_query import FieldFilter

from .models import (
    COLLECTION_NAME,
    PostRaceResultEntry,
    RaceGhostData,
    RaceResult,
    WorldRankingEntry,
)


def document_id(map_id: str, identity_key: str) -> str:
    return f"{map_id}_{identity_key}"


def should_update_best(
    stored: dict[str, Any] | None, map_version: int, new_score: float
) -> bool:
    if stored is None:
        return True
    stored_map_version = int(stored["map_version"])
    if map_version > stored_map_version:
        return True
    if map_version < stored_map_version:
        return False
    return new_score < float(stored["score"])


def select_ghost_indices(total: int) -> list[tuple[str, int]]:
    if total <= 0:
        return []

    def percentile_index(percent: int) -> int:
        return ((total * percent + 99) // 100) - 1

    # Keep the existing wire-level slot names for client compatibility.
    candidates = (
        ("world_first", 0),
        ("one_third", percentile_index(40)),
        ("two_thirds", percentile_index(70)),
        ("normal", percentile_index(90)),
    )
    selected: list[tuple[str, int]] = []
    used: set[int] = set()
    for slot, preferred in candidates:
        index = preferred
        if index in used:
            index = next((value for value in range(total) if value not in used), -1)
        if index < 0:
            break
        used.add(index)
        selected.append((slot, index))
    return selected


class RaceRankingRepository:
    def __init__(self, client: firestore.Client):
        self._client = client
        self._collection = client.collection(COLLECTION_NAME)

    def post_results(
        self, map_id: str, map_version: int, results: list[RaceResult]
    ) -> list[PostRaceResultEntry]:
        references = [
            self._collection.document(document_id(map_id, result.identity_key))
            for result in results
        ]
        transaction = self._client.transaction()

        @firestore.transactional
        def update_in_transaction(current_transaction):
            snapshots = [reference.get(transaction=current_transaction) for reference in references]
            stored_values = [
                snapshot.to_dict() if snapshot.exists else None for snapshot in snapshots
            ]
            response: list[PostRaceResultEntry] = []

            for reference, stored, result in zip(
                references, stored_values, results, strict=True
            ):
                updated_best = should_update_best(stored, map_version, result.score)
                if updated_best:
                    value = result.to_document(map_id, map_version)
                    value["best_updated_at"] = firestore.SERVER_TIMESTAMP
                    current_transaction.set(reference, value)
                    best_score = result.score
                elif stored is not None and map_version == int(stored["map_version"]):
                    if stored.get("player_name") != result.player_name:
                        current_transaction.update(
                            reference, {"player_name": result.player_name}
                        )
                    best_score = float(stored["score"])
                else:
                    best_score = float(stored["score"])
                response.append(
                    PostRaceResultEntry(
                        identity_key=result.identity_key,
                        updated_best=updated_best,
                        best_score=best_score,
                    )
                )
            return response

        return update_in_transaction(transaction)

    def _ranking_documents(self, map_id: str, map_version: int) -> list[dict[str, Any]]:
        query = self._collection
        query = query.where(filter=FieldFilter("map_id", "==", map_id))
        query = query.where(filter=FieldFilter("map_version", "==", map_version))
        query = query.where(filter=FieldFilter("is_valid", "==", True))
        query = query.order_by("score", direction=firestore.Query.ASCENDING)
        query = query.order_by("identity_key", direction=firestore.Query.ASCENDING)
        return [snapshot.to_dict() for snapshot in query.stream()]

    def get_world_ranking(
        self, map_id: str, map_version: int, identity_keys: Iterable[str]
    ) -> dict[str, Any]:
        documents = self._ranking_documents(map_id, map_version)
        ranked = [
            WorldRankingEntry(
                rank=index,
                user_id=str(value["user_id"]),
                identity_key=str(value["identity_key"]),
                player_name=str(value["player_name"]),
                score=float(value["score"]),
            )
            for index, value in enumerate(documents, start=1)
        ]
        top = [entry.to_dict() for entry in ranked[:5]]
        rankings_by_identity: dict[str, Any] = {}
        positions = {entry.identity_key: index for index, entry in enumerate(ranked)}
        for identity_key in identity_keys:
            position = positions.get(identity_key)
            if position is None:
                rankings_by_identity[identity_key] = {
                    "self_rank": None,
                    "around_self": [],
                }
                continue
            around = []
            for entry in ranked[max(0, position - 2) : position + 3]:
                self_entry = WorldRankingEntry(
                    rank=entry.rank,
                    user_id=entry.user_id,
                    identity_key=entry.identity_key,
                    player_name=entry.player_name,
                    score=entry.score,
                    is_self=entry.identity_key == identity_key,
                )
                around.append(self_entry.to_dict(include_is_self=True))
            rankings_by_identity[identity_key] = {
                "self_rank": ranked[position].rank,
                "around_self": around,
            }
        return {"top": top, "rankings_by_identity": rankings_by_identity}

    def get_race_ghosts(self, map_id: str, map_version: int) -> list[RaceGhostData]:
        documents = self._ranking_documents(map_id, map_version)
        ghosts: list[RaceGhostData] = []
        for slot, index in select_ghost_indices(len(documents)):
            value = documents[index]
            ghosts.append(
                RaceGhostData(
                    slot=slot,
                    user_id=str(value["user_id"]),
                    identity_key=str(value["identity_key"]),
                    player_name=str(value["player_name"]),
                    score=float(value["score"]),
                    ghost_schema_version=int(value["ghost_schema_version"]),
                    ghost_recorded_seconds=float(value["ghost_recorded_seconds"]),
                    is_ghost_partial=bool(value["is_ghost_partial"]),
                    ghost_data=str(value["ghost_data"]),
                )
            )
        return ghosts
