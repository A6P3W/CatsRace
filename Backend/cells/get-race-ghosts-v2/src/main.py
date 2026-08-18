import logging

import functions_framework

from flow import main
from common.race_ranking_repository.validation import RequestValidationError


@functions_framework.http
def cell_entry_point(request):
    if request.method != "POST":
        return {"status": "error", "message": "POST method is required"}, 400
    body = request.get_json(silent=True)
    if not isinstance(body, dict):
        return {"status": "error", "message": "JSON body must be an object"}, 400
    try:
        return {"status": "success", "data": main(body)}, 200
    except RequestValidationError as error:
        return {"status": "error", "message": str(error)}, 400
    except Exception:
        logging.exception("get-race-ghosts-v2 failed")
        return {"status": "error", "message": "Internal server error"}, 500
