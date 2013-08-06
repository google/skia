#!/usr/bin/python

'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Generate an HTML file with links to view/download currently generated GM results
'''

# System-level imports
import argparse
import re
import urllib2

# Imports from local directory
import gm_json

IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)

class _InternalException(Exception):
  pass

class Generator(object):
  """ Generates an HTML file with links to view/download currently generated
  GM results. """

  def __init__(self, input_path, output_path=None):
    """
    params:
      input_path: filepath (or URL) pointing at JSON summary of actual results
      output_path: filepath to write HTML file to, or None to write to stdout
    """
    self._input_path = input_path
    self._output_path = output_path
    self._output_file = None

  def _GetFileContents(self, filepath):
    """ Returns the full contents of filepath, as a single string.
    If filepath looks like a URL, try to read it that way instead of as
    a path on local storage.
    Raises _InternalException if there is a problem.
    """
    if filepath.startswith('http:') or filepath.startswith('https:'):
      try:
        return urllib2.urlopen(filepath).read()
      except urllib2.HTTPError as e:
        raise _InternalException('unable to read URL %s: %s' % (filepath, e))
    else:
      return open(filepath, 'r').read()

  def _OpenOutput(self):
    """Prepare to write output to either stdout or a file.
    """
    if self._output_path:
      self._output_file = open(self._output_path, 'w')

  def _CloseOutput(self):
    """Close the output file, if necessary.
    """
    if self._output_file:
      self._output_file.close()
      self._output_file = None

  def _Output(self, string):
    """Write a string to the appropriate output destination (file or stdout).
    """
    if self._output_file:
      print >> self._output_file, string
    else:
      print string

  def Run(self):
    self._OpenOutput()
    self._Output('<html><head><title>%s</title></head><body>' % self._input_path)
    self._Output('Actual GM results from <a href="%s">%s</a><br>' % (
        self._input_path, self._input_path))
    results = self._GetFileContents(self._input_path)
    results_dict = gm_json.LoadFromString(results)
    actual_results = results_dict[gm_json.JSONKEY_ACTUALRESULTS]
    sections = actual_results.keys()
    for section in sections:
      self._Output('\n<br><br><br>\nSection: %s<br>' % section)
      section_results = actual_results[section]
      if section_results:
        image_names = sorted(section_results.keys())
        for image_name in image_names:
          image_results = section_results[image_name]
          image_url = gm_json.CreateGmActualUrl(
              test_name=IMAGE_FILENAME_RE.match(image_name).group(1),
              hash_type=image_results[0],
              hash_digest=image_results[1])
          self._Output('<a href="%s">%s</a><br>' % (image_url, image_name))
    self._Output('</body></html>')
    self._CloseOutput()

def Main():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      'input',
      help='filepath or URL containing a JSON summary of actual GM results')
  parser.add_argument(
      '--output',
      help='filepath to write HTML output to; by default, writes to stdout')
  args = parser.parse_args()
  generator = Generator(input_path=args.input, output_path=args.output)
  generator.Run()

if __name__ == '__main__':
  Main()
