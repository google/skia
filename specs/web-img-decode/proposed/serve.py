#!/usr/bin/env python3
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import http.server
import socketserver

PORT = 8000

class Handler(http.server.SimpleHTTPRequestHandler):
    pass

Handler.extensions_map['.js'] = 'application/javascript'
# Without the correct MIME type, async compilation doesn't work
Handler.extensions_map['.wasm'] = 'application/wasm'

httpd = socketserver.TCPServer(("", PORT), Handler)

httpd.serve_forever()
