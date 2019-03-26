# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614

import os

from page_sets.login_helpers import google_login

from telemetry import story
from telemetry.page import page as page_module
from telemetry.page import shared_page_state
from telemetry.util import wpr_modes


class SkiaBuildbotDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaBuildbotDesktopPage, self).__init__(
        url=url,
        name=url,
        page_set=page_set,
        shared_page_state_class=shared_page_state.SharedDesktopPageState)
    self.archive_data_file = 'data/skia_gmail_desktop.json'

  def RunSmoothness(self, action_runner):
    action_runner.ScrollElement()

  def RunNavigateSteps(self, action_runner):
    if self.wpr_mode != wpr_modes.WPR_REPLAY:
      credentials_path = os.path.join(
          os.path.dirname(os.path.abspath(__file__)), 'data/credentials.json')
      google_login.BaseLoginGoogle(action_runner, 'google', credentials_path)
      action_runner.Wait(10)
    action_runner.Navigate(self.url)
    action_runner.Wait(10)


class SkiaGmailDesktopPageSet(story.StorySet):
  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaGmailDesktopPageSet, self).__init__(
      archive_data_file='data/skia_gmail_desktop.json')

    urls_list = [
      # Why: productivity, top google properties, long email .
      'https://mail.google.com/mail/?shva=1#inbox/13ba91194d0b8a2e',
    ]

    for url in urls_list:
      self.AddStory(SkiaBuildbotDesktopPage(url, self))
