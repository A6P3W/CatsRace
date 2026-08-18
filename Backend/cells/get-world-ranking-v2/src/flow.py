import os

from google.cloud import firestore

from common.race_ranking_repository.repository import RaceRankingRepository
from common.race_ranking_repository.validation import validate_ranking_request


_repository: RaceRankingRepository | None = None


def get_repository() -> RaceRankingRepository:
    global _repository
    if _repository is None:
        project_id = (
            os.environ.get("GOOGLE_CLOUD_PROJECT")
            or os.environ.get("PROJECT_ID")
            or "catsrace"
        )
        _repository = RaceRankingRepository(firestore.Client(project=project_id))
    return _repository


def main(
    body: dict, repository: RaceRankingRepository | None = None
) -> dict:
    map_id, map_version, identity_keys = validate_ranking_request(body)
    ranking = (repository or get_repository()).get_world_ranking(
        map_id, map_version, identity_keys
    )
    return {
        "map_id": map_id,
        "map_version": map_version,
        **ranking,
    }
