# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614


from telemetry import story
from telemetry.page import page as page_module
from telemetry.page import shared_page_state


class SkiaMobilePage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaMobilePage, self).__init__(
        url=url,
        name=url,
        page_set=page_set,
        shared_page_state_class=shared_page_state.SharedMobilePageState)
    self.archive_data_file = 'data/skia_cnnarticle_mobile.json'

  def RunNavigateSteps(self, action_runner):
    action_runner.Navigate(self.url)
    action_runner.Wait(15)


class SkiaCnnarticleMobilePageSet(story.StorySet):
  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaCnnarticleMobilePageSet, self).__init__(
      archive_data_file='data/skia_cnnarticle_mobile.json')

    urls_list = [
      # go/skia-skps-3-2019
      ('https://www.cnn.com/2019/05/16/entertainment/'
       'the-big-bang-theory-series-finale-review/index.html'),
    ]

    for url in urls_list:
      self.AddStory(SkiaMobilePage(url, self))
