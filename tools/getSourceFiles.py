# simple python command to list files like ls -1
import os
for file in os.listdir('src'):
    if (file.endswith('.cc')):
        print('src/'+file)
