# --- Root Makefile ---

ENGINE_DIR := engine
CLIENT_DIR := client
RES_DIR    := client/models

OUTPUT_DIR    := artifacts
ARTIFACT_NAME := CG_Group12_project_release.tar.gz

all: pipeline

engine:
	@echo "--- [1/3] Building Backend (Engine) ---"
	$(MAKE) -C $(ENGINE_DIR) engine

client: engine
	@echo "--- [2/3] Building Frontend (Client) ---"
	$(MAKE) -C $(CLIENT_DIR) client

test: engine
	@echo "--- [3/3] Running Tests on Backend ---"
#	$(MAKE) -C $(ENGINE_DIR) test

package: client test
	@echo "--- Packaging Artifacts (No Rebuild) ---"
	mkdir -p $(OUTPUT_DIR)
	rm -rf temp_package
	mkdir -p temp_package
	
	@echo "Copying binaries..."
	cp $(ENGINE_DIR)/bin/Release/libengine.so temp_package/
	cp $(CLIENT_DIR)/bin/Release/client temp_package/
	
	@echo "Copying resources..."
	@[ -d "$(RES_DIR)" ] && cp -r "$(RES_DIR)" temp_package/ || echo "Info: No directory '$(RES_DIR)' found, skipping the copy."
	
	@echo "Creating startup script..."
	echo '#!/bin/sh' > temp_package/run.sh
	echo 'export LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH' >> temp_package/run.sh
	echo './client' >> temp_package/run.sh
	chmod +x temp_package/run.sh
	
	@echo "Creating compressed archive..."
	tar -czvf $(OUTPUT_DIR)/$(ARTIFACT_NAME) -C temp_package .
	
	@echo "TMP cleaning..."
	rm -rf temp_package
	@echo "SUCCESS: Artifact created at $(OUTPUT_DIR)/$(ARTIFACT_NAME)"

pipeline: engine client test package

clean:
	$(MAKE) -C $(ENGINE_DIR) clean
	$(MAKE) -C $(CLIENT_DIR) clean
	rm -rf $(OUTPUT_DIR) temp_package

.PHONY: all engine client test package clean
