#!/bin/bash

set -xe

make_relative_when_children() {
    local base
    local path
    base=$(realpath "$1")
    path=$(realpath "$2")
    if [[ "$path" == "$base"* ]]; then
        realpath --relative-to="$base" "$path"
    else
        echo "$path"
    fi
}

napi_include_path=$(node -p "require('node-addon-api').include_dir" | tr -d '"')
node_include_path=$(node -p "require('node-api-headers').include_dir" | tr -d '"')

napi_include_path=$(make_relative_when_children "$(pwd)" "$napi_include_path")
node_include_path=$(make_relative_when_children "$(pwd)" "$node_include_path")

meson setup "./builddir" \
    --reconfigure \
    --prefix="$(pwd)/builddir/install" \
    -Dc_link_args=-Wl,-rpath,\$ORIGIN \
    -Dcpp_link_args=-Wl,-rpath,\$ORIGIN \
    -Dnapi_include_path="$napi_include_path" \
    -Dnode_include_path="$node_include_path"
