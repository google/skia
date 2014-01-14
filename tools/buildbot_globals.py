#!/usr/bin/python

# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Provides read access to buildbot's global_variables.json .
"""


from contextlib import closing

import HTMLParser
import json
import re
import svn
import sys
import urllib2


_global_vars = None


GLOBAL_VARS_JSON_URL = ('https://skia.googlesource.com/buildbot/+'
                        '/master/site_config/global_variables.json')


class GlobalVarsRetrievalError(Exception):
  """Exception which is raised when the global_variables.json file cannot be
  retrieved from the Skia buildbot repository."""
  pass


class JsonDecodeError(Exception):
  """Exception which is raised when the global_variables.json file cannot be
  interpreted as JSON. This may be due to the file itself being incorrectly
  formatted or due to an incomplete or corrupted downloaded version of the file.
  """
  pass


class NoSuchGlobalVariable(KeyError):
  """Exception which is raised when a given variable is not found in the
  global_variables.json file."""
  pass


def retrieve_from_googlesource(url):
  """Retrieve the given file from GoogleSource's HTTP interface, trimming the
  extraneous HTML. Intended to be a GoogleSource equivalent of "svn cat".

  This just returns the unescaped contents of the first <pre> tag which matches
  our expectations for GoogleSource's HTTP interface. If that interface changes,
  this function will almost surely break.

  Args:
      url: string; the URL of the file to retrieve.
  Returns:
      The contents of the file in GoogleSource, stripped of the extra HTML from
          the HTML interface.
  """
  with closing(urllib2.urlopen(url)) as f:
    contents = f.read()
    pre_open = '<pre class="git-blob prettyprint linenums lang-(\w+)">'
    pre_close = '</pre>'
    matched_tag = re.search(pre_open, contents).group()
    start_index = contents.find(matched_tag)
    end_index = contents.find(pre_close)
    parser = HTMLParser.HTMLParser()
    return parser.unescape(contents[start_index + len(matched_tag):end_index])


def Get(var_name):
  """Return the value associated with this name in global_variables.json.
  
  Args:
      var_name: string; the variable to look up.
  Returns:
      The value of the variable.
  Raises:
      NoSuchGlobalVariable if there is no variable with that name.
  """
  global _global_vars
  if not _global_vars:
    try:
      global_vars_text = retrieve_from_googlesource(GLOBAL_VARS_JSON_URL)
    except Exception:
      raise GlobalVarsRetrievalError('Failed to retrieve %s.' %
                                     GLOBAL_VARS_JSON_URL)
    try:
      _global_vars = json.loads(global_vars_text)
    except ValueError as e:
      raise JsonDecodeError(e.message + '\n' + global_vars_text)
  try:
    return _global_vars[var_name]['value']
  except KeyError:
    raise NoSuchGlobalVariable(var_name)


if __name__ == '__main__':
  print Get(sys.argv[1])
