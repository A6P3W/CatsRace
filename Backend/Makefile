PYTHON ?= python3
PY_SCRIPT = copy_module.py
TARGET_DIR =
CELLS ?=
PROXY_PORT ?= 8080

.PHONY: all run tfinit tfplan tfapply deploy local local-all local-clean help

all: help

help:
	@echo "Usage: make [run|tfinit|tfplan|tfapply|deploy] TARGET_DIR=./cells/example"
	@echo '       make local CELLS="get-race-ghosts-v2 post-race-result-v2 get-world-ranking-v2" [PROXY_PORT=8080]'
	@echo "       make local-all [PROXY_PORT=8080]"
	@echo "       make local-clean"

local:
ifeq ($(strip $(CELLS)),)
	$(error CELLS is required. Example: make local CELLS="get-race-ghosts-v2 post-race-result-v2 get-world-ranking-v2")
endif
	$(PYTHON) local.py --cells $(CELLS) --proxy-port $(PROXY_PORT)

local-all:
	$(PYTHON) local.py --all --proxy-port $(PROXY_PORT)

local-clean:
	rm -rf .local
	find cells -type d -path "*/src/common" -prune -exec rm -rf {} +
	find cells -type f -path "*/src/requirements.txt" -delete

run:
ifeq ($(TARGET_DIR),)
	$(error Error: TARGET_DIR is not set)
endif
	$(PYTHON) $(PY_SCRIPT) $(TARGET_DIR)

tfinit:
ifeq ($(TARGET_DIR),)
	$(error Error: TARGET_DIR is not set)
endif
	cd $(TARGET_DIR)/terraform && terraform init

tfplan:
ifeq ($(TARGET_DIR),)
	$(error Error: TARGET_DIR is not set)
endif
	cd $(TARGET_DIR)/terraform && terraform plan

tfapply:
ifeq ($(TARGET_DIR),)
	$(error Error: TARGET_DIR is not set)
endif
	cd $(TARGET_DIR)/terraform && terraform apply

deploy:
ifeq ($(TARGET_DIR),)
	$(error Error: TARGET_DIR is not set. Usage: make deploy TARGET_DIR=./cells/example)
endif
	$(MAKE) run TARGET_DIR=$(TARGET_DIR)
	$(MAKE) tfinit TARGET_DIR=$(TARGET_DIR)
	$(MAKE) tfplan TARGET_DIR=$(TARGET_DIR)
	$(MAKE) tfapply TARGET_DIR=$(TARGET_DIR)
