node-gyp configure --target=1.3.2 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=1.3.2 --arch=x64 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=1.3.2 --target_arch=x64


node-gyp configure --target=1.3.2 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-gyp build --target=1.3.2 --arch=ia32 --dist-url=https://atom.io/download/atom-shell --module_name=node_printer --module_path=../lib/binding/node_printer/
node-pre-gyp package --runtime=electron --target=1.3.2 --target_arch=ia32