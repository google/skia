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
BUILDER_ROLE_CANARY = 'Canary'
BUILDER_ROLE_BUILD = 'Build'
BUILDER_ROLE_HOUSEKEEPER = 'Housekeeper'
BUILDER_ROLE_INFRA = 'Infra'
BUILDER_ROLE_PERF = 'Perf'
BUILDER_ROLE_TEST = 'Test'
BUILDER_ROLES = (BUILDER_ROLE_CANARY,
                 BUILDER_ROLE_BUILD,
                 BUILDER_ROLE_HOUSEKEEPER,
                 BUILDER_ROLE_INFRA,
                 BUILDER_ROLE_PERF,
                 BUILDER_ROLE_TEST)


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
  split_name = builder_name.split(BUILDER_NAME_SEP)

  def pop_front():
    try:
      return split_name.pop(0)
    except:
      raise ValueError('Invalid builder name: %s' % builder_name)

  result = {}
  if split_name[0] in BUILDER_NAME_SCHEMA.keys():
    key_list = BUILDER_NAME_SCHEMA[split_name[0]]
    result['role'] = pop_front()
    for key in key_list:
      result[key] = pop_front()
    if split_name:
      result['extra_config'] = pop_front()
    if split_name:
      raise ValueError('Invalid builder name: %s' % builder_name)
  else:
    raise ValueError('Invalid builder name: %s' % builder_name)
  return result


