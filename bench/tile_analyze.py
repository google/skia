#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

""" Analyze per-tile and viewport bench data, and output visualized results.
"""

__author__ = 'bensong@google.com (Ben Chen)'

import bench_util
import boto
import math
import optparse
import os
import re
import shutil

from oauth2_plugin import oauth2_plugin

# The default platform to analyze. Used when OPTION_PLATFORM flag is not set.
DEFAULT_PLATFORM = 'Nexus10_4-1_Float_Bench_32'

# Template for gsutil uri.
GOOGLE_STORAGE_URI_SCHEME = 'gs'
URI_BUCKET = 'chromium-skia-gm'

# Maximum number of rows of tiles to track for viewport covering.
MAX_TILE_ROWS = 8

# Constants for optparse.
USAGE_STRING = 'USAGE: %s [options]'
HOWTO_STRING = """
Note: to read bench data stored in Google Storage, you will need to set up the
corresponding Python library.
See http://developers.google.com/storage/docs/gspythonlibrary for details.
"""
HELP_STRING = """
For the given platform and revision number, find corresponding viewport and
tile benchmarks for each available picture bench, and output visualization and
analysis in HTML. By default it reads from Skia's Google Storage location where
bot data are stored, but if --dir is given, will read from local directory
instead.
""" + HOWTO_STRING

OPTION_DIR = '--dir'
OPTION_DIR_SHORT = '-d'
OPTION_REVISION = '--rev'
OPTION_REVISION_SHORT = '-r'
OPTION_PLATFORM = '--platform'
OPTION_PLATFORM_SHORT = '-p'
# Bench representation algorithm flag.
OPTION_REPRESENTATION_ALG = '--algorithm'
OPTION_REPRESENTATION_ALG_SHORT = '-a'

# Bench representation algorithm. See trunk/bench/bench_util.py.
REPRESENTATION_ALG = bench_util.ALGORITHM_25TH_PERCENTILE

# Constants for bench file matching.
GOOGLE_STORAGE_OBJECT_NAME_PREFIX = 'perfdata/Skia_'
BENCH_FILE_PREFIX_TEMPLATE = 'bench_r%s_'
TILING_FILE_NAME_INDICATOR = '_tile_'
VIEWPORT_FILE_NAME_INDICATOR = '_viewport_'

# Regular expression for matching format '<integer>x<integer>'.
DIMENSIONS_RE = '(\d+)x(\d+)'

# HTML and JS output templates.
HTML_PREFIX = """
<html><head><script type="text/javascript" src="https://www.google.com/jsapi">
</script><script type="text/javascript">google.load("visualization", "1.1",
{packages:["table"]});google.load("prototype", "1.6");</script>
<script type="text/javascript" src="https://systemsbiology-visualizations.googlecode.com/svn/trunk/src/main/js/load.js"></script><script
type="text/javascript"> systemsbiology.load("visualization", "1.0",
{packages:["bioheatmap"]});</script><script type="text/javascript">
google.setOnLoadCallback(drawVisualization); function drawVisualization() {
"""
HTML_SUFFIX = '</body></html>'
BAR_CHART_TEMPLATE = ('<img src="https://chart.googleapis.com/chart?chxr=0,0,'
    '300&chxt=x&chbh=15,0&chs=600x150&cht=bhg&chco=80C65A,224499,FF0000,0A8C8A,'
    'EBB671,DE091A,000000,00ffff&chds=a&chdl=%s&chd=t:%s" /><br>\n')
DRAW_OPTIONS = ('{passThroughBlack:false,useRowLabels:false,cellWidth:30,'
                'cellHeight:30}')
TABLE_OPTIONS = '{showRowNumber:true,firstRowNumber:" ",sort:"disable"}'

