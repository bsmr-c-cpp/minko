#!/bin/bash

DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

pushd ${DIR}/../../.. > /dev/null
tool/mac/script/premake5.sh --no-test --with-writer xcode-osx
popd > /dev/null
