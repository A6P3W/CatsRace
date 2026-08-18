import unittest

from common.race_ranking_repository.repository import RaceRankingRepository


def document(index: int, *, partial: bool = False):
    identity_key = f"DeviceId:DEV-{index:03d}"
    return {
        "user_id": f"DEV-{index:03d}",
        "identity_key": identity_key,
        "player_name": f"Player{index}",
        "score": float(index),
        "ghost_schema_version": 1,
        "ghost_recorded_seconds": min(float(index), 240.0),
        "is_ghost_partial": partial,
        "ghost_data": f"GHOST-{index}",
    }


class StaticRepository(RaceRankingRepository):
    def __init__(self, documents):
        self._documents = documents

    def _ranking_documents(self, map_id, map_version):
        return list(self._documents)


class RankingAndGhostLogicTest(unittest.TestCase):
    def test_top_five_and_self_windows_for_top_middle_bottom(self):
        repository = StaticRepository([document(index) for index in range(1, 8)])
        result = repository.get_world_ranking(
            "b",
            1,
            ["DeviceId:DEV-001", "DeviceId:DEV-004", "DeviceId:DEV-007"],
        )
        self.assertEqual([entry["rank"] for entry in result["top"]], [1, 2, 3, 4, 5])
        self.assertEqual(
            [entry["rank"] for entry in result["rankings_by_identity"]["DeviceId:DEV-001"]["around_self"]],
            [1, 2, 3],
        )
        self.assertEqual(
            [entry["rank"] for entry in result["rankings_by_identity"]["DeviceId:DEV-004"]["around_self"]],
            [2, 3, 4, 5, 6],
        )
        self.assertEqual(
            [entry["rank"] for entry in result["rankings_by_identity"]["DeviceId:DEV-007"]["around_self"]],
            [5, 6, 7],
        )

    def test_unregistered_self_returns_null_rank(self):
        repository = StaticRepository([document(1), document(2)])
        result = repository.get_world_ranking("b", 1, ["DeviceId:MISSING"])
        missing = result["rankings_by_identity"]["DeviceId:MISSING"]
        self.assertIsNone(missing["self_rank"])
        self.assertEqual(missing["around_self"], [])

    def test_zero_to_four_ghosts_and_partial_ghost(self):
        for total in range(5):
            documents = [
                document(index, partial=index == total)
                for index in range(1, total + 1)
            ]
            ghosts = StaticRepository(documents).get_race_ghosts("b", 1)
            self.assertEqual(len(ghosts), total)
            self.assertEqual(len({ghost.identity_key for ghost in ghosts}), total)
            if total:
                partial = next(
                    ghost for ghost in ghosts if ghost.identity_key == f"DeviceId:DEV-{total:03d}"
                )
                self.assertTrue(partial.is_ghost_partial)


if __name__ == "__main__":
    unittest.main()
