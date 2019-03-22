# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614

import os

from page_sets.login_helpers import linkedin_login

from telemetry import story
from telemetry.page import page as page_module
from telemetry.page import shared_page_state
from telemetry.util import wpr_modes


class SkiaDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaDesktopPage, self).__init__(
        url=url,
        name=url,
        page_set=page_set,
        shared_page_state_class=shared_page_state.SharedDesktopPageState)
    self.archive_data_file = 'data/skia_linkedin_mobile.json'

  def RunNavigateSteps(self, action_runner):
    if self.wpr_mode != wpr_modes.WPR_REPLAY:
      credentials_path=os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                    'data/credentials.json')
      linkedin_login.LoginDesktopAccount(action_runner, 'linkedin',
                                         credentials_path)
      action_runner.Wait(15)
    action_runner.Navigate(self.url)
    action_runner.Wait(15)


class SkiaLinkedinDesktopPageSet(story.StorySet):
  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaLinkedinDesktopPageSet, self).__init__(
      archive_data_file='data/skia_linkedin_mobile.json')

    urls_list = [
      # go/skia-skps-3-2019
      'https://www.linkedin.com/in/linustorvalds',
    ]

    for url in urls_list:
      self.AddStory(SkiaDesktopPage(url, self))
