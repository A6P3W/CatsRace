import unittest

from common.race_ranking_repository.validation import (
    RequestValidationError,
    validate_common_request,
    validate_post_results_request,
    validate_ranking_request,
)


def valid_result(**overrides):
    value = {
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
    value.update(overrides)
    return value


class ValidationTest(unittest.TestCase):
    def test_common_fields_are_required(self):
        self.assertEqual(
            validate_common_request(
                {"schema_version": 1, "map_id": "b", "map_version": 1}
            ),
            ("b", 1),
        )
        with self.assertRaises(RequestValidationError):
            validate_common_request(
                {"schema_version": 1, "map_id": "b"}
            )

    def test_identity_key_must_match_type_and_user(self):
        request = {
            "schema_version": 1,
            "map_id": "b",
            "map_version": 1,
            "results": [valid_result(identity_key="PlayerName:DEV-001")],
        }
        with self.assertRaises(RequestValidationError):
            validate_post_results_request(request)

    def test_partial_ghost_at_240_seconds_is_accepted(self):
        request = {
            "schema_version": 1,
            "map_id": "b",
            "map_version": 1,
            "results": [
                valid_result(
                    ghost_recorded_seconds=240,
                    is_ghost_partial=True,
                )
            ],
        }
        _, _, results = validate_post_results_request(request)
        self.assertTrue(results[0].is_ghost_partial)
        self.assertEqual(results[0].ghost_recorded_seconds, 240.0)

    def test_ghost_recording_over_240_seconds_is_rejected(self):
        request = {
            "schema_version": 1,
            "map_id": "b",
            "map_version": 1,
            "results": [valid_result(ghost_recorded_seconds=240.001)],
        }
        with self.assertRaises(RequestValidationError):
            validate_post_results_request(request)

    def test_ranking_request_accepts_one_to_four_unique_identities(self):
        request = {
            "schema_version": 1,
            "map_id": "b",
            "map_version": 1,
            "identity_keys": ["DeviceId:A", "PlayerName:A"],
        }
        self.assertEqual(
            validate_ranking_request(request),
            ("b", 1, ["DeviceId:A", "PlayerName:A"]),
        )
        request["identity_keys"] = ["DeviceId:A", "DeviceId:A"]
        with self.assertRaises(RequestValidationError):
            validate_ranking_request(request)


if __name__ == "__main__":
    unittest.main()
