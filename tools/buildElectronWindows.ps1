#
# Usage: buildElectronWindows.ps1 <version>
#

if ($args.Length -ne 1) {
  echo "Must Supply only 1 argument - Version"
  return
}

$version = $args[0]

echo "Building Electron Version -> $version"

# Build Electron Windows 64bit
node_modules\.bin\node-pre-gyp.cmd configure --target=$version --arch=x64 --dist-url=https://electronjs.org/headers --module_name=node_printer
node_modules\.bin\node-pre-gyp.cmd build package --runtime=electron --target=$version --target_arch=x64 --build-from-source

# Build Electron Windows 32bit
node_modules\.bin\node-pre-gyp.cmd configure --target=$version --arch=ia32 --dist-url=https://electronjs.org/headers --module_name=node_printer
node_modules\.bin\node-pre-gyp.cmd build package --runtime=electron --target=$version --target_arch=ia32 --build-from-source