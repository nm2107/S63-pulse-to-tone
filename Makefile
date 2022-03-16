DEVICE ?= /dev/ttyACM0
COMPILE_FLAGS ?=

# export vars to invoked commands
export

.PHONY: build
build:
	docker-compose build

.PHONY: install-deps
install-deps:
	docker-compose run --rm app \
		arduino-cli core install arduino:megaavr@1.8.7

.PHONY: shell
shell:
	docker-compose run --rm app \
		/bin/bash

.PHONY: monitor
monitor:
	docker-compose run --rm app \
		arduino-cli monitor \
			-p $(DEVICE) \
			--fqbn arduino:megaavr:nona4809 \
			-l serial

.PHONY: compile
compile: .enable-logging .do-compile

.PHONY: compile-release
compile-release: .do-compile

.PHONY: upload
upload:
	docker-compose run --rm app \
		arduino-cli upload \
			-p $(DEVICE) \
			--fqbn arduino:megaavr:nona4809 \
			--input-dir build \
			--verify \
			src

#################
# PRIVATE TASKS #
#################

.PHONY: .enable-logging
.enable-logging:
	$(eval COMPILE_FLAGS += --build-properties build.extra_flags=-DENABLE_LOGGING)

.PHONY: .do-compile
.do-compile:
	docker-compose run --rm app \
		arduino-cli compile \
			--fqbn arduino:megaavr:nona4809 \
			--build-path build \
			$(COMPILE_FLAGS) \
			src
