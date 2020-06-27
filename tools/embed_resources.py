#!/usr/bin/python

'''
Copyright 2015 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import argparse


def bytes_from_file(f, chunksize=8192):
  while True:
    chunk = f.read(chunksize)
    if chunk:
      for b in chunk:
        if isinstance(b, str):
          # python 2
          yield ord(b)
        else:
          # python 3
          yield b
    else:
      break


def main():
  parser = argparse.ArgumentParser(
      formatter_class=argparse.RawDescriptionHelpFormatter,
      description='Convert resource files to embedded read only data.',
      epilog='''The output (when compiled and linked) can be used as:
struct SkEmbeddedResource {const uint8_t* data; const size_t size;};
struct SkEmbeddedHeader {const SkEmbeddedResource* entries; const int count;};
extern "C" SkEmbeddedHeader const NAME;''')
  parser.add_argument('--align', default=1, type=int,
                      help='minimum alignment (in bytes) of resource data')
  parser.add_argument('--name', default='_resource', type=str,
                      help='the name of the c identifier to export')
  parser.add_argument('--input', required=True, type=argparse.FileType('rb'),
                      nargs='+', help='list of resource files to embed')
  parser.add_argument('--output', required=True, type=argparse.FileType('w'),
                      help='the name of the cpp file to output')
  args = parser.parse_args()

  out = args.output.write;
  out('#include <stddef.h>\n')
  out('#include <stdint.h>\n')

  # Write the resources.
  index = 0
  for f in args.input:
    out('alignas({1:d}) static const uint8_t resource{0:d}[] = {{\n'
        .format(index, args.align))
    bytes_written = 0
    bytes_on_line = 0
    for b in bytes_from_file(f):
      out(hex(b) + ',')
      bytes_written += 1
      bytes_on_line += 1
      if bytes_on_line >= 32:
        out('\n')
        bytes_on_line = 0
    out('};\n')
    out('static const size_t resource{0:d}_size = {1:d};\n'
        .format(index, bytes_written))
    index += 1

  # Write the resource entries.
  out('struct SkEmbeddedResource { const uint8_t* d; const size_t s; };\n')
  out('static const SkEmbeddedResource header[] = {\n')
  index = 0
  for f in args.input:
    out('  {{ resource{0:d}, resource{0:d}_size }},\n'.format(index))
    index += 1
  out('};\n')
  out('static const int header_count = {0:d};\n'.format(index))

  # Export the resource header.
  out('struct SkEmbeddedHeader {const SkEmbeddedResource* e; const int c;};\n')
  out('extern "C" const SkEmbeddedHeader {0:s} = {{ header, header_count }};\n'
      .format(args.name))


if __name__ == "__main__":
  main()
