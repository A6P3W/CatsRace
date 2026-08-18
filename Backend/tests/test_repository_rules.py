import unittest

from common.race_ranking_repository.repository import (
    document_id,
    select_ghost_indices,
    should_update_best,
)


class RepositoryRulesTest(unittest.TestCase):
    def test_document_id_uses_map_and_identity(self):
        self.assertEqual(
            document_id("b", "DeviceId:DEV-001"),
            "b_DeviceId:DEV-001",
        )

    def test_newer_map_version_always_replaces_record(self):
        stored = {"map_version": 1, "score": 50.0}
        self.assertTrue(should_update_best(stored, 2, 80.0))

    def test_same_map_version_only_accepts_faster_score(self):
        stored = {"map_version": 2, "score": 50.0}
        self.assertTrue(should_update_best(stored, 2, 49.9))
        self.assertFalse(should_update_best(stored, 2, 50.0))
        self.assertFalse(should_update_best(stored, 2, 50.1))

    def test_older_map_version_never_replaces_record(self):
        stored = {"map_version": 2, "score": 50.0}
        self.assertFalse(should_update_best(stored, 1, 40.0))

    def test_ghost_selection_is_unique_and_capped_at_four(self):
        for total in range(0, 10):
            selected = select_ghost_indices(total)
            indices = [index for _, index in selected]
            self.assertEqual(len(indices), min(total, 4))
            self.assertEqual(len(indices), len(set(indices)))
            self.assertTrue(all(0 <= index < total for index in indices))

    def test_ghost_selection_uses_requested_percentile_ranks(self):
        self.assertEqual(
            select_ghost_indices(10),
            [
                ("world_first", 0),
                ("one_third", 3),
                ("two_thirds", 6),
                ("normal", 8),
            ],
        )
        self.assertEqual(
            select_ghost_indices(5),
            [
                ("world_first", 0),
                ("one_third", 1),
                ("two_thirds", 3),
                ("normal", 4),
            ],
        )


if __name__ == "__main__":
    unittest.main()
