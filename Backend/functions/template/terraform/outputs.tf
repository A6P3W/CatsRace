output "function_name" {
  description = "デプロイされた Cloud Function の名前"
  value       = google_cloudfunctions2_function.hello.name
}

output "function_url" {
  description = "外部からアクセスするための HTTP エンドポイント URL"
  value       = google_cloudfunctions2_function.hello.service_config[0].uri
}

output "cloud_run_service" {
  description = "実体として生成された Cloud Run サービスの名前"
  value       = google_cloudfunctions2_function.hello.service_config[0].service
}
