import json
import os
import re
import subprocess
import sys


tasks_json = 'tasks.json'
expect_dir = os.path.join('recipes', 'test.expected')
placeholder = re.compile('<\([A-Z_]+\)')
placeholder2 = re.compile('\$\{[A-Z_]+\}')


def after(cmd, key, offset):
  print cmd
  idx = 0
  for arg in cmd:
    idx += 1
    if key == arg:
      idx += offset
      break
  print cmd[idx:]
  return cmd[idx:]


def read_expectations(tests):
  dm_flags = {}
  for test in tests:
    f = os.path.join(expect_dir, test+'.json')
    with open(f, 'r') as fd:
      content = json.load(fd)
    cmd = None
    for step in content:
      if step.get('name') == 'dump dm flags':
        cmd = step['cmd'][3:]
        break
    if not cmd:
      raise Exception('"dm" step not found in %s' % f)
    dm_flags[test] = ' '.join(cmd)
  return dm_flags


def get_relevant_tasks(tasks_json):
  with open(tasks_json, 'r') as f:
    task_cfgs = json.load(f)['tasks']
  tasks = []
  for name, task in task_cfgs.iteritems():
    if name.startswith('Test') and task['command'][4] == 'test':
      tasks.append((name, task))
  tasks.sort(key=lambda elem: elem[0])
  return tasks


def read_actuals(tasks_json):
  tasks = get_relevant_tasks(tasks_json)
  dm_flags = {}
  for name, task in tasks:
    for arg in task['command']:
      if arg.startswith('{'):
        props = json.loads(arg)
        dm_flags[name] = props['dm_flags']
  return dm_flags


def write(f, cmd):
  with open(f, 'w') as fd:
    fd.writelines([line + '\n' for line in cmd])


def compare(expect, actual, name):
  e_split = expect.split(' ')
  a_split = actual.split(' ')
  if len(e_split) == len(a_split):
    match = True
    for i, e_arg in enumerate(e_split):
      a_arg = a_split[i]
      if e_arg != a_arg and not placeholder.match(a_arg) and not placeholder2.match(a_arg) and a_arg != 'TODO':
        print 'no match; %s != %s' % (e_arg, a_arg)
        match = False
        break
    if match:
      return

  e_file = '%s.expect' % name
  a_file = '%s.actual' % name
  write(e_file, e_split)
  write(a_file, a_split)
  subprocess.call(['diffuse', e_file, a_file])
  os.remove(e_file)
  os.remove(a_file)
  raise Exception('Wanted for %s:\n%s\nbut got:\n%s' % (name, expect, actual))


def refresh_bots(tasks_json):
  tasks = get_relevant_tasks(tasks_json)
  script = os.path.join('recipes', 'test.py')
  with open(script, 'r') as f:
    lines = f.readlines()
  new_lines = []
  in_list = False
  for line in lines:
    if line.startswith('TEST_BUILDERS = ['):
      in_list = True
    elif in_list:
      if line.startswith(']'):
        for name, _ in tasks:
          new_lines.append('  \'%s\',\n' % name)
        in_list = False
      else:
        continue
    new_lines.append(line)
  with open(script, 'w') as f:
    f.writelines(new_lines)


def main():
  if '--refresh' in sys.argv:
    refresh_bots(tasks_json)
  else:
    tests = sys.argv[1:]
    actual = read_actuals(tasks_json)
    if not tests:
      tests = actual.keys()
    expect = read_expectations(tests)
    for name in tests:
      compare(expect[name], actual[name], name)
    print 'Compared %d cases' % len(tests)


if __name__ == '__main__':
  main()
