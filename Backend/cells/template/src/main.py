import traceback

import functions_framework
from flow import main


@functions_framework.http
def cell_entry_point(request):
    try:
        print("--- HTTP Request Received ---")

        if request.method != "POST":
            return (
                {
                    "status": "error",
                    "message": "POST method is required",
                },
                400,
            )

        body = request.get_json(silent=True)
        if body is None:
            return (
                {
                    "status": "error",
                    "message": "JSON body is required",
                },
                400,
            )
        if not isinstance(body, dict):
            return (
                {
                    "status": "error",
                    "message": "JSON body must be an object",
                },
                400,
            )


        result = main(body)

        return (
            {
                "status": "success",
                "data": result
            },
            200
        )

    except Exception:
        error_text = traceback.format_exc()
        print(error_text)

        return (
            {
                "status": "error",
                "message": error_text,
            },
            500,
        )
