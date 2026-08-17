import functions_framework

from flow import main


@functions_framework.http
def cell_entry_point(request):
    if request.method != "POST":
        return {"status": "error", "message": "POST method is required"}, 400
    body = request.get_json(silent=True)
    if not isinstance(body, dict):
        return {"status": "error", "message": "JSON body must be an object"}, 400
    return {"status": "success", "data": main(body)}, 200
