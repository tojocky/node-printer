# Build Win32 64bit
node-pre-gyp configure
node-pre-gyp build package

# Build Win32 32bit
node-pre-gyp configure
node-pre-gyp build package --target_arch=ia32

if ($env:build_electron -ne "true") {
  echo "Skipping Electron Build as flag not set"
  return
}

# Build Electron Versions
./tools/buildElectronWindows.ps1 1.2.8
./tools/buildElectronWindows.ps1 1.4.5
