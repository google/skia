#!/usr/bin/python

'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Gathers diffs between 2 JSON expectations files, or between actual and
expected results within a single JSON actual-results file,
and generates an old-vs-new diff dictionary.

TODO(epoger): Fix indentation in this file (2-space indents, not 4-space).
'''

# System-level imports
import argparse
import json
import os
import sys
import urllib2

# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
#
# This assumes that the 'gm' directory has been checked out as a sibling of
# the 'tools' directory containing this script, which will be the case if
# 'trunk' was checked out as a single unit.
GM_DIRECTORY = os.path.realpath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gm'))
if GM_DIRECTORY not in sys.path:
    sys.path.append(GM_DIRECTORY)
import gm_json


# Object that generates diffs between two JSON gm result files.
class GMDiffer(object):

    def __init__(self):
        pass

    def _GetFileContentsAsString(self, filepath):
        """Returns the full contents of a file, as a single string.
        If the filename looks like a URL, download its contents.
        If the filename is None, return None."""
        if filepath is None:
            return None
        elif filepath.startswith('http:') or filepath.startswith('https:'):
            return urllib2.urlopen(filepath).read()
        else:
            return open(filepath, 'r').read()

    def _GetExpectedResults(self, contents):
        """Returns the dictionary of expected results from a JSON string,
        in this form:

        {
          'test1' : 14760033689012826769,
          'test2' : 9151974350149210736,
          ...
        }

        We make these simplifying assumptions:
        1. Each test has either 0 or 1 allowed results.
        2. All expectations are of type JSONKEY_HASHTYPE_BITMAP_64BITMD5.

        Any tests which violate those assumptions will cause an exception to
        be raised.

        Any tests for which we have no expectations will be left out of the
        returned dictionary.
        """
        result_dict = {}
        json_dict = gm_json.LoadFromString(contents)
        all_expectations = json_dict[gm_json.JSONKEY_EXPECTEDRESULTS]

        # Prevent https://code.google.com/p/skia/issues/detail?id=1588
        # ('svndiff.py: 'NoneType' object has no attribute 'keys'')
        if not all_expectations:
            return result_dict

        for test_name in all_expectations.keys():
            test_expectations = all_expectations[test_name]
            allowed_digests = test_expectations[
                gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS]
            if allowed_digests:
                num_allowed_digests = len(allowed_digests)
                if num_allowed_digests > 1:
                    raise ValueError(
                        'test %s has %d allowed digests' % (
                            test_name, num_allowed_digests))
                digest_pair = allowed_digests[0]
                if digest_pair[0] != gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5:
                    raise ValueError(
                        'test %s has unsupported hashtype %s' % (
                            test_name, digest_pair[0]))
                result_dict[test_name] = digest_pair[1]
        return result_dict

    def _GetActualResults(self, contents):
        """Returns the dictionary of actual results from a JSON string,
        in this form:

        {
          'test1' : 14760033689012826769,
          'test2' : 9151974350149210736,
          ...
        }

        We make these simplifying assumptions:
        1. All results are of type JSONKEY_HASHTYPE_BITMAP_64BITMD5.

        Any tests which violate those assumptions will cause an exception to
        be raised.

        Any tests for which we have no actual results will be left out of the
        returned dictionary.
        """
        result_dict = {}
        json_dict = gm_json.LoadFromString(contents)
        all_result_types = json_dict[gm_json.JSONKEY_ACTUALRESULTS]
        for result_type in all_result_types.keys():
            results_of_this_type = all_result_types[result_type]
            if results_of_this_type:
                for test_name in results_of_this_type.keys():
                    digest_pair = results_of_this_type[test_name]
                    if digest_pair[0] != gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5:
                        raise ValueError(
                            'test %s has unsupported hashtype %s' % (
                                test_name, digest_pair[0]))
                    result_dict[test_name] = digest_pair[1]
        return result_dict

    def _DictionaryDiff(self, old_dict, new_dict):
        """Generate a dictionary showing the diffs between old_dict and new_dict.
        Any entries which are identical across them will be left out."""
        diff_dict = {}
        all_keys = set(old_dict.keys() + new_dict.keys())
        for key in all_keys:
            if old_dict.get(key) != new_dict.get(key):
                new_entry = {}
                new_entry['old'] = old_dict.get(key)
                new_entry['new'] = new_dict.get(key)
                diff_dict[key] = new_entry
        return diff_dict

    def GenerateDiffDict(self, oldfile, newfile=None):
        """Generate a dictionary showing the diffs:
        old = expectations within oldfile
        new = expectations within newfile

        If newfile is not specified, then 'new' is the actual results within
        oldfile.
        """
        return self.GenerateDiffDictFromStrings(self._GetFileContentsAsString(oldfile),
                                                self._GetFileContentsAsString(newfile))

    def GenerateDiffDictFromStrings(self, oldjson, newjson=None):
        """Generate a dictionary showing the diffs:
        old = expectations within oldjson
        new = expectations within newjson

        If newfile is not specified, then 'new' is the actual results within
        oldfile.
        """
        old_results = self._GetExpectedResults(oldjson)
        if newjson:
            new_results = self._GetExpectedResults(newjson)
        else:
            new_results = self._GetActualResults(oldjson)
        return self._DictionaryDiff(old_results, new_results)


def _Main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'old',
        help='Path to JSON file whose expectations to display on ' +
        'the "old" side of the diff. This can be a filepath on ' +
        'local storage, or a URL.')
    parser.add_argument(
        'new', nargs='?',
        help='Path to JSON file whose expectations to display on ' +
        'the "new" side of the diff; if not specified, uses the ' +
        'ACTUAL results from the "old" JSON file. This can be a ' +
        'filepath on local storage, or a URL.')
    args = parser.parse_args()
    differ = GMDiffer()
    diffs = differ.GenerateDiffDict(oldfile=args.old, newfile=args.new)
    json.dump(diffs, sys.stdout, sort_keys=True, indent=2)


if __name__ == '__main__':
    _Main()
