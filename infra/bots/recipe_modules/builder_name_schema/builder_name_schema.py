# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


""" Utilities for dealing with builder names. This module obtains its attributes
dynamically from builder_name_schema.json. """


import json
import os


# All of these global variables are filled in by _LoadSchema().

# The full schema.
BUILDER_NAME_SCHEMA = None

# Character which separates parts of a builder name.
BUILDER_NAME_SEP = None

# Builder roles.
BUILDER_ROLE_BUILD = 'Build'
BUILDER_ROLE_BUILDSTATS = 'BuildStats'
BUILDER_ROLE_CANARY = 'Canary'
BUILDER_ROLE_HOUSEKEEPER = 'Housekeeper'
BUILDER_ROLE_INFRA = 'Infra'
BUILDER_ROLE_PERF = 'Perf'
BUILDER_ROLE_TEST = 'Test'
BUILDER_ROLE_FM = 'FM'
BUILDER_ROLE_UPLOAD = 'Upload'
BUILDER_ROLES = (BUILDER_ROLE_BUILD,
                 BUILDER_ROLE_BUILDSTATS,
                 BUILDER_ROLE_CANARY,
                 BUILDER_ROLE_HOUSEKEEPER,
                 BUILDER_ROLE_INFRA,
                 BUILDER_ROLE_PERF,
                 BUILDER_ROLE_TEST,
                 BUILDER_ROLE_FM,
                 BUILDER_ROLE_UPLOAD)


def _LoadSchema():
  """ Load the builder naming schema from the JSON file. """

  def ToStr(obj):
    """ Convert all unicode strings in obj to Python strings. """
    if isinstance(obj, str):
      return obj  # pragma: nocover
    elif isinstance(obj, dict):
      return dict(map(ToStr, obj.items()))
    elif isinstance(obj, list):
      return list(map(ToStr, obj))
    elif isinstance(obj, tuple):
      return tuple(map(ToStr, obj))
    else:
      return obj.decode('utf-8')

  builder_name_json_filename = os.path.join(
      os.path.dirname(__file__), 'builder_name_schema.json')
  builder_name_schema_json = json.load(open(builder_name_json_filename))

  global BUILDER_NAME_SCHEMA
  BUILDER_NAME_SCHEMA = ToStr(
      builder_name_schema_json['builder_name_schema'])

  global BUILDER_NAME_SEP
  BUILDER_NAME_SEP = ToStr(
      builder_name_schema_json['builder_name_sep'])

  # Since the builder roles are dictionary keys, just assert that the global
  # variables above account for all of them.
  assert len(BUILDER_ROLES) == len(BUILDER_NAME_SCHEMA)
  for role in BUILDER_ROLES:
    assert role in BUILDER_NAME_SCHEMA


_LoadSchema()


def MakeBuilderName(**parts):
  for v in parts.values():
    if BUILDER_NAME_SEP in v:
      raise ValueError('Parts cannot contain "%s"' % BUILDER_NAME_SEP)

  rv_parts = []

  def process(depth, parts):
    role_key = 'role'
    if depth != 0:
      role_key = 'sub-role-%d' % depth
    role = parts.get(role_key)
    if not role:
      raise ValueError('Invalid parts; missing key %s' % role_key)
    s = BUILDER_NAME_SCHEMA.get(role)
    if not s:
      raise ValueError('Invalid parts; unknown role %s' % role)
    rv_parts.append(role)
    del parts[role_key]

    for key in s.get('keys', []):
      value = parts.get(key)
      if not value:
        raise ValueError('Invalid parts; missing %s' % key)
      rv_parts.append(value)
      del parts[key]

    recurse_roles = s.get('recurse_roles', [])
    if len(recurse_roles) > 0:
      sub_role_key = 'sub-role-%d' % (depth+1)
      sub_role = parts.get(sub_role_key)
      if not sub_role:
        raise ValueError('Invalid parts; missing %s' % sub_role_key)

      found = False
      for recurse_role in recurse_roles:
        if recurse_role == sub_role:
          found = True
          parts = process(depth+1, parts)
          break
      if not found:
        raise ValueError('Invalid parts; unknown sub-role %s' % sub_role)

    for key in s.get('optional_keys', []):
      if parts.get(key):
        rv_parts.append(parts[key])
        del parts[key]

    if len(parts) > 0:
      raise ValueError('Invalid parts; too many parts: %s' % parts)

    return parts

  process(0, parts)

  return BUILDER_NAME_SEP.join(rv_parts)


def DictForBuilderName(builder_name):
  """Makes a dictionary containing details about the builder from its name."""
  split = builder_name.split(BUILDER_NAME_SEP)

  def pop_front(items):
    try:
      return items.pop(0), items
    except:
      raise ValueError(
          'Invalid builder name: %s (not enough parts)' % builder_name)

  result = {}

  def _parse(depth, role, parts):
    schema = BUILDER_NAME_SCHEMA.get(role)
    if not schema:
      raise ValueError('Invalid builder name: %s' % builder_name)
    if depth == 0:
      result['role'] = str(role)
    else:
      result['sub-role-%d' % depth] = str(role)
    for key in schema.get('keys', []):
      value, parts = pop_front(parts)
      result[key] = str(value)
    for sub_role in schema.get('recurse_roles', []):
      if len(parts) > 0 and sub_role == parts[0]:
        parts = _parse(depth+1, parts[0], parts[1:])
    for key in schema.get('optional_keys', []):
      if parts:
        value, parts = pop_front(parts)
        result[key] = str(value)
    if parts:
      raise ValueError('Invalid builder name: %s' % builder_name)
    return parts

  _parse(0, split[0], split[1:])

  return result
