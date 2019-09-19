#!/bin/bash

RELEASE_VERSION=$1
PACKAGE_VERSION=$(node -pe "require('./package.json').version")
SOURCE_PATH="${BASH_SOURCE%/*}/.."

declare -a node_versions=(
  "0.10.48"
  "0.12.18"
  "4.9.1"
  "5.9.1"
  "6.17.1"
  "8.16.1"
  "10.16.0"
  "11.15.0"
  "12.10.0"
)

declare -a electron_versions=(
  "1.2.8"
  "1.3.8"
  "1.4.6"
  "1.7.12"
  "2.0.18"
  "3.1.13"
  "4.2.10"
  "5.0.10"
  "6.0.7"
)

# remove old build directory
rm -rf "$SOURCHE_PATH/build" > /dev/null

# create release path
mkdir -p "$SOURCE_PATH/releases/$RELEASE_VERSION" > /dev/null

for version in "${node_versions[@]}"
do
  echo "Building for node version: $version..."
  node-pre-gyp configure --target=$version --module_name=node_printer --silent
  node-pre-gyp build package --target=$version --target_arch=x64 --build-from-source --silent
  node-pre-gyp configure --target=$version --module_name=node_printer --silent
  node-pre-gyp build package --target=$version --target_arch=ia32 --build-from-source --silent
  rsync -a -v "$SOURCE_PATH/build/stage/$PACKAGE_VERSION/" "$SOURCE_PATH/releases/$RELEASE_VERSION/" --remove-source-files > /dev/null
  echo "Done"
done

for version in "${electron_versions[@]}"
do
  echo "Building for electron version: $version..."
  node-pre-gyp configure --target=$version --dist-url=https://electronjs.org/headers --module_name=node_printer --silent
  node-pre-gyp build package --target=$version --target_arch=x64 --runtime=electron --build-from-source --silent
  node-pre-gyp configure --target=$version --dist-url=https://electronjs.org/headers --module_name=node_printer --silent
  node-pre-gyp build package --target=$version --target_arch=ia32 --runtime=electron --build-from-source --silent
  rsync -a -v "$SOURCE_PATH/build/stage/$PACKAGE_VERSION/" "$SOURCE_PATH/releases/$RELEASE_VERSION/" --remove-source-files > /dev/null
  echo "Done"
done

echo "Finished succesfully!"
