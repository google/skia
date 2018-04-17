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
BUILDER_ROLE_HOUSEKEEPER = 'Housekeeper'
BUILDER_ROLE_INFRA = 'Infra'
BUILDER_ROLE_PERF = 'Perf'
BUILDER_ROLE_TEST = 'Test'
BUILDER_ROLE_UPLOAD = 'Upload'
BUILDER_ROLE_CALMBENCH = "Calmbench"
BUILDER_ROLES = (BUILDER_ROLE_BUILD,
                 BUILDER_ROLE_HOUSEKEEPER,
                 BUILDER_ROLE_INFRA,
                 BUILDER_ROLE_PERF,
                 BUILDER_ROLE_TEST,
                 BUILDER_ROLE_UPLOAD,
                 BUILDER_ROLE_CALMBENCH)


def _LoadSchema():
  """ Load the builder naming schema from the JSON file. """

  def _UnicodeToStr(obj):
    """ Convert all unicode strings in obj to Python strings. """
    if isinstance(obj, unicode):
      return str(obj)
    elif isinstance(obj, dict):
      return dict(map(_UnicodeToStr, obj.iteritems()))
    elif isinstance(obj, list):
      return list(map(_UnicodeToStr, obj))
    elif isinstance(obj, tuple):
      return tuple(map(_UnicodeToStr, obj))

  builder_name_json_filename = os.path.join(
      os.path.dirname(__file__), 'builder_name_schema.json')
  builder_name_schema_json = json.load(open(builder_name_json_filename))

  global BUILDER_NAME_SCHEMA
  BUILDER_NAME_SCHEMA = _UnicodeToStr(
      builder_name_schema_json['builder_name_schema'])

  global BUILDER_NAME_SEP
  BUILDER_NAME_SEP = _UnicodeToStr(
      builder_name_schema_json['builder_name_sep'])

  # Since the builder roles are dictionary keys, just assert that the global
  # variables above account for all of them.
  assert len(BUILDER_ROLES) == len(BUILDER_NAME_SCHEMA)
  for role in BUILDER_ROLES:
    assert role in BUILDER_NAME_SCHEMA


_LoadSchema()


def MakeBuilderName(role, extra_config=None, **kwargs):
  schema = BUILDER_NAME_SCHEMA.get(role)
  if not schema:
    raise ValueError('%s is not a recognized role.' % role)
  for k, v in kwargs.iteritems():
    if BUILDER_NAME_SEP in v:
      raise ValueError('%s not allowed in %s.' % (BUILDER_NAME_SEP, v))
    if not k in schema:
      raise ValueError('Schema does not contain "%s": %s' %(k, schema))
  if extra_config and BUILDER_NAME_SEP in extra_config:
    raise ValueError('%s not allowed in %s.' % (BUILDER_NAME_SEP,
                                                extra_config))
  name_parts = [role]
  name_parts.extend([kwargs[attribute] for attribute in schema])
  if extra_config:
    name_parts.append(extra_config)
  return BUILDER_NAME_SEP.join(name_parts)


def DictForBuilderName(builder_name):
  """Makes a dictionary containing details about the builder from its name."""
  split = builder_name.split(BUILDER_NAME_SEP)

  def pop_front(items):
    try:
      return items.pop(0), items
    except:
      raise ValueError('Invalid builder name: %s (not enough parts)' % builder_name)

  result = {}

  def _parse(depth, role, parts):
    schema = BUILDER_NAME_SCHEMA.get(role)
    if not schema:
      raise ValueError('Invalid builder name: %s' % builder_name)
    if depth == 0:
      result['role'] = role
    else:
      result['sub-role-%d' % depth] = role
    for key in schema.get('keys', []):
      value, parts = pop_front(parts)
      result[key] = value
    for sub_role in schema.get('recurse_roles', []):
      if len(parts) > 0 and sub_role == parts[0]:
        parts = _parse(depth+1, parts[0], parts[1:])
    for key in schema.get('optional_keys', []):
      if parts:
        value, parts = pop_front(parts)
        result[key] = value
    if parts:
      raise ValueError('Invalid builder name: %s' % builder_name)
    return parts

  _parse(0, split[0], split[1:])

  return result
