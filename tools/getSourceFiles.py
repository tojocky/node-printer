# simple python command to list files like ls -1
import os, sys

if len(sys.argv) < 3:
    sys.stderr.write('use: '+sys.argv[0]+' <path> <ext>. e.g.:'+sys.argv[0]+' src cc\n')
    sys.exit(1)
folder = sys.argv[1]
file_ext = '.'+sys.argv[2]

for file in os.listdir(folder):
    if (file.endswith(file_ext)):
        print(folder+'/'+file)
