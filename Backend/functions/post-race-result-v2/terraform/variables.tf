variable "project_id" {
  description = "GCP project ID"
  type        = string
  default     = "catsrace"
}

variable "region" {
  description = "GCP region"
  type        = string
  default     = "us-west1"
}

variable "memory_size" {
  description = "Available memory for the Cloud Function"
  type        = string
  default     = "512Mi"
}

variable "runtime" {
  description = "The runtime environment for the Cloud Function"
  type        = string
  default     = "python312"
}

variable "function_name" {
  description = "Cloud Function name"
  type        = string
  default     = "post-race-result-v2"
}

variable "bucket_name" {
  description = "GCS bucket used for function source archives"
  type        = string
  default     = "tfsource-catsrace"
}

