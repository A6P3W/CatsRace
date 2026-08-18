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
  type        = string
  default     = "512Mi"
  description = "Available memory for the Cloud Function"
}

variable "runtime" {
  type        = string
  default     = "python312"
  description = "The runtime environment for the Cloud Function"
}

variable "function_name" {
  description = "Cloud Function name"
  type        = string
  default     = "template"
}

variable "bucket_name" {
  description = "The name of the GCS bucket to store function source"
  type        = string
  default     = "tfsource-catsrace"
}
