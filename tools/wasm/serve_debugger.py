#!/usr/bin/env python3
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Serves debugger.html alongside a wasm dm build, for debugging dm in Chrome."""

import argparse
import http.server
import os

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
SKIA_ROOT_DIR = os.path.abspath(os.path.join(THIS_DIR, os.pardir, os.pardir))
DEBUGGER_HTML = os.path.join(THIS_DIR, 'debugger.html')


class Handler(http.server.SimpleHTTPRequestHandler):
  """http handler for debug page"""

  # Skia source dir, set in main().
  src = None

  def translate_path(self, path):
    if path.split('?')[0] in ('/', '/debugger.html'):
      return DEBUGGER_HTML
    # Serve from the build dir if the file is there, else from the source dir.
    # The source map's paths are relative to the build dir (e.g.
    # ../../dm/DM.cpp), so the browser requests them at the root (/dm/DM.cpp).
    built = super().translate_path(path)
    if os.path.exists(built):
      return built
    return os.path.join(self.src, os.path.relpath(built, self.directory))

  def end_headers(self):
    # wasm pthreads need SharedArrayBuffer, which needs these headers.
    self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
    self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
    super().end_headers()


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--src', default=SKIA_ROOT_DIR,
                      help='Skia source dir, for source mappings')
  parser.add_argument('--build', required=True,
                      help='wasm build dir containing dm.js')
  parser.add_argument('--port', type=int, default=8000, help='default: 8000')
  args = parser.parse_args()

  Handler.src = os.path.abspath(args.src)
  build = os.path.abspath(args.build)
  for f in ['dm.js', 'dm.wasm']:
    if not os.path.isfile(os.path.join(build, f)):
      parser.error(f'no {f} found in {build}')

  os.chdir(build)
  server = http.server.ThreadingHTTPServer(('localhost', args.port), Handler)
  try:
    print('Serving:')
    print(f'- artifacts from {build}')
    print(f'- sources map to {Handler.src}')
    print(f'Serving at http://localhost:{args.port}/')
    server.serve_forever()
  except KeyboardInterrupt:
    pass


if __name__ == '__main__':
  main()
