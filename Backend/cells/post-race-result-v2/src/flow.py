import os

from google.cloud import firestore

from common.race_ranking_repository.repository import RaceRankingRepository
from common.race_ranking_repository.validation import validate_post_results_request


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
    map_id, map_version, results = validate_post_results_request(body)
    posted = (repository or get_repository()).post_results(
        map_id, map_version, results
    )
    return {
        "map_id": map_id,
        "map_version": map_version,
        "results": [entry.to_dict() for entry in posted],
    }
