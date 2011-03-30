#!/usr/bin/python

# Copyright (c) 2009 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Handle version information related to Visual Stuio."""

import os
import re
import subprocess
import sys


class VisualStudioVersion:
  """Information regarding a version of Visual Studio."""

  def __init__(self, short_name, description,
               solution_version, project_version, flat_sln, uses_vcxproj):
    self.short_name = short_name
    self.description = description
    self.solution_version = solution_version
    self.project_version = project_version
    self.flat_sln = flat_sln
    self.uses_vcxproj = uses_vcxproj

  def ShortName(self):
    return self.short_name

  def Description(self):
    """Get the full description of the version."""
    return self.description

  def SolutionVersion(self):
    """Get the version number of the sln files."""
    return self.solution_version

  def ProjectVersion(self):
    """Get the version number of the vcproj or vcxproj files."""
    return self.project_version

  def FlatSolution(self):
    return self.flat_sln

  def UsesVcxproj(self):
    """Returns true if this version uses a vcxproj file."""
    return self.uses_vcxproj

  def ProjectExtension(self):
    """Returns the file extension for the project."""
    return self.uses_vcxproj and '.vcxproj' or '.vcproj'

def _RegistryGetValue(key, value):
  """Use reg.exe to read a paricular key.

  While ideally we might use the win32 module, we would like gyp to be
  python neutral, so for instance cygwin python lacks this module.

  Arguments:
    key: The registry key to read from.
    value: The particular value to read.
  Return:
    The contents there, or None for failure.
  """
  # Skip if not on Windows.
  if sys.platform not in ('win32', 'cygwin'):
    return None
  # Run reg.exe.
  cmd = [os.path.join(os.environ.get('WINDIR', ''), 'System32', 'reg.exe'),
         'query', key, '/v', value]
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  text = p.communicate()[0]
  # Require a successful return value.
  if p.returncode:
    return None
  # Extract value.
  match = re.search(r'REG_\w+\s+([^\r]+)\r\n', text)
  if not match:
    return None
  return match.group(1)


def _RegistryKeyExists(key):
  """Use reg.exe to see if a key exists.

  Args:
    key: The registry key to check.
  Return:
    True if the key exists
  """
  # Skip if not on Windows.
  if sys.platform not in ('win32', 'cygwin'):
    return None
  # Run reg.exe.
  cmd = [os.path.join(os.environ.get('WINDIR', ''), 'System32', 'reg.exe'),
         'query', key]
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  return p.returncode == 0


def _CreateVersion(name):
  versions = {
      '2010': VisualStudioVersion('2010',
                                  'Visual Studio 2010',
                                  solution_version='11.00',
                                  project_version='4.0',
                                  flat_sln=False,
                                  uses_vcxproj=True),
      '2008': VisualStudioVersion('2008',
                                  'Visual Studio 2008',
                                  solution_version='10.00',
                                  project_version='9.00',
                                  flat_sln=False,
                                  uses_vcxproj=False),
      '2008e': VisualStudioVersion('2008e',
                                   'Visual Studio 2008',
                                   solution_version='10.00',
                                   project_version='9.00',
                                   flat_sln=True,
                                   uses_vcxproj=False),
      '2005': VisualStudioVersion('2005',
                                  'Visual Studio 2005',
                                  solution_version='9.00',
                                  project_version='8.00',
                                  flat_sln=False,
                                  uses_vcxproj=False),
      '2005e': VisualStudioVersion('2005e',
                                   'Visual Studio 2005',
                                   solution_version='9.00',
                                   project_version='8.00',
                                   flat_sln=True,
                                   uses_vcxproj=False),
  }
  return versions[str(name)]


def _DetectVisualStudioVersions():
  """Collect the list of installed visual studio versions.

  Returns:
    A list of visual studio versions installed in descending order of
    usage preference.
    Base this on the registry and a quick check if devenv.exe exists.
    Only versions 8-10 are considered.
    Possibilities are:
      2005 - Visual Studio 2005 (8)
      2008 - Visual Studio 2008 (9)
      2010 - Visual Studio 2010 (10)
  """
  version_to_year = {'8.0': '2005', '9.0': '2008', '10.0': '2010'}
  versions = []
  # For now, prefer versions before VS2010
  for version in ('9.0', '8.0', '10.0'):
    # Check if VS2010 and later is installed as specified by
    # http://msdn.microsoft.com/en-us/library/bb164659.aspx
    key32 = r'HKLM\SOFTWARE\Microsoft\DevDiv\VS\Servicing\%s' % version
    key64 = r'HKLM\SOFTWARE\Wow6432Node\Microsoft\DevDiv\VS\Servicing\%sD' % (
         version)
    if _RegistryKeyExists(key32) or _RegistryKeyExists(key64):
      # Add this one.
      # TODO(jeanluc) This does not check for an express version.
      versions.append(_CreateVersion(version_to_year[version]))
      continue
    # Get the install dir for this version.
    key = r'HKLM\Software\Microsoft\VisualStudio\%s' % version
    path = _RegistryGetValue(key, 'InstallDir')
    if not path:
      continue
    # Check for full.
    if os.path.exists(os.path.join(path, 'devenv.exe')):
      # Add this one.
      versions.append(_CreateVersion(version_to_year[version]))
    # Check for express.
    elif os.path.exists(os.path.join(path, 'vcexpress.exe')):
      # Add this one.
      versions.append(_CreateVersion(version_to_year[version] + 'e'))
  return versions


def SelectVisualStudioVersion(version='auto'):
  """Select which version of Visual Studio projects to generate.

  Arguments:
    version: Hook to allow caller to force a particular version (vs auto).
  Returns:
    An object representing a visual studio project format version.
  """
  # In auto mode, check environment variable for override.
  if version == 'auto':
    version = os.environ.get('GYP_MSVS_VERSION', 'auto')
  # In auto mode, pick the most preferred version present.
  if version == 'auto':
    versions = _DetectVisualStudioVersions()
    if not versions:
      # Default to 2005.
      return _CreateVersion('2005')
    return versions[0]
  # Convert version string into a version object.
  return _CreateVersion(version)