def GetFiles(rev, bench_dir, platform):
  """Reads in bench files of interest into a dictionary.

  If bench_dir is not empty, tries to read in local bench files; otherwise check
  Google Storage. Filters files by revision (rev) and platform, and ignores
  non-tile, non-viewport bench files.
  Outputs dictionary [filename] -> [file content].
  """
  file_dic = {}
  if not bench_dir:
    uri = boto.storage_uri(URI_BUCKET, GOOGLE_STORAGE_URI_SCHEME)
    # The boto API does not allow prefix/wildcard matching of Google Storage
    # objects. And Google Storage has a flat structure instead of being
    # organized in directories. Therefore, we have to scan all objects in the
    # Google Storage bucket to find the files we need, which is slow.
    # The option of implementing prefix matching as in gsutil seems to be
    # overkill, but gsutil does not provide an API ready for use. If speed is a
    # big concern, we suggest copying bot bench data from Google Storage using
    # gsutil and use --log_dir for fast local data reading.
    for obj in uri.get_bucket():
      # Filters out files of no interest.
      if (not obj.name.startswith(GOOGLE_STORAGE_OBJECT_NAME_PREFIX) or
          (obj.name.find(TILING_FILE_NAME_INDICATOR) < 0 and
           obj.name.find(VIEWPORT_FILE_NAME_INDICATOR) < 0) or
          obj.name.find(platform) < 0 or
          obj.name.find(BENCH_FILE_PREFIX_TEMPLATE % rev) < 0):
        continue
      file_dic[
          obj.name[obj.name.rfind('/') + 1 : ]] = obj.get_contents_as_string()
  else:
    for f in os.listdir(bench_dir):
      if (not os.path.isfile(os.path.join(bench_dir, f)) or
          (f.find(TILING_FILE_NAME_INDICATOR) < 0 and
           f.find(VIEWPORT_FILE_NAME_INDICATOR) < 0) or
          not f.startswith(BENCH_FILE_PREFIX_TEMPLATE % rev)):
        continue
      file_dic[f] = open(os.path.join(bench_dir, f)).read()

  if not file_dic:
    raise Exception('No bench file found in "%s" or Google Storage.' %
                    bench_dir)

  return file_dic

def GetTileMatrix(layout, tile_size, values, viewport):
  """For the given tile layout and per-tile bench values, returns a matrix of
  bench values with tiles outside the given viewport set to 0.

  layout, tile_size and viewport are given in string of format <w>x<h>, where
  <w> is viewport width or number of tile columns, and <h> is viewport height or
  number of tile rows. We truncate tile rows to MAX_TILE_ROWS to adjust for very
  long skp's.

  values: per-tile benches ordered row-by-row, starting from the top-left tile.

  Returns [sum, matrix] where sum is the total bench tile time that covers the
  viewport, and matrix is used for visualizing the tiles.
  """
  [tile_cols, tile_rows] = [int(i) for i in layout.split('x')]
  [tile_x, tile_y] = [int(i) for i in tile_size.split('x')]
  [viewport_x, viewport_y] = [int(i) for i in viewport.split('x')]
  viewport_cols = int(math.ceil(viewport_x * 1.0 / tile_x))
  viewport_rows = int(math.ceil(viewport_y * 1.0 / tile_y))
  truncated_tile_rows = min(tile_rows, MAX_TILE_ROWS)

  viewport_tile_sum = 0
  matrix = [[0 for y in range(tile_cols)] for x in range(truncated_tile_rows)]
  for y in range(min(viewport_cols, tile_cols)):
    for x in range(min(truncated_tile_rows, viewport_rows)):
      matrix[x][y] = values[x * tile_cols + y]
      viewport_tile_sum += values[x * tile_cols + y]

  return [viewport_tile_sum, matrix]

def GetTileVisCodes(suffix, matrix):
  """Generates and returns strings of [js_codes, row1, row2] which are codes for
  visualizing the benches from the given tile config and matrix data.
  row1 is used for the first row of heatmaps; row2 is for corresponding tables.
  suffix is only used to avoid name conflicts in the whole html output.
  """
  this_js = 'var data_%s=new google.visualization.DataTable();' % suffix
  for i in range(len(matrix[0])):
    this_js += 'data_%s.addColumn("number","%s");' % (suffix, i)
  this_js += 'data_%s.addRows(%s);' % (suffix, str(matrix))
  # Adds heatmap chart.
  this_js += ('var heat_%s=new org.systemsbiology.visualization' % suffix +
              '.BioHeatMap(document.getElementById("%s"));' % suffix +
              'heat_%s.draw(data_%s,%s);' % (suffix, suffix, DRAW_OPTIONS))
  # Adds data table chart.
  this_js += ('var table_%s=new google.visualization.Table(document.' % suffix +
              'getElementById("t%s"));table_%s.draw(data_%s,%s);\n' % (
                  suffix, suffix, suffix, TABLE_OPTIONS))
  table_row1 = '<td>%s<div id="%s"></div></td>' % (suffix, suffix)
  table_row2 = '<td><div id="t%s"></div></td>' % suffix

  return [this_js, table_row1, table_row2]

