import traceback
from common.firestore.entry import FirestoreHandler


def main(body: dict) -> dict:
    try:

        return 0
    except Exception as e:
        error = traceback.format_exc()
        print(f"Error in main: {error}")
        raise e


if __name__ == "__main__":
    print(main({"0":0}))
