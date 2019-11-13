#!/usr/bin/env python

# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import with_statement

# Imports the monkeyrunner modules used by this program
from com.android.monkeyrunner import MonkeyRunner, MonkeyDevice

import ast
import os
import subprocess
import time


# Time to wait between performing UI actions and capturing the SKP.
WAIT_FOR_SKP_CAPTURE = 1


class DragAction:
  """Action describing a touch drag."""
  def __init__(self, start, end, duration, points):
    self.start = start
    self.end = end
    self.duration = duration
    self.points = points

  def run(self, device):
    """Perform the action."""
    return device.drag(self.start, self.end, self.duration, self.points)


class PressAction:
  """Action describing a button press."""
  def __init__(self, button, press_type):
    self.button = button
    self.press_type = press_type

  def run(self, device):
    """Perform the action."""
    return device.press(self.button, self.press_type)


def parse_action(action_dict):
  """Parse a dict describing an action and return an Action object."""
  if action_dict['type'] == 'drag':
    return DragAction(tuple(action_dict['start']),
                      tuple(action_dict['end']),
                      action_dict['duration'],
                      action_dict['points'])
  elif action_dict['type'] == 'press':
    return PressAction(action_dict['button'], action_dict['press_type'])
  else:
    raise TypeError('Unsupported action type: %s' % action_dict['type'])


class App:
  """Class which describes an app to launch and actions to run."""
  def __init__(self, name, package, activity, app_launch_delay, actions):
    self.name = name
    self.package = package
    self.activity = activity
    self.app_launch_delay = app_launch_delay
    self.run_component = '%s/%s' % (self.package, self.activity)
    self.actions = [parse_action(a) for a in actions]

  def launch(self, device):
    """Launch the app on the device."""
    device.startActivity(component=self.run_component)
    time.sleep(self.app_launch_delay)

  def kill(self):
    """Kill the app."""
    adb_shell('am force-stop %s' % self.package)


def check_output(cmd):
  """Convenience implementation of subprocess.check_output."""
  proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  if proc.wait() != 0:
    raise Exception('Command failed: %s' % ' '.join(cmd))
  return proc.communicate()[0]


def adb_shell(cmd):
  """Run the given ADB shell command and emulate the exit code."""
  output = check_output(['adb', 'shell', cmd + '; echo $?']).strip()
  lines = output.splitlines()
  if lines[-1] != '0':
    raise Exception('ADB command failed: %s\n\nOutput:\n%s' % (cmd, output))
  return '\n'.join(lines[:-1])


def remote_file_exists(filename):
  """Return True if the given file exists on the device and False otherwise."""
  try:
    adb_shell('test -f %s' % filename)
    return True
  except Exception:
    return False


def capture_skp(skp_file, package, device):
  """Capture an SKP."""
  remote_path = '/data/data/%s/cache/%s' % (package, os.path.basename(skp_file))
  try:
    adb_shell('rm %s' % remote_path)
  except Exception:
    if remote_file_exists(remote_path):
      raise

  adb_shell('setprop debug.hwui.capture_frame_as_skp %s' % remote_path)
  try:
    # Spin, wait for the SKP to be written.
    timeout = 10  # Seconds
    start = time.time()
    device.drag((300, 300), (300, 350), 1, 10)  # Dummy action to force a draw.
    while not remote_file_exists(remote_path):
      if time.time() - start > timeout:
        raise Exception('Timed out waiting for SKP capture.')
      time.sleep(1)

    # Pull the SKP from the device.
    cmd = ['adb', 'pull', remote_path, skp_file]
    check_output(cmd)

  finally:
    adb_shell('setprop debug.hwui.capture_frame_as_skp ""')


def load_app(filename):
  """Load the JSON file describing an app and return an App instance."""
  with open(filename) as f:
    app_dict = ast.literal_eval(f.read())
  return App(app_dict['name'],
             app_dict['package'],
             app_dict['activity'],
             app_dict['app_launch_delay'],
             app_dict['actions'])


def main():
  """Capture SKPs for all apps."""
  device = MonkeyRunner.waitForConnection()

  # TODO(borenet): Kill all apps.
  device.wake()
  device.drag((600, 600), (10, 10), 0.2, 10)

  apps_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'apps')
  app_files = [os.path.join(apps_dir, app) for app in os.listdir(apps_dir)]

  for app_file in app_files:
    app = load_app(app_file)
    print app.name
    print '  Package %s' % app.package
    app.launch(device)
    print '  Launched activity %s' % app.activity

    for action in app.actions:
      print '  %s' % action.__class__.__name__
      action.run(device)

    time.sleep(WAIT_FOR_SKP_CAPTURE)
    print '  Capturing SKP.'
    skp_file = '%s.skp' % app.name
    capture_skp(skp_file, app.package, device)
    print '  Wrote SKP to %s' % skp_file
    print
    app.kill()


if __name__ == '__main__':
  main()
