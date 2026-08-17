//gcloud storage buckets create gs://tfstate-987203 --project=catsrace --location=us-west1
terraform {
  backend "gcs" {
    bucket = "tfstate-987203"
    prefix = "infra-state"
  }

  required_version = ">= 1.6.0"
  required_providers {
    google = {
      source  = "hashicorp/google"
      version = "~> 5.0"
    }
  }
}

provider "google" {
  project = "catsrace"
}

resource "google_project_service" "cloudfunctions" {
  service            = "cloudfunctions.googleapis.com"
  disable_on_destroy = false
}

resource "google_project_service" "cloudrun" {
  service            = "run.googleapis.com"
  disable_on_destroy = false
}

resource "google_project_service" "artifactregistry" {
  service            = "artifactregistry.googleapis.com"
  disable_on_destroy = false
}

resource "google_project_service" "cloudbuild" {
  service            = "cloudbuild.googleapis.com"
  disable_on_destroy = false
}

resource "google_storage_bucket" "tfsource-987203" {
  name                        = "tfsource-987203"
  location                    = "us-west1"
  uniform_bucket_level_access = true
  force_destroy               = true
}
