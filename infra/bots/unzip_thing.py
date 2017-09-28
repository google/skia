import os
import sys
import zip_utils


zip_file = sys.argv[1]
target_dir = sys.argv[2]


zip_utils.unzip(zip_file, target_dir)
