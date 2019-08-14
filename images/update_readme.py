'''
Run this to update the README.md file so that it contains all .png and .jpg files in this directory.
'''

import os

with open('README.md', 'w') as fh:
    for f in sorted([s for s in os.listdir() if s[-3:] in ('png','jpg')]):
        fh.write(f'![{f}]({f})\n')
