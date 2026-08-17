output "function_name" {
  description = "Deployed Cloud Function name"
  value       = google_cloudfunctions2_function.function.name
}

output "function_url" {
  description = "HTTP endpoint URL"
  value       = google_cloudfunctions2_function.function.service_config[0].uri
}

