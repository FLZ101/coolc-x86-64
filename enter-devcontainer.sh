#!/bin/bash

set -e

cd "$(dirname "$0")"

if ! command -v devcontainer > /dev/null; then
    echo "devcontainer CLI not found, install with: npm install -g @devcontainers/cli" >&2
    exit 1
fi

devcontainer up --docker-path podman --workspace-folder .
exec devcontainer exec --docker-path podman --workspace-folder . bash
