import subprocess as sp
import sys
import os

# Usage: commit_id.py check <angle_dir> (checks if git is present)
# Usage: commit_id.py gen <angle_dir> <file_to_write> (generates commit id)

def grab_output(command, cwd):
    return sp.Popen(command, stdout=sp.PIPE, shell=True, cwd=cwd).communicate()[0].strip()

operation = sys.argv[1]
cwd = sys.argv[2]

if operation == 'check':
    index_path = os.path.join(cwd, '.git', 'index')
    if os.path.exists(index_path):
        print("1")
    else:
        print("0")
    sys.exit(0)

output_file = sys.argv[3]
commit_id_size = 12

try:
    commit_id = grab_output('git rev-parse --short=%d HEAD' % commit_id_size, cwd)
    commit_date = grab_output('git show -s --format=%ci HEAD', cwd)
except:
    commit_id = 'invalid-hash'
    commit_date = 'invalid-date'

hfile = open(output_file, 'w')

hfile.write('#define ANGLE_COMMIT_HASH "%s"\n'    % commit_id)
hfile.write('#define ANGLE_COMMIT_HASH_SIZE %d\n' % commit_id_size)
hfile.write('#define ANGLE_COMMIT_DATE "%s"\n'    % commit_date)

hfile.close()
