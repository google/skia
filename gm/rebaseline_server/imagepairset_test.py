#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test imagepairset.py
"""

# System-level imports
import unittest

# Local imports
import column
import imagepair
import imagepairset


BASE_URL_1 = 'http://base/url/1'
BASE_URL_2 = 'http://base/url/2'
IMAGEPAIR_1_AS_DICT = {
    imagepair.KEY__EXTRA_COLUMN_VALUES: {
        'builder': 'MyBuilder',
        'test': 'test1',
    },
    imagepair.KEY__IMAGE_A_URL: 'test1/1111.png',
    imagepair.KEY__IMAGE_B_URL: 'test1/1111.png',
    imagepair.KEY__IS_DIFFERENT: False,
}
IMAGEPAIR_2_AS_DICT = {
    imagepair.KEY__DIFFERENCE_DATA: {
        'maxDiffPerChannel': [1, 2, 3],
        'numDifferingPixels': 111,
        'percentDifferingPixels': 22.222,
        'weightedDiffMeasure': 33.333,
    },
    imagepair.KEY__EXTRA_COLUMN_VALUES: {
        'builder': 'MyBuilder',
        'test': 'test2',
    },
    imagepair.KEY__IMAGE_A_URL: 'test2/2222.png',
    imagepair.KEY__IMAGE_B_URL: 'test2/22223.png',
    imagepair.KEY__IS_DIFFERENT: True,
}
IMAGEPAIR_3_AS_DICT = {
    imagepair.KEY__DIFFERENCE_DATA: {
        'maxDiffPerChannel': [4, 5, 6],
        'numDifferingPixels': 111,
        'percentDifferingPixels': 44.444,
        'weightedDiffMeasure': 33.333,
    },
    imagepair.KEY__EXPECTATIONS_DATA: {
        'bugs': [1001, 1002],
        'ignoreFailure': True,
    },
    imagepair.KEY__EXTRA_COLUMN_VALUES: {
        'builder': 'MyBuilder',
        'test': 'test3',
    },
    imagepair.KEY__IMAGE_A_URL: 'test3/3333.png',
    imagepair.KEY__IMAGE_B_URL: 'test3/33334.png',
    imagepair.KEY__IS_DIFFERENT: True,
}
SET_A_DESCRIPTION = 'expectations'
SET_B_DESCRIPTION = 'actuals'


class ImagePairSetTest(unittest.TestCase):

  def setUp(self):
    self.maxDiff = None  # do not truncate diffs when tests fail

  def shortDescription(self):
    """Tells unittest framework to not print docstrings for test cases."""
    return None

  def test_success(self):
    """Assembles some ImagePairs into an ImagePairSet, and validates results.
    """
    image_pairs = [
        MockImagePair(base_url=BASE_URL_1, dict_to_return=IMAGEPAIR_1_AS_DICT),
        MockImagePair(base_url=BASE_URL_1, dict_to_return=IMAGEPAIR_2_AS_DICT),
        MockImagePair(base_url=BASE_URL_1, dict_to_return=IMAGEPAIR_3_AS_DICT),
    ]
    expected_imageset_dict = {
        'extraColumnHeaders': {
            'builder': {
                'headerText': 'builder',
                'isFilterable': True,
                'isSortable': True,
                'valuesAndCounts': {
                    'MyBuilder': 3
                },
            },
            'test': {
                'headerText': 'which GM test',
                'headerUrl': 'http://learn/about/gm/tests',
                'isFilterable': True,
                'isSortable': False,
            },
        },
        'imagePairs': [
            IMAGEPAIR_1_AS_DICT,
            IMAGEPAIR_2_AS_DICT,
            IMAGEPAIR_3_AS_DICT,
        ],
        'imageSets': [
            {
                'baseUrl': BASE_URL_1,
                'description': SET_A_DESCRIPTION,
            },
            {
                'baseUrl': BASE_URL_1,
                'description': SET_B_DESCRIPTION,
            },
        ],
    }

    image_pair_set = imagepairset.ImagePairSet(
        descriptions=(SET_A_DESCRIPTION, SET_B_DESCRIPTION))
    for image_pair in image_pairs:
      image_pair_set.add_image_pair(image_pair)
    # The 'builder' column header uses the default settings,
    # but the 'test' column header has manual adjustments.
    image_pair_set.set_column_header_factory(
        'test',
        column.ColumnHeaderFactory(
            header_text='which GM test',
            header_url='http://learn/about/gm/tests',
            is_filterable=True,
            is_sortable=False,
            include_values_and_counts=False))
    self.assertEqual(image_pair_set.as_dict(), expected_imageset_dict)

  def test_mismatched_base_url(self):
    """Confirms that mismatched base_urls will cause an exception."""
    image_pair_set = imagepairset.ImagePairSet()
    image_pair_set.add_image_pair(
        MockImagePair(base_url=BASE_URL_1, dict_to_return=IMAGEPAIR_1_AS_DICT))
    image_pair_set.add_image_pair(
        MockImagePair(base_url=BASE_URL_1, dict_to_return=IMAGEPAIR_2_AS_DICT))
    with self.assertRaises(Exception):
      image_pair_set.add_image_pair(
          MockImagePair(base_url=BASE_URL_2,
                        dict_to_return=IMAGEPAIR_3_AS_DICT))


class MockImagePair(object):
  """Mock ImagePair object, which will return canned results."""
  def __init__(self, base_url, dict_to_return):
    """
    Args:
      base_url: base_url attribute for this object
      dict_to_return: dictionary to return from as_dict()
    """
    self.base_url = base_url
    self.extra_columns_dict = dict_to_return.get(
        imagepair.KEY__EXTRA_COLUMN_VALUES, None)
    self._dict_to_return = dict_to_return

  def as_dict(self):
    return self._dict_to_return


def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(ImagePairSetTest)
  unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == '__main__':
  main()
