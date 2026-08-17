# The backend bucket is bootstrapped once, before terraform init.
# Intended bootstrap command:
# gcloud storage buckets create gs://tfstate-catsrace-1a747116 --project=catsrace --location=us-west1
terraform {
  backend "gcs" {
    bucket = "tfstate-catsrace-1a747116"
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

resource "google_project_service" "firestore" {
  service            = "firestore.googleapis.com"
  disable_on_destroy = false
}

resource "google_storage_bucket" "tfsource_catsrace" {
  name                        = "tfsource-catsrace"
  location                    = "us-west1"
  uniform_bucket_level_access = true
  force_destroy               = false

  depends_on = [google_project_service.cloudbuild]
}

resource "google_firestore_database" "default" {
  project                 = "catsrace"
  name                    = "(default)"
  location_id             = "us-west1"
  type                    = "FIRESTORE_NATIVE"
  delete_protection_state = "DELETE_PROTECTION_ENABLED"
  deletion_policy         = "DELETE"

  depends_on = [google_project_service.firestore]
}

resource "google_firestore_index" "race_leaderboards_v2_ranking" {
  project    = "catsrace"
  database   = google_firestore_database.default.name
  collection = "race_leaderboards_v2"

  fields {
    field_path = "map_id"
    order      = "ASCENDING"
  }
  fields {
    field_path = "map_version"
    order      = "ASCENDING"
  }
  fields {
    field_path = "is_valid"
    order      = "ASCENDING"
  }
  fields {
    field_path = "score"
    order      = "ASCENDING"
  }
  fields {
    field_path = "identity_key"
    order      = "ASCENDING"
  }
}
