#
# Usage: buildElectronWindows.ps1 <version>
#

if ($args.Length -ne 1) {
  echo "Must Supply only 1 argument - Version"
  return
}

$version = $args[0]

echo "Building Electron Version -> $version"

node-gyp configure --target=$version --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/
node-gyp build --target=$version --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/
node-pre-gyp package --runtime=electron --target=$version --target_arch=x64

node-gyp configure --target=$version --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/
node-gyp build --target=$version --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/
node-pre-gyp package --runtime=electron --target=$version --target_arch=ia32