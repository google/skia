#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generate a spatial analysis against an arbitrary library.

Adapted for Skia's use case from
chromium/src/tools/binary_size/run_binary_size_analysis.py. Main changes:

-- Cleans up some deprecated codes.
-- Always use relative code path so the tree root is Skia repo's root.
-- Instead of outputting the standalone HTML/CSS/JS filesets, writes the
    TreeMap JSON data into a Google Storage bucket.
-- Adds githash and total_size to the JSON data.
-- Outputs another summary data in JSON Bench format for skiaperf ingestion.

The output JSON data for visualization is in the following format:

{
  "githash": 123abc,
  "commit_ts": 1234567890,
  "total_size": 1234567,
  "key": {"source_type": "binary_size"},
  "tree_data": {
    "maxDepth": 9,
    "k": "p", "children":[
      {"k":"p","children":[
        {"k":"p","children":[
          {"k":"p","lastPathElement":true,"children":[
            {"k":"b","t":"t","children":[
              {"k":"s", "t":"t", "value":4029,
               "n":"etc_encode_subblock_helper(unsigned char const*, ...)"
              },
          ......
  }
}

Another JSON file is generated for size summaries to be used in skiaperf. The
JSON format details can be found at:
  https://github.com/google/skia/blob/master/bench/ResultsWriter.h#L54
and:
  https://skia.googlesource.com/buildbot/+/master/perf/go/ingester/nanobench.go

In the binary size case, outputs look like:

{
  "gitHash": "123abc",
  "key": {
    "source_type": "binarysize"
  }
  "results: {
    "src_lazy_global_weak_symbol": {
      "memory": {
        "bytes": 41,
        "options": {
          "path": "src_lazy",
          "symbol": "global_weak_symbol"
        }
      }
    },
    "src_lazy_global_read_only_data": {
      "memory": {
        "bytes": 13476,
        "options": {
          "path": "src_lazy",
          "symbol": "global_read_only_data"
        }
      }
    },
    ...
  }
}

"""

import collections
import datetime
import json
import logging
import multiprocessing
import optparse
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import urllib2

import binary_size_utils
import elf_symbolizer

from recipe_engine.types import freeze

# Node dictionary keys. These are output in json read by the webapp so
# keep them short to save file size.
# Note: If these change, the webapp must also change.
NODE_TYPE_KEY = 'k'
NODE_NAME_KEY = 'n'
NODE_CHILDREN_KEY = 'children'
NODE_SYMBOL_TYPE_KEY = 't'
NODE_SYMBOL_SIZE_KEY = 'value'
NODE_MAX_DEPTH_KEY = 'maxDepth'
NODE_LAST_PATH_ELEMENT_KEY = 'lastPathElement'

# The display name of the bucket where we put symbols without path.
NAME_NO_PATH_BUCKET = '(No Path)'

# Try to keep data buckets smaller than this to avoid killing the
# graphing lib.
BIG_BUCKET_LIMIT = 3000

# Skia addition: relative dir for libskia.so from code base.
LIBSKIA_RELATIVE_PATH = os.path.join('out', 'Release', 'lib')

# Skia addition: dictionary mapping symbol type code to symbol name.
# See
# https://code.google.com/p/chromium/codesearch#chromium/src/tools/binary_size/template/D3SymbolTreeMap.js&l=74
SYMBOL_MAP = freeze({
    'A': 'global_absolute',
    'B': 'global_uninitialized_data',
    'b': 'local_uninitialized_data',
    'C': 'global_uninitialized_common',
    'D': 'global_initialized_data',
    'd': 'local_initialized_data',
    'G': 'global_small initialized_data',
    'g': 'local_small_initialized_data',
    'i': 'indirect_function',
    'N': 'debugging',
    'p': 'stack_unwind',
    'R': 'global_read_only_data',
    'r': 'local_read_only_data',
    'S': 'global_small_uninitialized_data',
    's': 'local_small_uninitialized_data',
    'T': 'global_code',
    't': 'local_code',
    'U': 'undefined',
    'u': 'unique',
    'V': 'global_weak_object',
    'v': 'local_weak_object',
    'W': 'global_weak_symbol',
    'w': 'local_weak_symbol',
    '@': 'vtable_entry',
    '-': 'stabs_debugging',
    '?': 'unrecognized',
})


def _MkChild(node, name):
  child = node[NODE_CHILDREN_KEY].get(name)
  if child is None:
    child = {NODE_NAME_KEY: name,
             NODE_CHILDREN_KEY: {}}
    node[NODE_CHILDREN_KEY][name] = child
  return child


def SplitNoPathBucket(node):
  """NAME_NO_PATH_BUCKET can be too large for the graphing lib to
  handle. Split it into sub-buckets in that case."""
  root_children = node[NODE_CHILDREN_KEY]
  if NAME_NO_PATH_BUCKET in root_children:
    no_path_bucket = root_children[NAME_NO_PATH_BUCKET]
    old_children = no_path_bucket[NODE_CHILDREN_KEY]
    count = 0
    for symbol_type, symbol_bucket in old_children.iteritems():
      count += len(symbol_bucket[NODE_CHILDREN_KEY])
    if count > BIG_BUCKET_LIMIT:
      new_children = {}
      no_path_bucket[NODE_CHILDREN_KEY] = new_children
      current_bucket = None
      index = 0
      for symbol_type, symbol_bucket in old_children.iteritems():
        for symbol_name, value in symbol_bucket[NODE_CHILDREN_KEY].iteritems():
          if index % BIG_BUCKET_LIMIT == 0:
            group_no = (index / BIG_BUCKET_LIMIT) + 1
            current_bucket = _MkChild(no_path_bucket,
                                      '%s subgroup %d' % (NAME_NO_PATH_BUCKET,
                                                          group_no))
            assert not NODE_TYPE_KEY in node or node[NODE_TYPE_KEY] == 'p'
            node[NODE_TYPE_KEY] = 'p'  # p for path
          index += 1
          symbol_size = value[NODE_SYMBOL_SIZE_KEY]
          AddSymbolIntoFileNode(current_bucket, symbol_type,
                                symbol_name, symbol_size)


def MakeChildrenDictsIntoLists(node):
  largest_list_len = 0
  if NODE_CHILDREN_KEY in node:
    largest_list_len = len(node[NODE_CHILDREN_KEY])
    child_list = []
    for child in node[NODE_CHILDREN_KEY].itervalues():
      child_largest_list_len = MakeChildrenDictsIntoLists(child)
      if child_largest_list_len > largest_list_len:
        largest_list_len = child_largest_list_len
      child_list.append(child)
    node[NODE_CHILDREN_KEY] = child_list

  return largest_list_len


def AddSymbolIntoFileNode(node, symbol_type, symbol_name, symbol_size):
  """Puts symbol into the file path node |node|.
  Returns the number of added levels in tree. I.e. returns 2."""

  # 'node' is the file node and first step is to find its symbol-type bucket.
  node[NODE_LAST_PATH_ELEMENT_KEY] = True
  node = _MkChild(node, symbol_type)
  assert not NODE_TYPE_KEY in node or node[NODE_TYPE_KEY] == 'b'
  node[NODE_SYMBOL_TYPE_KEY] = symbol_type
  node[NODE_TYPE_KEY] = 'b'  # b for bucket

  # 'node' is now the symbol-type bucket. Make the child entry.
  node = _MkChild(node, symbol_name)
  if NODE_CHILDREN_KEY in node:
    if node[NODE_CHILDREN_KEY]:
      logging.warning('A container node used as symbol for %s.' % symbol_name)
    # This is going to be used as a leaf so no use for child list.
    del node[NODE_CHILDREN_KEY]
  node[NODE_SYMBOL_SIZE_KEY] = symbol_size
  node[NODE_SYMBOL_TYPE_KEY] = symbol_type
  node[NODE_TYPE_KEY] = 's'  # s for symbol

  return 2  # Depth of the added subtree.


def MakeCompactTree(symbols, symbol_path_origin_dir):
  result = {NODE_NAME_KEY: '/',
            NODE_CHILDREN_KEY: {},
            NODE_TYPE_KEY: 'p',
            NODE_MAX_DEPTH_KEY: 0}
  seen_symbol_with_path = False
  for symbol_name, symbol_type, symbol_size, file_path in symbols:

    if 'vtable for ' in symbol_name:
      symbol_type = '@'  # hack to categorize these separately
    if file_path and file_path != "??":
      seen_symbol_with_path = True
    else:
      file_path = NAME_NO_PATH_BUCKET

    path_parts = file_path.split('/')

    # Find pre-existing node in tree, or update if it already exists
    node = result
    depth = 0
    while len(path_parts) > 0:
      path_part = path_parts.pop(0)
      if len(path_part) == 0:
        continue
      depth += 1
      node = _MkChild(node, path_part)
      assert not NODE_TYPE_KEY in node or node[NODE_TYPE_KEY] == 'p'
      node[NODE_TYPE_KEY] = 'p'  # p for path

    depth += AddSymbolIntoFileNode(node, symbol_type, symbol_name, symbol_size)
    result[NODE_MAX_DEPTH_KEY] = max(result[NODE_MAX_DEPTH_KEY], depth)

  if not seen_symbol_with_path:
    logging.warning('Symbols lack paths. Data will not be structured.')

  # The (no path) bucket can be extremely large if we failed to get
  # path information. Split it into subgroups if needed.
  SplitNoPathBucket(result)

  largest_list_len = MakeChildrenDictsIntoLists(result)

  if largest_list_len > BIG_BUCKET_LIMIT:
    logging.warning('There are sections with %d nodes. '
                    'Results might be unusable.' % largest_list_len)
  return result


# Skia added: summarizes tree size by symbol type for the given root node.
# Returns a dict keyed by symbol type, and value the type's overall size.
# e.g., {"t": 12345, "W": 543}.
def GetTreeSizes(node):
  if 'children' not in node or not node['children']:
    return {node['t']: node['value']}
  dic = {}
  for i in node['children']:
    for k, v in GetTreeSizes(i).items():
      dic.setdefault(k, 0)
      dic[k] += v

  return dic


# Skia added: creates dict to be converted to JSON in bench format.
# See top of file for the structure description.
def GetBenchDict(githash, tree_root):
  dic = {'gitHash': githash,
         'key': {'source_type': 'binarysize'},
         'results': {},}
  for i in tree_root['children']:
    if '(No Path)' == i['n']:  # Already at symbol summary level.
      for k, v in GetTreeSizes(i).items():
        dic['results']['no_path_' + SYMBOL_MAP[k]] = {
            'memory': {
              'bytes': v,
              'options': {'path': 'no_path',
                          'symbol': SYMBOL_MAP[k],},}}
    else:  # We need to go deeper.
      for c in i['children']:
        path = i['n'] + '_' + c['n']
        for k, v in GetTreeSizes(c).items():
          dic['results'][path + '_' + SYMBOL_MAP[k]] = {
              'memory': {
                'bytes': v,
                'options': {'path': path,
                            'symbol': SYMBOL_MAP[k],}}}

  return dic


# Skia added: constructs 'gsutil cp' subprocess command list.
def GetGsCopyCommandList(gsutil, src, dst):
  return [gsutil, '-h', 'Content-Type:application/json', 'cp', '-a',
          'public-read', src, dst]


def DumpCompactTree(symbols, symbol_path_origin_dir, ha, ts, issue, gsutil):
  tree_root = MakeCompactTree(symbols, symbol_path_origin_dir)
  json_data = {'tree_data': tree_root,
               'githash': ha,
               'commit_ts': ts,
               'key': {'source_type': 'binary_size'},
               'total_size': sum(GetTreeSizes(tree_root).values()),}
  tmpfile = tempfile.NamedTemporaryFile(delete=False).name
  with open(tmpfile, 'w') as out:
    # Use separators without whitespace to get a smaller file.
    json.dump(json_data, out, separators=(',', ':'))

  GS_PREFIX = 'gs://skia-perf/'
  # Writes to Google Storage for visualization.
  subprocess.check_call(GetGsCopyCommandList(
      gsutil, tmpfile, GS_PREFIX + 'size/' + ha + '.json'))
  # Updates the latest data.
  if not issue:
    subprocess.check_call(GetGsCopyCommandList(gsutil, tmpfile,
                                               GS_PREFIX + 'size/latest.json'))
  # Writes an extra copy using year/month/day/hour path for easy ingestion.
  with open(tmpfile, 'w') as out:
    json.dump(GetBenchDict(ha, tree_root), out, separators=(',', ':'))
  now = datetime.datetime.utcnow()
  ingest_path = '/'.join(('nano-json-v1', str(now.year).zfill(4),
                          str(now.month).zfill(2), str(now.day).zfill(2),
                          str(now.hour).zfill(2)))
  if issue:
    ingest_path = '/'.join('trybot', ingest_path, issue)
  subprocess.check_call(GetGsCopyCommandList(gsutil, tmpfile,
      GS_PREFIX + ingest_path + '/binarysize_' + ha + '.json'))


def MakeSourceMap(symbols):
  sources = {}
  for _sym, _symbol_type, size, path in symbols:
    key = None
    if path:
      key = os.path.normpath(path)
    else:
      key = '[no path]'
    if key not in sources:
      sources[key] = {'path': path, 'symbol_count': 0, 'size': 0}
    record = sources[key]
    record['size'] += size
    record['symbol_count'] += 1
  return sources


# Regex for parsing "nm" output. A sample line looks like this:
# 0167b39c 00000018 t ACCESS_DESCRIPTION_free /path/file.c:95
#
# The fields are: address, size, type, name, source location
# Regular expression explained ( see also: https://xkcd.com/208 ):
# ([0-9a-f]{8,}+)   The address
# [\s]+             Whitespace separator
# ([0-9a-f]{8,}+)   The size. From here on out it's all optional.
# [\s]+             Whitespace separator
# (\S?)             The symbol type, which is any non-whitespace char
# [\s*]             Whitespace separator
# ([^\t]*)          Symbol name, any non-tab character (spaces ok!)
# [\t]?             Tab separator
# (.*)              The location (filename[:linennum|?][ (discriminator n)]
sNmPattern = re.compile(
  r'([0-9a-f]{8,})[\s]+([0-9a-f]{8,})[\s]*(\S?)[\s*]([^\t]*)[\t]?(.*)')

class Progress():
  def __init__(self):
    self.count = 0
    self.skip_count = 0
    self.collisions = 0
    self.time_last_output = time.time()
    self.count_last_output = 0
    self.disambiguations = 0
    self.was_ambiguous = 0


def RunElfSymbolizer(outfile, library, addr2line_binary, nm_binary, jobs,
                     disambiguate, src_path):
  nm_output = RunNm(library, nm_binary)
  nm_output_lines = nm_output.splitlines()
  nm_output_lines_len = len(nm_output_lines)
  address_symbol = {}
  progress = Progress()
  def map_address_symbol(symbol, addr):
    progress.count += 1
    if addr in address_symbol:
      # 'Collision between %s and %s.' % (str(symbol.name),
      #                                   str(address_symbol[addr].name))
      progress.collisions += 1
    else:
      if symbol.disambiguated:
        progress.disambiguations += 1
      if symbol.was_ambiguous:
        progress.was_ambiguous += 1

      address_symbol[addr] = symbol

    progress_output()

  def progress_output():
    progress_chunk = 100
    if progress.count % progress_chunk == 0:
      time_now = time.time()
      time_spent = time_now - progress.time_last_output
      if time_spent > 1.0:
        # Only output at most once per second.
        progress.time_last_output = time_now
        chunk_size = progress.count - progress.count_last_output
        progress.count_last_output = progress.count
        if time_spent > 0:
          speed = chunk_size / time_spent
        else:
          speed = 0
        progress_percent = (100.0 * (progress.count + progress.skip_count) /
                            nm_output_lines_len)
        disambiguation_percent = 0
        if progress.disambiguations != 0:
          disambiguation_percent = (100.0 * progress.disambiguations /
                                    progress.was_ambiguous)

        sys.stdout.write('\r%.1f%%: Looked up %d symbols (%d collisions, '
              '%d disambiguations where %.1f%% succeeded)'
              ' - %.1f lookups/s.' %
              (progress_percent, progress.count, progress.collisions,
               progress.disambiguations, disambiguation_percent, speed))

  # In case disambiguation was disabled, we remove the source path (which upon
  # being set signals the symbolizer to enable disambiguation)
  if not disambiguate:
    src_path = None
  symbol_path_origin_dir = os.path.dirname(library)
  # Skia specific.
  symbol_path_prefix = symbol_path_origin_dir.replace(LIBSKIA_RELATIVE_PATH, '')
  symbolizer = elf_symbolizer.ELFSymbolizer(library, addr2line_binary,
                                            map_address_symbol,
                                            max_concurrent_jobs=jobs,
                                            source_root_path=src_path,
                                            prefix_to_remove=symbol_path_prefix)
  user_interrupted = False
  try:
    for line in nm_output_lines:
      match = sNmPattern.match(line)
      if match:
        location = match.group(5)
        if not location:
          addr = int(match.group(1), 16)
          size = int(match.group(2), 16)
          if addr in address_symbol:  # Already looked up, shortcut
                                      # ELFSymbolizer.
            map_address_symbol(address_symbol[addr], addr)
            continue
          elif size == 0:
            # Save time by not looking up empty symbols (do they even exist?)
            print('Empty symbol: ' + line)
          else:
            symbolizer.SymbolizeAsync(addr, addr)
            continue

      progress.skip_count += 1
  except KeyboardInterrupt:
    user_interrupted = True
    print('Interrupting - killing subprocesses. Please wait.')

  try:
    symbolizer.Join()
  except KeyboardInterrupt:
    # Don't want to abort here since we will be finished in a few seconds.
    user_interrupted = True
    print('Patience you must have my young padawan.')

  print ''

  if user_interrupted:
    print('Skipping the rest of the file mapping. '
          'Output will not be fully classified.')

  symbol_path_origin_dir = os.path.dirname(library)
  # Skia specific: path prefix to strip.
  symbol_path_prefix = symbol_path_origin_dir.replace(LIBSKIA_RELATIVE_PATH, '')

  with open(outfile, 'w') as out:
    for line in nm_output_lines:
      match = sNmPattern.match(line)
      if match:
        location = match.group(5)
        if not location:
          addr = int(match.group(1), 16)
          symbol = address_symbol.get(addr)
          if symbol is not None:
            path = '??'
            if symbol.source_path is not None:
              path = symbol.source_path.replace(symbol_path_prefix, '')
            line_number = 0
            if symbol.source_line is not None:
              line_number = symbol.source_line
            out.write('%s\t%s:%d\n' % (line, path, line_number))
            continue

      out.write('%s\n' % line)

  print('%d symbols in the results.' % len(address_symbol))


def RunNm(binary, nm_binary):
  cmd = [nm_binary, '-C', '--print-size', '--size-sort', '--reverse-sort',
         binary]
  nm_process = subprocess.Popen(cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
  (process_output, err_output) = nm_process.communicate()

  if nm_process.returncode != 0:
    if err_output:
      raise Exception, err_output
    else:
      raise Exception, process_output

  return process_output


def GetNmSymbols(nm_infile, outfile, library, jobs, verbose,
                 addr2line_binary, nm_binary, disambiguate, src_path):
  if nm_infile is None:
    if outfile is None:
      outfile = tempfile.NamedTemporaryFile(delete=False).name

    if verbose:
      print 'Running parallel addr2line, dumping symbols to ' + outfile
    RunElfSymbolizer(outfile, library, addr2line_binary, nm_binary, jobs,
                     disambiguate, src_path)

    nm_infile = outfile

  elif verbose:
    print 'Using nm input from ' + nm_infile
  with file(nm_infile, 'r') as infile:
    return list(binary_size_utils.ParseNm(infile))


PAK_RESOURCE_ID_TO_STRING = { "inited": False }

def LoadPakIdsFromResourceFile(filename):
  """Given a file name, it loads everything that looks like a resource id
  into PAK_RESOURCE_ID_TO_STRING."""
  with open(filename) as resource_header:
    for line in resource_header:
      if line.startswith("#define "):
        line_data = line.split()
        if len(line_data) == 3:
          try:
            resource_number = int(line_data[2])
            resource_name = line_data[1]
            PAK_RESOURCE_ID_TO_STRING[resource_number] = resource_name
          except ValueError:
            pass

def GetReadablePakResourceName(pak_file, resource_id):
  """Pak resources have a numeric identifier. It is not helpful when
  trying to locate where footprint is generated. This does its best to
  map the number to a usable string."""
  if not PAK_RESOURCE_ID_TO_STRING['inited']:
    # Try to find resource header files generated by grit when
    # building the pak file. We'll look for files named *resources.h"
    # and lines of the type:
    #    #define MY_RESOURCE_JS 1234
    PAK_RESOURCE_ID_TO_STRING['inited'] = True
    gen_dir = os.path.join(os.path.dirname(pak_file), 'gen')
    if os.path.isdir(gen_dir):
      for dirname, _dirs, files in os.walk(gen_dir):
        for filename in files:
          if filename.endswith('resources.h'):
            LoadPakIdsFromResourceFile(os.path.join(dirname, filename))
  return PAK_RESOURCE_ID_TO_STRING.get(resource_id,
                                       'Pak Resource %d' % resource_id)

def AddPakData(symbols, pak_file):
  """Adds pseudo-symbols from a pak file."""
  pak_file = os.path.abspath(pak_file)
  with open(pak_file, 'rb') as pak:
    data = pak.read()

  PAK_FILE_VERSION = 4
  HEADER_LENGTH = 2 * 4 + 1  # Two uint32s. (file version, number of entries)
                             # and one uint8 (encoding of text resources)
  INDEX_ENTRY_SIZE = 2 + 4  # Each entry is a uint16 and a uint32.
  version, num_entries, _encoding = struct.unpack('<IIB', data[:HEADER_LENGTH])
  assert version == PAK_FILE_VERSION, ('Unsupported pak file '
                                       'version (%d) in %s. Only '
                                       'support version %d' %
                                       (version, pak_file, PAK_FILE_VERSION))
  if num_entries > 0:
    # Read the index and data.
    data = data[HEADER_LENGTH:]
    for _ in range(num_entries):
      resource_id, offset = struct.unpack('<HI', data[:INDEX_ENTRY_SIZE])
      data = data[INDEX_ENTRY_SIZE:]
      _next_id, next_offset = struct.unpack('<HI', data[:INDEX_ENTRY_SIZE])
      resource_size = next_offset - offset

      symbol_name = GetReadablePakResourceName(pak_file, resource_id)
      symbol_path = pak_file
      symbol_type = 'd' # Data. Approximation.
      symbol_size = resource_size
      symbols.append((symbol_name, symbol_type, symbol_size, symbol_path))

def _find_in_system_path(binary):
  """Locate the full path to binary in the system path or return None
  if not found."""
  system_path = os.environ["PATH"].split(os.pathsep)
  for path in system_path:
    binary_path = os.path.join(path, binary)
    if os.path.isfile(binary_path):
      return binary_path
  return None

def CheckDebugFormatSupport(library, addr2line_binary):
  """Kills the program if debug data is in an unsupported format.

  There are two common versions of the DWARF debug formats and
  since we are right now transitioning from DWARF2 to newer formats,
  it's possible to have a mix of tools that are not compatible. Detect
  that and abort rather than produce meaningless output."""
  tool_output = subprocess.check_output([addr2line_binary, '--version'])
  version_re = re.compile(r'^GNU [^ ]+ .* (\d+).(\d+).*?$', re.M)
  parsed_output = version_re.match(tool_output)
  major = int(parsed_output.group(1))
  minor = int(parsed_output.group(2))
  supports_dwarf4 = major > 2 or major == 2 and minor > 22

  if supports_dwarf4:
    return

  print('Checking version of debug information in %s.' % library)
  debug_info = subprocess.check_output(['readelf', '--debug-dump=info',
                                       '--dwarf-depth=1', library])
  dwarf_version_re = re.compile(r'^\s+Version:\s+(\d+)$', re.M)
  parsed_dwarf_format_output = dwarf_version_re.search(debug_info)
  version = int(parsed_dwarf_format_output.group(1))
  if version > 2:
    print('The supplied tools only support DWARF2 debug data but the binary\n' +
          'uses DWARF%d. Update the tools or compile the binary\n' % version +
          'with -gdwarf-2.')
    sys.exit(1)


def main():
  usage = """%prog [options]

  Runs a spatial analysis on a given library, looking up the source locations
  of its symbols and calculating how much space each directory, source file,
  and so on is taking. The result is a report that can be used to pinpoint
  sources of large portions of the binary, etceteras.

  Under normal circumstances, you only need to pass two arguments, thusly:

      %prog --library /path/to/library --destdir /path/to/output

  In this mode, the program will dump the symbols from the specified library
  and map those symbols back to source locations, producing a web-based
  report in the specified output directory.

  Other options are available via '--help'.
  """
  parser = optparse.OptionParser(usage=usage)
  parser.add_option('--nm-in', metavar='PATH',
                    help='if specified, use nm input from <path> instead of '
                    'generating it. Note that source locations should be '
                    'present in the file; i.e., no addr2line symbol lookups '
                    'will be performed when this option is specified. '
                    'Mutually exclusive with --library.')
  parser.add_option('--destdir', metavar='PATH',
                    help='write output to the specified directory. An HTML '
                    'report is generated here along with supporting files; '
                    'any existing report will be overwritten. Not used in '
                    'Skia.')
  parser.add_option('--library', metavar='PATH',
                    help='if specified, process symbols in the library at '
                    'the specified path. Mutually exclusive with --nm-in.')
  parser.add_option('--pak', metavar='PATH',
                    help='if specified, includes the contents of the '
                    'specified *.pak file in the output.')
  parser.add_option('--nm-binary',
                    help='use the specified nm binary to analyze library. '
                    'This is to be used when the nm in the path is not for '
                    'the right architecture or of the right version.')
  parser.add_option('--addr2line-binary',
                    help='use the specified addr2line binary to analyze '
                    'library. This is to be used when the addr2line in '
                    'the path is not for the right architecture or '
                    'of the right version.')
  parser.add_option('--jobs', type='int',
                    help='number of jobs to use for the parallel '
                    'addr2line processing pool; defaults to 1. More '
                    'jobs greatly improve throughput but eat RAM like '
                    'popcorn, and take several gigabytes each. Start low '
                    'and ramp this number up until your machine begins to '
                    'struggle with RAM. '
                    'This argument is only valid when using --library.')
  parser.add_option('-v', dest='verbose', action='store_true',
                    help='be verbose, printing lots of status information.')
  parser.add_option('--nm-out', metavar='PATH',
                    help='keep the nm output file, and store it at the '
                    'specified path. This is useful if you want to see the '
                    'fully processed nm output after the symbols have been '
                    'mapped to source locations. By default, a tempfile is '
                    'used and is deleted when the program terminates.'
                    'This argument is only valid when using --library.')
  parser.add_option('--legacy', action='store_true',
                    help='emit legacy binary size report instead of modern')
  parser.add_option('--disable-disambiguation', action='store_true',
                    help='disables the disambiguation process altogether,'
                    ' NOTE: this may, depending on your toolchain, produce'
                    ' output with some symbols at the top layer if addr2line'
                    ' could not get the entire source path.')
  parser.add_option('--source-path', default='./',
                    help='the path to the source code of the output binary, '
                    'default set to current directory. Used in the'
                    ' disambiguation process.')
  parser.add_option('--githash', default='latest',
                    help='Git hash for the binary version. Added by Skia.')
  parser.add_option('--commit_ts', type='int', default=-1,
                    help='Timestamp for the commit. Added by Skia.')
  parser.add_option('--issue_number', default='',
                    help='The trybot issue number in string. Added by Skia.')
  parser.add_option('--gsutil_path', default='gsutil',
                    help='Path to gsutil binary. Added by Skia.')
  opts, _args = parser.parse_args()

  if ((not opts.library) and (not opts.nm_in)) or (opts.library and opts.nm_in):
    parser.error('exactly one of --library or --nm-in is required')
  if (opts.nm_in):
    if opts.jobs:
      print >> sys.stderr, ('WARNING: --jobs has no effect '
                            'when used with --nm-in')
  if not opts.jobs:
    # Use the number of processors but cap between 2 and 4 since raw
    # CPU power isn't the limiting factor. It's I/O limited, memory
    # bus limited and available-memory-limited. Too many processes and
    # the computer will run out of memory and it will be slow.
    opts.jobs = max(2, min(4, str(multiprocessing.cpu_count())))

  if opts.addr2line_binary:
    assert os.path.isfile(opts.addr2line_binary)
    addr2line_binary = opts.addr2line_binary
  else:
    addr2line_binary = _find_in_system_path('addr2line')
    assert addr2line_binary, 'Unable to find addr2line in the path. '\
        'Use --addr2line-binary to specify location.'

  if opts.nm_binary:
    assert os.path.isfile(opts.nm_binary)
    nm_binary = opts.nm_binary
  else:
    nm_binary = _find_in_system_path('nm')
    assert nm_binary, 'Unable to find nm in the path. Use --nm-binary '\
        'to specify location.'

  if opts.pak:
    assert os.path.isfile(opts.pak), 'Could not find ' % opts.pak

  print('addr2line: %s' % addr2line_binary)
  print('nm: %s' % nm_binary)

  if opts.library:
    CheckDebugFormatSupport(opts.library, addr2line_binary)

  symbols = GetNmSymbols(opts.nm_in, opts.nm_out, opts.library,
                         opts.jobs, opts.verbose is True,
                         addr2line_binary, nm_binary,
                         opts.disable_disambiguation is None,
                         opts.source_path)

  if opts.pak:
    AddPakData(symbols, opts.pak)

  if opts.legacy: # legacy report
    print 'Do Not set legacy flag.'

  else: # modern report
    if opts.library:
      symbol_path_origin_dir = os.path.dirname(os.path.abspath(opts.library))
    else:
      # Just a guess. Hopefully all paths in the input file are absolute.
      symbol_path_origin_dir = os.path.abspath(os.getcwd())
    DumpCompactTree(symbols, symbol_path_origin_dir, opts.githash,
                    opts.commit_ts, opts.issue_number, opts.gsutil_path)
    print 'Report data uploaded to GS.'


if __name__ == '__main__':
  sys.exit(main())
