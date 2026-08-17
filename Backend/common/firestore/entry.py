import logging
from google.cloud import firestore

class FirestoreHandler:
    def __init__(self, project_id: str = None, database: str = "(default)"):
        """
        Firestore クライアントを初期化します。
        database 引数により、2024年以降推奨されているマルチデータベース構成にも対応可能です。
        """
        try:
            # 実行時に認証情報を読み込み、クライアントを生成
            self.client = firestore.Client(project=project_id, database=database)
        except Exception as e:
            logging.error(f"Firestore initialization failed: {e}")
            raise

    def download_json(self, collection_path: str, document_id: str) -> dict:
        """
        指定したドキュメントを取得し、辞書形式で返します。
        """
        doc_ref = self.client.collection(collection_path).document(document_id)
        try:
            doc = doc_ref.get()
            if doc.exists:
                return doc.to_dict()
            else:
                logging.warning(f"Document {document_id} not found in {collection_path}")
                return {}
        except Exception as e:
            logging.error(f"Failed to download document {document_id}: {e}")
            raise

    def upload_json(self, collection_path: str, document_id: str, data: dict, merge: bool = True):
        """
        指定したドキュメントにデータを書き込みます。
        merge=True にすることで、既存のフィールドを保持したまま更新（上書き回避）が可能です。
        """
        doc_ref = self.client.collection(collection_path).document(document_id)
        try:
            # set() はドキュメントが存在しない場合は新規作成、存在する場合は更新
            doc_ref.set(data, merge=merge)
            # ログ出力（必要に応じて）
            # logging.info(f"Successfully uploaded to {collection_path}/{document_id}")
        except Exception as e:
            logging.error(f"Failed to upload document {document_id}: {e}")
            raise

    def delete_document(self, collection_path: str, document_id: str):
        """
        ドキュメントを削除します。
        """
        doc_ref = self.client.collection(collection_path).document(document_id)
        try:
            doc_ref.delete()
        except Exception as e:
            logging.error(f"Failed to delete document {document_id}: {e}")
            raise
        
    def search_docs(
            self, 
            collection_path: str, 
            filters: list[tuple[str, str, object]] = None, 
            order_by: str = None, 
            descending: bool = False,
            limit: int = None,
            analyze: bool = False
        ):
            """
            汎用的なクエリ実行メソッド
            filters: [("field", "==", "value"), ...] の形式で指定
            """
            
            if not filters:
                return self._execute_single_query(collection_path, [], order_by, descending, limit, analyze)

            needs_chunking = any(
                    isinstance(v, list) and len(v) > 30 and op in ["in", "not_in", "array_contains_any"]
                for _, op, v in filters
            )
            
            if not needs_chunking:
                return self._execute_single_query(collection_path, filters, order_by, descending, limit, analyze)



    
            return self._execute_chunked_query(collection_path, filters, order_by, descending, limit, analyze)
    def _execute_single_query(
            self, 
            collection_path: str, 
            filters: list[tuple[str, str, object]] = None, 
            order_by: str = None, 
            descending: bool = False,
            limit: int = None,
            analyze: bool = False
        ):
            query = self.client.collection(collection_path)
            # 動的にフィルタを追加
            for field, op, value in filters:
                query = query.where(filter=firestore.FieldFilter(field, op, value))

            # ソートの追加
            if order_by:
                direction = firestore.Query.DESCENDING if descending else firestore.Query.ASCENDING
                query = query.order_by(order_by, direction=direction)

            # 件数制限
            if limit:
                query = query.limit(limit)

            # 実行計画の解析オプション
            explain_options = firestore.ExplainOptions(analyze=True) if analyze else None
            
            stream = query.stream(explain_options=explain_options)
            results = [dict(doc.to_dict(), document_id=doc.id) for doc in stream]

            # 分析が有効な場合のみメトリクスを出力
            if analyze:
                self._report_metrics(stream.get_explain_metrics())
                
            return results
        
        
    
    
    def _execute_chunked_query(
        self, 
        collection_path: str, 
        filters: list, 
        order_by: str = None, 
        descending: bool = False, 
        limit: int = None,
        analyze: bool = False
        ) -> list:
        """
        30個を超える 'in' 系のフィルタを分割実行し、結果を統合する。
        """
        # 1. 分割対象のフィルタを特定
        target_idx, field, op, large_list = next(
            (i, f, o, v) for i, (f, o, v) in enumerate(filters)
            if isinstance(v, list) and len(v) > 30 and o in ["in", "not_in", "array_contains_any"]
        )

        all_results = []

        # 2. 30個ずつのチャンクでループ
        for i in range(0, len(large_list), 30):
            chunk = large_list[i : i + 30]
            
            # フィルタを差し替えた一時的なリストを作成
            current_filters = list(filters)
            current_filters[target_idx] = (field, op, chunk)
            
            # 個別のクエリを実行（既存の単発実行メソッドを呼び出す）
            # 各クエリに limit を適用することで、最小限の取得で済ませる
            res = self._execute_single_query(
                collection_path, 
                current_filters, 
                order_by, 
                descending, 
                limit,
                analyze# 各チャンクでも最大 limit 分だけ取れば十分
            )
            
            all_results.extend(res)

            # 3. 早期終了判定
            # limit 指定があり、既に十分な数が集まっていれば、以降のチャンク実行をスキップ
            if limit and len(all_results) >= limit:
                all_results = all_results[:limit]
                break

        # 4. 全体ソートの再適用（重要）
        # チャンクごとにバラバラに取得されるため、結合後に再度ソートが必要
        if order_by and len(all_results) > 1:
            all_results.sort(
                key=lambda x: x.get(order_by), 
                reverse=descending
            )

        return all_results
    def _report_metrics(self, metrics):
            stats = metrics.execution_stats
            print(f"[Analyze] Results: {stats.results_returned}, Reads: {stats.read_operations}")
import json
if __name__ == "__main__":
    FS = FirestoreHandler(database="app-db")
    result = FS.search_docs(
        collection_path="users",
        filters=[("group", "==", "B"), ("age", "in", [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35])],
        analyze=True
    )
    print(result)

    # data_base = input("database: ")
    # FS = FirestoreHandler(database=data_base)
    # collection_path = input("collection_path: ")
    # document_id = input("document_id: ")
    # csv_path = input("csv_path: ")
    # data = json.load(open(csv_path))
    # confirm = input("Upload data to Firestore? (y/n): ")
    # if confirm.lower() == "y":
    #     FS.upload_json(collection_path, document_id, data)
    #     print("Upload completed successfully.")