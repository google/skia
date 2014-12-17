# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614


from telemetry.page import page as page_module
from telemetry.page import page_set as page_set_module


class SkiaBuildbotDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaBuildbotDesktopPage, self).__init__(
        url=url,
        page_set=page_set,
        credentials_path='data/credentials.json')
    self.user_agent_type = 'desktop'
    self.archive_data_file = 'data/skia_mapsvg_desktop.json'


class SkiaMapsvgDesktopPageSet(page_set_module.PageSet):

  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaMapsvgDesktopPageSet, self).__init__(
      user_agent_type='desktop',
      archive_data_file='data/skia_mapsvg_desktop.json')

    urls_list = [
      # Why: from fmalita
      ('http://upload.wikimedia.org/wikipedia/commons/6/63/'
       'A_large_blank_world_map_with_oceans_marked_in_blue.svg'),
    ]

    for url in urls_list:
      self.AddUserStory(SkiaBuildbotDesktopPage(url, self))
