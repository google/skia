#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Test compare_rendered_pictures.py

TODO(epoger): Create a command to update the expected results (in
self._output_dir_expected) when appropriate.  For now, you should:
1. examine the results in self.output_dir_actual and make sure they are ok
2. rm -rf self._output_dir_expected
3. mv self.output_dir_actual self._output_dir_expected
Although, if you're using an SVN checkout, this will blow away .svn directories
within self._output_dir_expected, which wouldn't be good...

"""

# System-level imports
import os
import posixpath
import subprocess

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
import base_unittest
import compare_rendered_pictures
import find_run_binary
import gm_json
import imagediffdb
import imagepairset
import results


class CompareRenderedPicturesTest(base_unittest.TestCase):

  def test_endToEnd(self):
    """Generate two sets of SKPs, run render_pictures over both, and compare
    the results."""
    setA_subdir = 'before_patch'
    setB_subdir = 'after_patch'
    self._generate_skps_and_run_render_pictures(
        subdir=setA_subdir, skpdict={
            'changed.skp': 200,
            'unchanged.skp': 100,
            'only-in-before.skp': 128,
        })
    self._generate_skps_and_run_render_pictures(
        subdir=setB_subdir, skpdict={
            'changed.skp': 201,
            'unchanged.skp': 100,
            'only-in-after.skp': 128,
        })

    results_obj = compare_rendered_pictures.RenderedPicturesComparisons(
        setA_dir=os.path.join(self.temp_dir, setA_subdir),
        setB_dir=os.path.join(self.temp_dir, setB_subdir),
        setA_section=gm_json.JSONKEY_ACTUALRESULTS,
        setB_section=gm_json.JSONKEY_ACTUALRESULTS,
        image_diff_db=imagediffdb.ImageDiffDB(self.temp_dir),
        image_base_gs_url='gs://fakebucket/fake/path',
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp

    # Overwrite elements within the results that change from one test run
    # to the next.
    # pylint: disable=W0212
    results_obj._setA_descriptions[results.KEY__SET_DESCRIPTIONS__DIR] = [
        'before-patch-fake-dir']
    results_obj._setB_descriptions[results.KEY__SET_DESCRIPTIONS__DIR] = [
        'after-patch-fake-dir']

    gm_json.WriteToFile(
        results_obj.get_packaged_results_of_type(
            results.KEY__HEADER__RESULTS_ALL),
        os.path.join(self.output_dir_actual, 'compare_rendered_pictures.json'))

  def test_endToEnd_withImageBaseGSUrl(self):
    """Generate two sets of SKPs, run render_pictures over both, and compare
    the results."""
    setA_subdir = 'before_patch'
    setB_subdir = 'after_patch'
    imageA_gs_base = 'superman/kent-camera/pictures'
    imageB_gs_base = 'batman/batarang/pictures'
    self._generate_skps_and_run_render_pictures(
        subdir=setA_subdir, skpdict={
            'changed.skp': 200,
            'unchanged.skp': 100,
            'only-in-before.skp': 128,
        },
        image_base_gs_url='gs://%s' % imageA_gs_base)
    self._generate_skps_and_run_render_pictures(
        subdir=setB_subdir, skpdict={
            'changed.skp': 201,
            'unchanged.skp': 100,
            'only-in-after.skp': 128,
        },
        image_base_gs_url='gs://%s' % imageB_gs_base)

    results_obj = compare_rendered_pictures.RenderedPicturesComparisons(
        setA_dir=os.path.join(self.temp_dir, setA_subdir),
        setB_dir=os.path.join(self.temp_dir, setB_subdir),
        setA_section=gm_json.JSONKEY_ACTUALRESULTS,
        setB_section=gm_json.JSONKEY_ACTUALRESULTS,
        image_diff_db=imagediffdb.ImageDiffDB(self.temp_dir),
        image_base_gs_url='gs://fakebucket/fake/path',
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp

    output_dict = results_obj.get_packaged_results_of_type(
        results.KEY__HEADER__RESULTS_ALL)
    # Assert that the baseURLs are as expected.
    self.assertEquals(
        output_dict[imagepairset.KEY__ROOT__IMAGESETS]
                   [imagepairset.KEY__IMAGESETS__SET__IMAGE_A]
                   [imagepairset.KEY__IMAGESETS__FIELD__BASE_URL],
        'http://storage.cloud.google.com/%s' % imageA_gs_base)
    self.assertEquals(
        output_dict[imagepairset.KEY__ROOT__IMAGESETS]
                   [imagepairset.KEY__IMAGESETS__SET__IMAGE_B]
                   [imagepairset.KEY__IMAGESETS__FIELD__BASE_URL],
        'http://storage.cloud.google.com/%s' % imageB_gs_base)
    # Overwrite elements within the results that change from one test run
    # to the next.
    # pylint: disable=W0212
    results_obj._setA_descriptions[results.KEY__SET_DESCRIPTIONS__DIR] = [
        'before-patch-fake-dir']
    results_obj._setB_descriptions[results.KEY__SET_DESCRIPTIONS__DIR] = [
        'after-patch-fake-dir']

    gm_json.WriteToFile(
        output_dict,
        os.path.join(self.output_dir_actual,
                               'compare_rendered_pictures.json'))

  def test_repo_url(self):
    """Use repo: URL to specify summary files."""
    base_repo_url = 'repo:gm/rebaseline_server/testdata/inputs/skp-summaries'
    results_obj = compare_rendered_pictures.RenderedPicturesComparisons(
        setA_dir=posixpath.join(base_repo_url, 'expectations'),
        setB_dir=posixpath.join(base_repo_url, 'actuals'),
        setA_section=gm_json.JSONKEY_EXPECTEDRESULTS,
        setB_section=gm_json.JSONKEY_ACTUALRESULTS,
        image_diff_db=imagediffdb.ImageDiffDB(self.temp_dir),
        image_base_gs_url='gs://fakebucket/fake/path',
        diff_base_url='/static/generated-images')
    results_obj.get_timestamp = mock_get_timestamp

    # Overwrite elements within the results that change from one test run
    # to the next.
    # pylint: disable=W0212
    results_obj._setA_descriptions\
        [results.KEY__SET_DESCRIPTIONS__REPO_REVISION] = 'fake-repo-revision'
    results_obj._setB_descriptions\
        [results.KEY__SET_DESCRIPTIONS__REPO_REVISION] = 'fake-repo-revision'

    gm_json.WriteToFile(
        results_obj.get_packaged_results_of_type(
            results.KEY__HEADER__RESULTS_ALL),
        os.path.join(self.output_dir_actual, 'compare_rendered_pictures.json'))

  def _generate_skps_and_run_render_pictures(self, subdir, skpdict,
                                             image_base_gs_url=None):
    """Generate SKPs and run render_pictures on them.

    Args:
      subdir: subdirectory (within self.temp_dir) to write all files into
      skpdict: {skpname: redvalue} dictionary describing the SKP files to render
    """
    out_path = os.path.join(self.temp_dir, subdir)
    os.makedirs(out_path)
    for skpname, redvalue in skpdict.iteritems():
      self._run_skpmaker(
          output_path=os.path.join(out_path, skpname), red=redvalue)

    # TODO(epoger): Add --mode tile 256 256 --writeWholeImage to the unittest,
    # and fix its result!  (imageURLs within whole-image entries are wrong when
    # I tried adding that)
    binary = find_run_binary.find_path_to_program('render_pictures')
    render_pictures_cmd = [
        binary,
        '--config', '8888',
        '-r', out_path,
        '--writeChecksumBasedFilenames',
        '--writeJsonSummaryPath', os.path.join(out_path, 'summary.json'),
        '--writePath', out_path]
    if image_base_gs_url:
      render_pictures_cmd.extend(['--imageBaseGSUrl', image_base_gs_url])
    return subprocess.check_output(render_pictures_cmd)

  def _run_skpmaker(self, output_path, red=0, green=0, blue=0,
                    width=640, height=400):
    """Runs the skpmaker binary to generate SKP with known characteristics.

    Args:
      output_path: Filepath to write the SKP into.
      red: Value of red color channel in image, 0-255.
      green: Value of green color channel in image, 0-255.
      blue: Value of blue color channel in image, 0-255.
      width: Width of canvas to create.
      height: Height of canvas to create.
    """
    binary = find_run_binary.find_path_to_program('skpmaker')
    return subprocess.check_output([
        binary,
        '--red', str(red),
        '--green', str(green),
        '--blue', str(blue),
        '--width', str(width),
        '--height', str(height),
        '--writePath', str(output_path)])

def mock_get_timestamp():
  """Mock version of BaseComparisons.get_timestamp() for testing."""
  return 12345678


def main():
  base_unittest.main(CompareRenderedPicturesTest)


if __name__ == '__main__':
  main()