def OutputTileAnalysis(rev, representation_alg, bench_dir, platform):
  """Reads skp bench data and outputs tile vs. viewport analysis for the given
  platform.

  Ignores data with revisions other than rev. If bench_dir is not empty, read
  from the local directory instead of Google Storage.
  Uses the provided representation_alg for calculating bench representations.

  Returns (js_codes, body_codes): strings of js/html codes for stats and
  visualization.
  """
  js_codes = ''
  body_codes = ('}</script></head><body>'
                '<h3>PLATFORM: %s REVISION: %s</h3><br>' % (platform, rev))
  bench_dic = {}  # [bench][config] -> [layout, [values]]
  file_dic = GetFiles(rev, bench_dir, platform)
  for f in file_dic:
    for point in bench_util.parse('', file_dic[f].split('\n'),
                                  representation_alg):
      if point.time_type:  # Ignores non-walltime time_type.
        continue
      bench = point.bench.replace('.skp', '')
      config = point.config.replace('simple_', '')
      components = config.split('_')
      if components[0] == 'viewport':
        bench_dic.setdefault(bench, {})[config] = [components[1], [point.time]]
      else:  # Stores per-tile benches.
        bench_dic.setdefault(bench, {})[config] = [
          point.tile_layout, point.per_tile_values]
  benches = bench_dic.keys()
  benches.sort()
  for bench in benches:
    body_codes += '<h4>%s</h4><br><table><tr>' % bench
    heat_plots = ''  # For table row of heatmap plots.
    table_plots = ''  # For table row of data table plots.
    # For bar plot legends and values in URL string.
    legends = ''
    values = ''
    keys = bench_dic[bench].keys()
    keys.sort()
    if not keys[-1].startswith('viewport'):  # No viewport to analyze; skip.
      continue
    else:
      # Extracts viewport size, which for all viewport configs is the same.
      viewport = bench_dic[bench][keys[-1]][0]
    for config in keys:
      [layout, value_li] = bench_dic[bench][config]
      if config.startswith('tile_'):  # For per-tile data, visualize tiles.
        tile_size = config.split('_')[1]
        if (not re.search(DIMENSIONS_RE, layout) or
            not re.search(DIMENSIONS_RE, tile_size) or
            not re.search(DIMENSIONS_RE, viewport)):
          continue  # Skip unrecognized formats.
        [viewport_tile_sum, matrix] = GetTileMatrix(
            layout, tile_size, value_li, viewport)
        values += '%s|' % viewport_tile_sum
        [this_js, row1, row2] = GetTileVisCodes(config + '_' + bench, matrix)
        heat_plots += row1
        table_plots += row2
        js_codes += this_js
      else:  # For viewport data, there is only one element in value_li.
        values += '%s|' % sum(value_li)
      legends += '%s:%s|' % (config, sum(value_li))
    body_codes += (heat_plots + '</tr><tr>' + table_plots + '</tr></table>' +
                   '<br>' + BAR_CHART_TEMPLATE % (legends[:-1], values[:-1]))

  return (js_codes, body_codes)

def main():
  """Parses flags and outputs expected Skia picture bench results."""
  parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
  parser.add_option(OPTION_PLATFORM_SHORT, OPTION_PLATFORM,
      dest='plat', default=DEFAULT_PLATFORM,
      help='Platform to analyze. Set to DEFAULT_PLATFORM if not given.')
  parser.add_option(OPTION_REVISION_SHORT, OPTION_REVISION,
      dest='rev',
      help='(Mandatory) revision number to analyze.')
  parser.add_option(OPTION_DIR_SHORT, OPTION_DIR,
      dest='log_dir', default='',
      help=('(Optional) local directory where bench log files reside. If left '
            'empty (by default), will try to read from Google Storage.'))
  parser.add_option(OPTION_REPRESENTATION_ALG_SHORT, OPTION_REPRESENTATION_ALG,
      dest='alg', default=REPRESENTATION_ALG,
      help=('Bench representation algorithm. '
            'Default to "%s".' % REPRESENTATION_ALG))
  (options, args) = parser.parse_args()
  if not (options.rev and options.rev.isdigit()):
    parser.error('Please provide correct mandatory flag %s' % OPTION_REVISION)
    return
  rev = int(options.rev)
  (js_codes, body_codes) = OutputTileAnalysis(
      rev, options.alg, options.log_dir, options.plat)
  print HTML_PREFIX + js_codes + body_codes + HTML_SUFFIX


if '__main__' == __name__:
  main()
