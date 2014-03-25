#!/usr/bin/python2

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Skia's Chromium Codereview Comparison Script.

This script takes two Codereview URLs, looks at the trybot results for
the two codereviews and compares the results.

Usage:
  compare_codereview.py CONTROL_URL ROLL_URL
"""

import collections
import os
import re
import sys
import urllib2
import HTMLParser


class CodeReviewHTMLParser(HTMLParser.HTMLParser):
  """Parses CodeReview web page.

  Use the CodeReviewHTMLParser.parse static function to make use of
  this class.

  This uses the HTMLParser class because it's the best thing in
  Python's standard library.  We need a little more power than a
  regex.  [Search for "You can't parse [X]HTML with regex." for more
  information.
  """
  # pylint: disable=I0011,R0904
  @staticmethod
  def parse(url):
    """Parses a CodeReview web pages.

    Args:
      url (string), a codereview URL like this:
        'https://codereview.chromium.org/?????????'.

    Returns:
      A dictionary; the keys are bot_name strings, the values
      are CodeReviewHTMLParser.Status objects
    """
    parser = CodeReviewHTMLParser()
    try:
      parser.feed(urllib2.urlopen(url).read())
    except (urllib2.URLError,):
      print >> sys.stderr, 'Error getting', url
      return None
    parser.close()
    return parser.statuses

  # namedtuples are like lightweight structs in Python.  The low
  # overhead of a tuple, but the ease of use of an object.
  Status = collections.namedtuple('Status', ['status', 'url'])

  def __init__(self):
    HTMLParser.HTMLParser.__init__(self)
    self._id = None
    self._status = None
    self._href = None
    self._anchor_data = ''
    self._currently_parsing_trybotdiv = False
    # statuses is a dictionary of CodeReviewHTMLParser.Status
    self.statuses = {}

  def handle_starttag(self, tag, attrs):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to handle the start of a tag
    (e.g. <div id="main">).

    The tag argument is the name of the tag converted to lower
    case. The attrs argument is a list of (name, value) pairs
    containing the attributes found inside the tag's <>
    brackets. The name will be translated to lower case, and
    quotes in the value have been removed, and character and
    entity references have been replaced.

    For instance, for the tag <A HREF="http://www.cwi.nl/">, this
    method would be called as handle_starttag('a', [('href',
    'http://www.cwi.nl/')]).
    [[end standard library documentation]]
    """
    attrs = dict(attrs)
    if tag == 'div':
      # We are looking for <div id="tryjobdiv*">.
      id_attr = attrs.get('id','')
      if id_attr.startswith('tryjobdiv'):
        self._id = id_attr
    if (self._id and tag == 'a'
      and 'build-result' in attrs.get('class', '').split()):
      # If we are already inside a <div id="tryjobdiv*">, we
      # look for a link if the form
      # <a class="build-result" href="*">.  Then we save the
      # (non-standard) status attribute and the URL.
      self._status = attrs.get('status')
      self._href = attrs.get('href')
      self._currently_parsing_trybotdiv = True
      # Start saving anchor data.

  def handle_data(self, data):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to process arbitrary data (e.g. text
    nodes and the content of <script>...</script> and
    <style>...</style>).
    [[end standard library documentation]]
    """
    # Save the text inside the <a></a> tags.  Assume <a> tags
    # aren't nested.
    if self._currently_parsing_trybotdiv:
      self._anchor_data += data

  def handle_endtag(self, tag):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to handle the end tag of an element
    (e.g. </div>).  The tag argument is the name of the tag
    converted to lower case.
    [[end standard library documentation]]
    """
    if tag == 'a' and self._status:
      # We take the accumulated self._anchor_data and save it as
      # the bot name.
      bot = self._anchor_data.strip()
      stat = CodeReviewHTMLParser.Status(status=self._status,
                         url=self._href)
      if bot:
        # Add to accumulating dictionary.
        self.statuses[bot] = stat
      # Reset state to search for the next bot.
      self._currently_parsing_trybotdiv = False
      self._anchor_data = ''
      self._status = None
      self._href = None


class BuilderHTMLParser(HTMLParser.HTMLParser):
  """parses Trybot web pages.

  Use the BuilderHTMLParser.parse static function to make use of
  this class.

  This uses the HTMLParser class because it's the best thing in
  Python's standard library.  We need a little more power than a
  regex.  [Search for "You can't parse [X]HTML with regex." for more
  information.
  """
  # pylint: disable=I0011,R0904
  @staticmethod
  def parse(url):
    """Parses a Trybot web page.

    Args:
      url (string), a trybot result URL.

    Returns:
      An array of BuilderHTMLParser.Results, each a description
      of failure results, along with an optional url
    """
    parser = BuilderHTMLParser()
    try:
      parser.feed(urllib2.urlopen(url).read())
    except (urllib2.URLError,):
      print >> sys.stderr, 'Error getting', url
      return []
    parser.close()
    return parser.failure_results

  Result = collections.namedtuple('Result', ['text', 'url'])

  def __init__(self):
    HTMLParser.HTMLParser.__init__(self)
    self.failure_results = []
    self._current_failure_result = None
    self._divlevel = None
    self._li_level = 0
    self._li_data = ''
    self._current_failure = False
    self._failure_results_url = ''

  def handle_starttag(self, tag, attrs):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to handle the start of a tag
    (e.g. <div id="main">).

    The tag argument is the name of the tag converted to lower
    case. The attrs argument is a list of (name, value) pairs
    containing the attributes found inside the tag's <>
    brackets. The name will be translated to lower case, and
    quotes in the value have been removed, and character and
    entity references have been replaced.

    For instance, for the tag <A HREF="http://www.cwi.nl/">, this
    method would be called as handle_starttag('a', [('href',
    'http://www.cwi.nl/')]).
    [[end standard library documentation]]
    """
    attrs = dict(attrs)
    if tag == 'li':
      # <li> tags can be nested.  So we have to count the
      # nest-level for backing out.
      self._li_level += 1
      return
    if tag == 'div' and attrs.get('class') == 'failure result':
      # We care about this sort of thing:
      # <li>
      #   <li>
      #   <li>
      #     <div class="failure result">...</div>
      #   </li>
      #   </li>
      #   We want this text here.
      # </li>
      if self._li_level > 0:
        self._current_failure = True  # Tells us to keep text.
      return

    if tag == 'a' and self._current_failure:
      href = attrs.get('href')
      # Sometimes we want to keep the stdio url.  We always
      # return it, just in case.
      if href.endswith('/logs/stdio'):
        self._failure_results_url = href

  def handle_data(self, data):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to process arbitrary data (e.g. text
    nodes and the content of <script>...</script> and
    <style>...</style>).
    [[end standard library documentation]]
    """
    if self._current_failure:
      self._li_data += data

  def handle_endtag(self, tag):
    """Overrides the HTMLParser method to implement functionality.

    [[begin standard library documentation]]
    This method is called to handle the end tag of an element
    (e.g. </div>).  The tag argument is the name of the tag
    converted to lower case.
    [[end standard library documentation]]
    """
    if tag == 'li':
      self._li_level -= 1
      if 0 == self._li_level:
        if self._current_failure:
          result = self._li_data.strip()
          first = result.split()[0]
          if first:
            result = re.sub(
              r'^%s(\s+%s)+' % (first, first), first, result)
            # Sometimes, it repeats the same thing
            # multiple times.
          result = re.sub(r'unexpected flaky.*', '', result)
          # Remove some extra unnecessary text.
          result = re.sub(r'\bpreamble\b', '', result)
          result = re.sub(r'\bstdio\b', '', result)
          url = self._failure_results_url
          self.failure_results.append(
            BuilderHTMLParser.Result(result, url))
          self._current_failure_result = None
        # Reset the state.
        self._current_failure = False
        self._li_data = ''
        self._failure_results_url = ''


def printer(indent, string):
  """Print indented, wrapped text.
  """
  def wrap_to(line, columns):
    """Wrap a line to the given number of columns, return a list
    of strings.
    """
    ret = []
    nextline = ''
    for word in line.split():
      if nextline:
        if len(nextline) + 1 + len(word) > columns:
          ret.append(nextline)
          nextline = word
        else:
          nextline += (' ' + word)
      else:
        nextline = word
    if nextline:
      ret.append(nextline)
    return ret
  out = sys.stdout
  spacer = '  '
  for line in string.split('\n'):
    for i, wrapped_line in enumerate(wrap_to(line, 68 - (2 * indent))):
      out.write(spacer * indent)
      if i > 0:
        out.write(spacer)
      out.write(wrapped_line)
      out.write('\n')
  out.flush()


def main(control_url, roll_url, verbosity=1):
  """Compare two Codereview URLs

  Args:
    control_url, roll_url: (strings) URL of the format
      https://codereview.chromium.org/?????????

    verbosity: (int) verbose level.  0, 1, or 2.
  """
  # pylint: disable=I0011,R0914,R0912
  control = CodeReviewHTMLParser.parse(control_url)
  roll = CodeReviewHTMLParser.parse(roll_url)
  all_bots = set(control) & set(roll)  # Set intersection.
  if not all_bots:
    print >> sys.stderr, (
      'Error:  control %s and roll %s have no common trybots.'
      % (list(control), list(roll)))
    return

  control_name = '[control %s]' % control_url.split('/')[-1]
  roll_name = '[roll %s]' % roll_url.split('/')[-1]

  out = sys.stdout

  for bot in sorted(all_bots):
    if (roll[bot].status == 'success'):
      if verbosity > 1:
        printer(0, '==%s==' % bot)
        printer(1, 'OK')
      continue

    if control[bot].status != 'failure' and roll[bot].status != 'failure':
      continue
    printer(0, '==%s==' % bot)

    formatted_results = []
    for (status, name, url) in [
            (control[bot].status, control_name, control[bot].url),
            (   roll[bot].status,    roll_name,    roll[bot].url)]:
      lines = []
      if status == 'failure':
        results = BuilderHTMLParser.parse(url)
        for result in results:
          formatted_result = re.sub(r'(\S*\.html) ', '\n__\g<1>\n', result.text)
          # Strip runtimes.
          formatted_result = re.sub(r'\(.*\)', '', formatted_result)
          lines.append((2, formatted_result))
          if ('compile' in result.text or '...and more' in result.text):
            lines.append((3, re.sub('/[^/]*$', '/', url) + result.url))
      formatted_results.append(lines)

    identical = formatted_results[0] == formatted_results[1]


    for (formatted_result, (status, name, url)) in zip(
        formatted_results,
        [(control[bot].status, control_name, control[bot].url),
          (roll[bot].status,  roll_name,  roll[bot].url)]):
      if status != 'failure' and not identical:
        printer(1, name)
        printer(2, status)
      elif status == 'failure':
        if identical:
          printer(1, control_name + ' and ' + roll_name + ' failed identically')
        else:
          printer(1, name)
        for (indent, line) in formatted_result:
          printer(indent, line)
        if identical:
          break
    out.write('\n')

  if verbosity > 0:
    # Print out summary of all of the bots.
    out.write('%11s %11s %4s %s\n\n' %
          ('CONTROL', 'ROLL', 'DIFF', 'BOT'))
    for bot in sorted(all_bots):
      if roll[bot].status == 'success':
        diff = ''
      elif (control[bot].status == 'success' and
           roll[bot].status == 'failure'):
        diff = '!!!!'
      elif ('pending' in control[bot].status or
          'pending' in roll[bot].status):
        diff = '....'
      else:
        diff = '****'
      out.write('%11s %11s %4s %s\n' % (
          control[bot].status, roll[bot].status, diff, bot))
    out.write('\n')
    out.flush()

if __name__ == '__main__':
  if len(sys.argv) < 3:
    print >> sys.stderr, __doc__
    exit(1)
  main(sys.argv[1], sys.argv[2],
     int(os.environ.get('COMPARE_CODEREVIEW_VERBOSITY', 1)))

