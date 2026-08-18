terraform {
  backend "gcs" {
    bucket = "tfstate-catsrace-1a747116"
    prefix = "cells/get-race-ghosts-v2"
  }

  required_version = ">= 1.6.0"

  required_providers {
    archive = {
      source  = "hashicorp/archive"
      version = "~> 2.4"
    }
    google = {
      source  = "hashicorp/google"
      version = "~> 5.0"
    }
  }
}

provider "google" {
  project = var.project_id
  region  = var.region
}

data "google_storage_bucket" "source_bucket" {
  name = var.bucket_name
}

data "archive_file" "function_zip" {
  type        = "zip"
  source_dir  = "${path.module}/../src"
  output_path = "${path.module}/function.zip"
}

resource "google_storage_bucket_object" "function_source" {
  name   = "${var.function_name}-${data.archive_file.function_zip.output_md5}.zip"
  bucket = data.google_storage_bucket.source_bucket.name
  source = data.archive_file.function_zip.output_path
}

resource "google_cloudfunctions2_function" "function" {
  name     = var.function_name
  location = var.region

  build_config {
    runtime     = var.runtime
    entry_point = "cell_entry_point"

    source {
      storage_source {
        bucket = data.google_storage_bucket.source_bucket.name
        object = google_storage_bucket_object.function_source.name
      }
    }
  }

  service_config {
    available_memory                 = var.memory_size
    timeout_seconds                  = 60
    max_instance_count               = 1
    max_instance_request_concurrency = 4
    ingress_settings                 = "ALLOW_ALL"
    all_traffic_on_latest_revision   = true

    environment_variables = {
      PROJECT_ID = var.project_id
    }
  }
}

resource "google_cloud_run_service_iam_member" "public_access" {
  location = google_cloudfunctions2_function.function.location
  service  = google_cloudfunctions2_function.function.name
  role     = "roles/run.invoker"
  member   = "allUsers"
}
