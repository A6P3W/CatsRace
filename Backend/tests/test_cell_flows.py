import importlib.util
import sys
import unittest
from pathlib import Path

from common.race_ranking_repository.models import (
    PostRaceResultEntry,
    RaceGhostData,
)


BACKEND_ROOT = Path(__file__).resolve().parents[1]


def load_flow(cell_name: str):
    module_name = f"test_{cell_name.replace('-', '_')}_flow"
    path = BACKEND_ROOT / "cells" / cell_name / "src" / "flow.py"
    spec = importlib.util.spec_from_file_location(module_name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class FakeRepository:
    def __init__(self):
        self.call = None

    def post_results(self, map_id, map_version, results):
        self.call = (map_id, map_version, results)
        return [
            PostRaceResultEntry(
                identity_key=results[0].identity_key,
                updated_best=True,
                best_score=results[0].score,
            )
        ]

    def get_world_ranking(self, map_id, map_version, identity_keys):
        self.call = (map_id, map_version, identity_keys)
        return {
            "top": [],
            "rankings_by_identity": {
                identity_keys[0]: {"self_rank": None, "around_self": []}
            },
        }

    def get_race_ghosts(self, map_id, map_version):
        self.call = (map_id, map_version)
        return [
            RaceGhostData(
                slot="world_first",
                user_id="DEV-001",
                identity_key="DeviceId:DEV-001",
                player_name="PlayerA",
                score=58.452,
                ghost_schema_version=1,
                ghost_recorded_seconds=58.452,
                is_ghost_partial=False,
                ghost_data="GHOST",
            )
        ]


class CellFlowTest(unittest.TestCase):
    def test_post_result_flow_returns_batch_contract(self):
        flow = load_flow("post-race-result-v2")
        repository = FakeRepository()
        response = flow.main(
            {
                "schema_version": 1,
                "map_id": "b",
                "map_version": 1,
                "results": [
                    {
                        "user_id": "DEV-001",
                        "id_type": "DeviceId",
                        "identity_key": "DeviceId:DEV-001",
                        "player_name": "PlayerA",
                        "score": 58.452,
                        "ghost_schema_version": 1,
                        "ghost_recorded_seconds": 58.452,
                        "is_ghost_partial": False,
                        "ghost_data": "GHOST",
                    }
                ],
            },
            repository,
        )
        self.assertEqual(response["map_id"], "b")
        self.assertTrue(response["results"][0]["updated_best"])

    def test_ranking_flow_returns_batch_contract(self):
        flow = load_flow("get-world-ranking-v2")
        repository = FakeRepository()
        response = flow.main(
            {
                "schema_version": 1,
                "map_id": "b",
                "map_version": 1,
                "identity_keys": ["DeviceId:DEV-001"],
            },
            repository,
        )
        self.assertEqual(response["top"], [])
        self.assertIn("DeviceId:DEV-001", response["rankings_by_identity"])

    def test_ghost_flow_preserves_ghost_metadata(self):
        flow = load_flow("get-race-ghosts-v2")
        repository = FakeRepository()
        response = flow.main(
            {"schema_version": 1, "map_id": "b", "map_version": 1},
            repository,
        )
        ghost = response["ghosts"][0]
        self.assertEqual(ghost["slot"], "world_first")
        self.assertEqual(ghost["ghost_recorded_seconds"], 58.452)
        self.assertFalse(ghost["is_ghost_partial"])


if __name__ == "__main__":
    unittest.main()
