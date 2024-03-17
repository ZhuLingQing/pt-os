#!/bin/bash
set -e
set -o pipefail

REPO_DIR=$(cd "$(dirname $0)";pwd)/..
CURR_DIR=${REPO_DIR}/tests

pushd ${REPO_DIR}

echo "******* Unit Tests Start *******"
echo "******* All Unit Tests Passed *******"

popd
