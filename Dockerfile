FROM docker.io/debian:11.2

# where to install arduino-cli (used by the install script)
ENV BINDIR=/usr/local/bin

ENV ARDUINO_CLI_VERSION=0.21.1
# do not check for new versions. updates should be done by bumping the above number.
ENV ARDUINO_UPDATER_ENABLE_NOTIFICATION=false

RUN set -eux; \
    BUILD_DEPS=" \
        curl \
    "; \
    apt-get update; \
    apt-get install -y ${BUILD_DEPS}; \
    rm -rf /var/lib/apt/lists/*; \
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/${ARDUINO_CLI_VERSION}/install.sh | sh -s ${ARDUINO_CLI_VERSION}

WORKDIR /app

USER 1000
