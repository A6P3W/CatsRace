from __future__ import annotations

from dataclasses import dataclass
from typing import Any


SCHEMA_VERSION = 1
GHOST_SCHEMA_VERSION = 1
MAX_GHOST_RECORDED_SECONDS = 240.0
COLLECTION_NAME = "race_leaderboards_v2"


@dataclass(frozen=True)
class RaceResult:
    user_id: str
    id_type: str
    identity_key: str
    player_name: str
    score: float
    ghost_schema_version: int
    ghost_recorded_seconds: float
    is_ghost_partial: bool
    ghost_data: str

    def to_document(self, map_id: str, map_version: int) -> dict[str, Any]:
        return {
            "schema_version": SCHEMA_VERSION,
            "map_id": map_id,
            "map_version": map_version,
            "user_id": self.user_id,
            "id_type": self.id_type,
            "identity_key": self.identity_key,
            "player_name": self.player_name,
            "score": self.score,
            "ghost_schema_version": self.ghost_schema_version,
            "ghost_recorded_seconds": self.ghost_recorded_seconds,
            "is_ghost_partial": self.is_ghost_partial,
            "ghost_data": self.ghost_data,
            "is_valid": True,
        }


@dataclass(frozen=True)
class PostRaceResultEntry:
    identity_key: str
    updated_best: bool
    best_score: float

    def to_dict(self) -> dict[str, Any]:
        return {
            "identity_key": self.identity_key,
            "updated_best": self.updated_best,
            "best_score": self.best_score,
        }


@dataclass(frozen=True)
class WorldRankingEntry:
    rank: int
    user_id: str
    identity_key: str
    player_name: str
    score: float
    is_self: bool = False

    def to_dict(self, include_is_self: bool = False) -> dict[str, Any]:
        value = {
            "rank": self.rank,
            "user_id": self.user_id,
            "identity_key": self.identity_key,
            "player_name": self.player_name,
            "score": self.score,
        }
        if include_is_self:
            value["is_self"] = self.is_self
        return value


@dataclass(frozen=True)
class RaceGhostData:
    slot: str
    user_id: str
    identity_key: str
    player_name: str
    score: float
    ghost_schema_version: int
    ghost_recorded_seconds: float
    is_ghost_partial: bool
    ghost_data: str

    def to_dict(self) -> dict[str, Any]:
        return {
            "slot": self.slot,
            "user_id": self.user_id,
            "identity_key": self.identity_key,
            "player_name": self.player_name,
            "score": self.score,
            "ghost_schema_version": self.ghost_schema_version,
            "ghost_recorded_seconds": self.ghost_recorded_seconds,
            "is_ghost_partial": self.is_ghost_partial,
            "ghost_data": self.ghost_data,
        }
