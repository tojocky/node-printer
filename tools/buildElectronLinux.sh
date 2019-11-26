#!/bin/bash
VERSION=$1

# # Build Electron Linux 64bit
node-pre-gyp configure --target=$VERSION --arch=x64 --dist-url=https://electronjs.org/headers --module_name=node_printer --module_path=../lib/
node-pre-gyp build package --runtime=electron --target=$VERSION --target_arch=x64 --build-from-source

# #Build Electron Linux 32bit
node-pre-gyp configure --target=$VERSION --arch=ia32 --dist-url=https://electronjs.org/headers --module_name=node_printer --module_path=../lib/
node-pre-gyp build package --runtime=electron --target=$VERSION --target_arch=ia32 --build-from-source
