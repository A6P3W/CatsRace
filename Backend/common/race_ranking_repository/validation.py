from __future__ import annotations

import math
from typing import Any

from .models import MAX_GHOST_RECORDED_SECONDS, SCHEMA_VERSION, RaceResult


class RequestValidationError(ValueError):
    pass


def _require_object(body: Any) -> dict[str, Any]:
    if not isinstance(body, dict):
        raise RequestValidationError("JSON body must be an object")
    return body


def _require_string(value: Any, field_name: str) -> str:
    if not isinstance(value, str) or not value:
        raise RequestValidationError(f"{field_name} must be a non-empty string")
    return value


def _require_int(value: Any, field_name: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise RequestValidationError(f"{field_name} must be an integer")
    return value


def _require_number(value: Any, field_name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise RequestValidationError(f"{field_name} must be a number")
    number = float(value)
    if not math.isfinite(number):
        raise RequestValidationError(f"{field_name} must be finite")
    return number


def validate_common_request(body: Any) -> tuple[str, int]:
    request = _require_object(body)
    schema_version = _require_int(request.get("schema_version"), "schema_version")
    if schema_version != SCHEMA_VERSION:
        raise RequestValidationError(
            f"schema_version must be {SCHEMA_VERSION}"
        )
    map_id = _require_string(request.get("map_id"), "map_id")
    map_version = _require_int(request.get("map_version"), "map_version")
    return map_id, map_version


def validate_post_results_request(
    body: Any,
) -> tuple[str, int, list[RaceResult]]:
    request = _require_object(body)
    map_id, map_version = validate_common_request(request)
    raw_results = request.get("results")
    if not isinstance(raw_results, list) or not 1 <= len(raw_results) <= 4:
        raise RequestValidationError("results must contain 1 to 4 objects")

    results: list[RaceResult] = []
    identity_keys: set[str] = set()
    for index, raw_result in enumerate(raw_results):
        if not isinstance(raw_result, dict):
            raise RequestValidationError(f"results[{index}] must be an object")
        prefix = f"results[{index}]"
        user_id = _require_string(raw_result.get("user_id"), f"{prefix}.user_id")
        id_type = _require_string(raw_result.get("id_type"), f"{prefix}.id_type")
        if id_type not in {"DeviceId", "PlayerName"}:
            raise RequestValidationError(
                f'{prefix}.id_type must be "DeviceId" or "PlayerName"'
            )
        identity_key = _require_string(
            raw_result.get("identity_key"), f"{prefix}.identity_key"
        )
        expected_identity_key = f"{id_type}:{user_id}"
        if identity_key != expected_identity_key:
            raise RequestValidationError(
                f"{prefix}.identity_key must equal {expected_identity_key!r}"
            )
        if identity_key in identity_keys:
            raise RequestValidationError("results must not repeat identity_key")
        identity_keys.add(identity_key)

        score = _require_number(raw_result.get("score"), f"{prefix}.score")
        if score <= 0:
            raise RequestValidationError(f"{prefix}.score must be greater than 0")
        ghost_schema_version = _require_int(
            raw_result.get("ghost_schema_version"),
            f"{prefix}.ghost_schema_version",
        )
        ghost_recorded_seconds = _require_number(
            raw_result.get("ghost_recorded_seconds"),
            f"{prefix}.ghost_recorded_seconds",
        )
        if not 0 < ghost_recorded_seconds <= MAX_GHOST_RECORDED_SECONDS:
            raise RequestValidationError(
                f"{prefix}.ghost_recorded_seconds must be greater than 0 "
                f"and at most {MAX_GHOST_RECORDED_SECONDS:g}"
            )
        is_ghost_partial = raw_result.get("is_ghost_partial")
        if not isinstance(is_ghost_partial, bool):
            raise RequestValidationError(
                f"{prefix}.is_ghost_partial must be a boolean"
            )

        results.append(
            RaceResult(
                user_id=user_id,
                id_type=id_type,
                identity_key=identity_key,
                player_name=_require_string(
                    raw_result.get("player_name"), f"{prefix}.player_name"
                ),
                score=score,
                ghost_schema_version=ghost_schema_version,
                ghost_recorded_seconds=ghost_recorded_seconds,
                is_ghost_partial=is_ghost_partial,
                ghost_data=_require_string(
                    raw_result.get("ghost_data"), f"{prefix}.ghost_data"
                ),
            )
        )
    return map_id, map_version, results


def validate_ranking_request(body: Any) -> tuple[str, int, list[str]]:
    request = _require_object(body)
    map_id, map_version = validate_common_request(request)
    raw_identity_keys = request.get("identity_keys")
    if not isinstance(raw_identity_keys, list) or not 1 <= len(raw_identity_keys) <= 4:
        raise RequestValidationError("identity_keys must contain 1 to 4 strings")
    identity_keys = [
        _require_string(value, f"identity_keys[{index}]")
        for index, value in enumerate(raw_identity_keys)
    ]
    if len(set(identity_keys)) != len(identity_keys):
        raise RequestValidationError("identity_keys must not contain duplicates")
    return map_id, map_version, identity_keys
