param (
  [Parameter(Mandatory=$true)][string]$release
)

$SOURCE_PATH = split-path -parent $MyInvocation.MyCommand.Definition
$RELEASE_VERSION = $release
$PACKAGE_VERSION = node -pe "require('./package.json').version"

echo $SOURCE_PATH

$node_versions = @(
  "0.10.48",
  "0.12.18",
  "4.9.1",
  "5.9.1",
  "6.17.1",
  "8.16.1",
  "10.16.0",
  "11.15.0",
  "12.10.0"
)

$electron_versions = @(
  "1.2.8",
  "1.3.8",
  "1.4.6",
  "1.7.12",
  "2.0.18",
  "3.1.13",
  "4.2.10",
  "5.0.10",
  "6.0.7"
)

# remove old build directory
Remove-Item -Recurse -Force $SOURCE_PATH'\..\build' -ErrorAction Ignore | Out-Null

# create release path
New-Item $SOURCE_PATH'\..\releases\'$RELEASE_VERSION -ItemType Directory -ea 0 | Out-Null

foreach ($version in $node_versions) {
  Write-Output "Building for node version: $version..."
  node-pre-gyp configure --target=$version --module_name=node_printer --silent | Out-Null
  node-pre-gyp build package --target=$version --target_arch=x64 --build-from-source --silent | Out-Null
  node-pre-gyp configure --target=$version --module_name=node_printer --silent | Out-Null
  node-pre-gyp build package --target=$version --target_arch=ia32 --build-from-source --silent | Out-Null
  Copy-item -Force -Recurse $SOURCE_PATH'\..\build\stage\'$PACKAGE_VERSION\* -Destination $SOURCE_PATH'\..\releases\'$RELEASE_VERSION -ErrorAction Ignore | Out-Null
  Remove-Item -Recurse -Force $SOURCE_PATH'\..\build\stage' | Out-Null
  Write-Output "Done"
}

foreach ($version in $electron_versions) {
  Write-Output "Building for electron version: $version..."
  node-pre-gyp configure --target=$version --dist-url=https://electronjs.org/headers --module_name=node_printer --silent | Out-Null
  node-pre-gyp build package --target=$version --target_arch=x64 --runtime=electron --build-from-source --silent | Out-Null
  node-pre-gyp configure --target=$version --dist-url=https://electronjs.org/headers --module_name=node_printer --silent | Out-Null
  node-pre-gyp build package --target=$version --target_arch=ia32 --runtime=electron --build-from-source --silent | Out-Null
  Copy-item -Force -Recurse $SOURCE_PATH'\..\build\stage\'$PACKAGE_VERSION\* -Destination $SOURCE_PATH'\..\releases\'$RELEASE_VERSION -ErrorAction Ignore | Out-Null
  Remove-Item -Recurse -Force $SOURCE_PATH'\..\build\stage' | Out-Null
  Write-Output "Done"
}

Write-Output "Finished succesfully!"