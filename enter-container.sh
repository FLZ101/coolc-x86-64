#!/bin/bash

set -e

cd "$(dirname "$0")"

NAME=coolc-x86-64
DOCKERFILE=./Containerfile
CONTEXT=.

IMAGE="$NAME-dev-image"
CONTAINER="$NAME-dev"
WORKSPACE="/workspaces/$NAME"

if ! podman image exists "$IMAGE"; then
    podman build -f "$DOCKERFILE" -t "$IMAGE" "$CONTEXT"
fi

if ! podman container exists "$CONTAINER"; then
    podman run -d --name "$CONTAINER" \
        -v "$PWD:$WORKSPACE" \
        -w "$WORKSPACE" \
        "$IMAGE" sleep infinity
fi

if [ "$(podman inspect -f '{{.State.Running}}' "$CONTAINER")" != "true" ]; then
    podman start "$CONTAINER"
fi

exec podman exec -it -w "$WORKSPACE" "$CONTAINER" bash
