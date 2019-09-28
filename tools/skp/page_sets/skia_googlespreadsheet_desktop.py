# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614


from telemetry import story
from telemetry.page import page as page_module
from telemetry.page import shared_page_state


class SkiaBuildbotDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaBuildbotDesktopPage, self).__init__(
        url=url,
        name=url,
        page_set=page_set,
        shared_page_state_class=shared_page_state.SharedDesktopPageState)
    self.archive_data_file = 'data/skia_googlespreadsheet_desktop.json'


class SkiaGooglespreadsheetDesktopPageSet(story.StorySet):

  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaGooglespreadsheetDesktopPageSet, self).__init__(
      archive_data_file='data/skia_googlespreadsheet_desktop.json')

    urls_list = [
      # Why: from Tom W's list.
      ('https://docs.google.com/spreadsheets/d/'
       '1YnmSPu-p-1nj-lkWd8q_GRgzjiWzg_6A-HvFYqVoVxI/edit#gid=0'),
    ]

    for url in urls_list:
      self.AddStory(SkiaBuildbotDesktopPage(url, self))
