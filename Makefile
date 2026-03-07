BLUE := \033[36m
YELLOW := \033[33m
BOLD := \033[1m
RESET := \033[0m
HELP_WIDTH=14
.PHONY: help
help:   ## Show this help.
	@# targets should be "target: whatever ## help"
	@# general printing "##@ just printed out in the same output as targets, use for more description"
	@echo "Usage: make ${BLUE}[target]${RESET}:"
	@echo "${BLUE}targets${RESET}:"
	@awk 'BEGIN {FS = ":.*##"} /^[0-9a-zA-Z_-]+:.*?##/ { printf "  ${BLUE}%-${HELP_WIDTH}s${RESET} %s\n", $$1, $$2 } /^##@/ { printf "\n${BOLD}%s${RESET}\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

BUILD_DIR := build
BINARY := ${BUILD_DIR}/epicyclic.elf

.DEFAULT_GOAL := ${BINARY}

${BUILD_DIR}: CMakeLists.txt
	cmake -B $@ .

${BINARY}: ${BUILD_DIR} CMakeLists.txt src/* ## compile the project
	cmake --build ${BUILD_DIR}

.PHONY: compile
compile: ${BINARY} ## actually build the project, build target just runs cmake -B build

.PHONY: flash
flash: ${BINARY} ## flash with picotool
	./tools/flash.sh $<

.PHONY: flash-prog
flash-prog: ${BINARY} ## flash with a programmer
	./tools/flash.sh --programmer $<

.PHONY: debug
debug: ${BINARY} ## debug with a programmer
	./tools/debug.sh $<

.PHONY: clean
clean: ## remove build dir
	cmake --build ${BUILD_DIR} --target clean

.PHONY: clean-harder
clean-harder: ## remove build dir; WARNING: will need to redownload pico sdk
	rm -rf ${BUILD_DIR}

.PHONY: test
test: ## run crsf lib tests TODO this is bad approach
	cmake --build lib/crsf/build -t check
	cmake --build lib/crsf/build -t check_mem
