# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api

from . import builder_name_schema


class BuilderNameSchemaApi(recipe_api.RecipeApi):
  def __init__(self, *args, **kwargs):
    super(BuilderNameSchemaApi, self).__init__(*args, **kwargs)

    # See builder_name_schema.py for documentation.
    self.BUILDER_NAME_SCHEMA = builder_name_schema.BUILDER_NAME_SCHEMA
    self.BUILDER_NAME_SEP = builder_name_schema.BUILDER_NAME_SEP

    self.BUILDER_ROLE_BUILD = builder_name_schema.BUILDER_ROLE_BUILD
    self.BUILDER_ROLE_HOUSEKEEPER = builder_name_schema.BUILDER_ROLE_HOUSEKEEPER
    self.BUILDER_ROLE_INFRA = builder_name_schema.BUILDER_ROLE_INFRA
    self.BUILDER_ROLE_PERF = builder_name_schema.BUILDER_ROLE_PERF
    self.BUILDER_ROLE_TEST = builder_name_schema.BUILDER_ROLE_TEST
    self.BUILDER_ROLES = builder_name_schema.BUILDER_ROLES

  def MakeBuilderName(self, **kwargs):
    return builder_name_schema.MakeBuilderName(**kwargs)

  def DictForBuilderName(self, *args, **kwargs):
    return builder_name_schema.DictForBuilderName(*args, **kwargs)
