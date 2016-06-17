node-gyp clean
node-pre-gyp configure
node-pre-gyp build package
node-gyp configure --target=0.36.1 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=0.36.1 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=0.36.1 --target_arch=x64
node-gyp configure --target=1.2.6 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=1.2.6 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=1.2.6 --target_arch=x64
node-pre-gyp configure
node-pre-gyp build package --target_arch=ia32
node-gyp configure --target=0.36.1 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=0.36.1 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=0.36.1 --target_arch=ia32
node-gyp configure --target=1.2.6 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=1.2.6 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=1.2.6 --target_arch=ia32
node-pre-gyp-github publish
