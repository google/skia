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
    self.archive_data_file = 'data/skia_samoasvg_desktop.json'

  def RunNavigateSteps(self, action_runner):
    action_runner.Navigate(self.url)
    action_runner.Wait(5)


class SkiaSamoasvgDesktopPageSet(story.StorySet):

  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaSamoasvgDesktopPageSet, self).__init__(
      archive_data_file='data/skia_samoasvg_desktop.json')

    urls_list = [
      # Why: from fmalita
      ('http://upload.wikimedia.org/wikipedia/commons/d/df/'
       'Seal_of_American_Samoa.svg'),
    ]

    for url in urls_list:
      self.AddStory(SkiaBuildbotDesktopPage(url, self))
