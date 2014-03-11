import BaseHTTPServer
import base64
import json
import logging
import string
import subprocess

HOST_NAME = 'localhost'
PORT_NUMBER = 8765

def runCode(usercode):
  f = open('template.cpp', 'rb')
  template = string.Template(f.read())
  f.close()

  code = template.substitute(usercode=usercode)

  f = open('result.cpp', 'wb')
  f.write(code)
  f.close()

  msg = ""
  img = ""
  try:
    logging.info("compiling")
    msg = subprocess.check_output('ninja -C ../../out/Debug webtry'.split())
    try:
      logging.info("running")
      msg = subprocess.check_output('../../out/Debug/webtry'.split())
      f = open('foo.png', 'rb')
      img = base64.b64encode(f.read())
      f.close()
    except subprocess.CalledProcessError as e:
      logging.info(e)
      msg = e.output
  except subprocess.CalledProcessError as e:
    logging.info(e)
    msg = e.output

  retval = {
    'message': msg
  }
  if img:
    retval['img'] = img
  return retval

class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  def do_POST(self):
    logging.info("POST")
    body = ""
    l = self.rfile.readline()
    while l.strip() != "EOF":
      body += l
      l = self.rfile.readline()
    self.send_response(200)
    self.send_header("Content-type", "application/json")
    self.end_headers()
    resp = runCode(body)
    self.wfile.write(json.dumps(resp))
    self.end_headers()

  def do_GET(self):
    """Respond to a GET request."""
    self.send_response(200)
    self.send_header("Content-type", "text/html")
    self.end_headers()
    f = open('index.html', 'rb')
    self.wfile.write(f.read())
    f.close()

if __name__ == '__main__':
  server_class = BaseHTTPServer.HTTPServer
  httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
  logging.info("Server Start: %s:%s" % (HOST_NAME, PORT_NUMBER))
  try:
    httpd.serve_forever()
  except KeyboardInterrupt:
    pass
  httpd.server_close()
