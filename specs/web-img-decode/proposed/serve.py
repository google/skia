# Copyright 2018 Google LLC

# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import SimpleHTTPServer
import SocketServer

PORT = 8005

class Handler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    pass

Handler.extensions_map['.js'] = 'application/javascript'
# Without the correct MIME type, async compilation doesn't work
Handler.extensions_map['.wasm'] = 'application/wasm'

httpd = SocketServer.TCPServer(("", PORT), Handler)

httpd.serve_forever()
