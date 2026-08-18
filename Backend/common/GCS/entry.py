from google.cloud import storage
import json

# グローバルでは定義のみ行い、None で初期化する
storage_client = None
BUCKET = None

def _get_or_init_bucket(bucket_name: str = None):
    """バケットオブジェクトを必要に応じて初期化する内部関数"""
    global storage_client, BUCKET
    if BUCKET is None:
        if not bucket_name:
            raise ValueError("bucket_name is required when initializing the bucket")
        # 関数が呼ばれた時（実行時）に初めて実行される
        storage_client = storage.Client()
        # get_bucket ではなく bucket() を使うことで、
        # メタデータ取得のための余計な権限チェック(storage.buckets.get)を回避できる場合があります
        BUCKET = storage_client.bucket(bucket_name)
    return BUCKET

def blob_download_as_json(parent_dir, filename):
    bucket = _get_or_init_bucket()
    blob = bucket.blob(f"{parent_dir}/{filename}")
    try:
        data = blob.download_as_text()
        return json.loads(data)
    except Exception:
        raise

def delete_folder(folder_path):
    bucket = _get_or_init_bucket()
    prefix = folder_path if folder_path.endswith('/') else folder_path + '/'
    blobs = list(bucket.list_blobs(prefix=prefix))
    if len(blobs) == 0:
        print(f"{folder_path} は存在しないか、既に空です")
        return
    for blob in blobs:
        blob.delete()
        print(f"{blob.name} を削除しました")
    print(f"{folder_path} フォルダを削除しました（{len(blobs)} 個のファイル）")

def blob_upload_from_json(parent_dir, data_dict, filename):
    bucket = _get_or_init_bucket()
    blob = bucket.blob(f"{parent_dir}/{filename}")
    data = json.dumps(data_dict, ensure_ascii=False)
    blob.upload_from_string(data=data)
    print(f"{parent_dir}/{filename} をアップロードしました")


import json
import logging
from google.cloud import storage

class GCSHandler:
    def __init__(self, bucket_name: str):
        # 実行時に初めて初期化される
        self.client = storage.Client()
        self.bucket = self.client.bucket(bucket_name)

    def download_json(self, blob_path: str) -> dict:
        blob = self.bucket.blob(blob_path)
        try:
            content = blob.download_as_text()
            return json.loads(content)
        except Exception as e:
            logging.error(f"Failed to download {blob_path}: {e}")
            raise

    def upload_json(self, blob_path: str, data: dict):
        blob = self.bucket.blob(blob_path)
        payload = json.dumps(data, ensure_ascii=False)
        blob.upload_from_string(payload, content_type='application/json')
        
if __name__ == "__main__":
    # ローカル実行時のみ動作
    sample_data = {
        "last_action": 2,
        "base_balance": 1000,
        "quote_balance": 5000
    }
    bucket = GCSHandler("trade_cell_a433")
    bucket.upload_json("test/sample.json", sample_data)
    sample = bucket.download_json("test/sample.json")
    print(sample)