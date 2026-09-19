# CatsRace Backend

GCP プロジェクトID: `catsrace`

## 開発環境 (Docker)

VS Code の **Dev Containers** 拡張機能を利用します。
プロジェクトを開き、**「Reopen in Container」** を実行して開発コンテナを起動してください。

## セットアップ (GCP 認証)

コンテナ起動後、GCP にログインします。

```sh
gcloud config set project catsrace
gcloud auth login
gcloud auth application-default login
```

## インフラ基盤のデプロイ (初回)

`gcp-infra/` 配下の Terraform を適用し、必要な GCP API の有効化およびソースバケットを作成します。

```sh
cd gcp-infra
terraform init
terraform plan
terraform apply
```

## functionのローカル実行 (GCFデプロイ前のデバッグ)

指定した function を Functions Framework で同時に起動し、Caddy 経由（`http://localhost:8080/<function-name>`）で公開します。
ローカルデバッグ時も Firestore は GCP 実環境に接続されます。

`functions/` にはランキング・ゴースト V2 API の3関数と、関数追加用の `template` があります。

```sh
# 新API 3関数の同時ローカル起動
make local FUNCTIONS="get-race-ghosts-v2 post-race-result-v2 get-world-ranking-v2"
```

アクセス先:

```text
http://localhost:8080/get-race-ghosts-v2
http://localhost:8080/post-race-result-v2
http://localhost:8080/get-world-ranking-v2
```

リクエスト例 (get-race-ghosts-v2):

```sh
curl -X POST \
  -H "Content-Type: application/json" \
  -d '{"schema_version": 1, "map_id": "b", "map_version": 1}' \
  http://localhost:8080/get-race-ghosts-v2
```

`functions/` 直下の全 function を起動する場合:

```sh
make local-all
```

停止は起動中のターミナルで `Ctrl+C` を入力します。

生成された共通モジュール、requirements、Caddyfile を削除する場合:

```sh
make local-clean
```

## 本番デプロイ (全ローカル検証完了後)

ローカルでの単体・結合テストが完了した後、各 function をデプロイします。

```sh
make deploy TARGET_DIR=./functions/post-race-result-v2
make deploy TARGET_DIR=./functions/get-world-ranking-v2
make deploy TARGET_DIR=./functions/get-race-ghosts-v2
```
