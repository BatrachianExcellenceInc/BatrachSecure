#!/usr/bin/env python
# vim: fileencoding=utf-8

import BaseHTTPServer
import SimpleHTTPServer


class BtxSecFakeHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        elems = self.path.split('/')
        elems = elems[1:]
        if elems[0] != 'v1':
            self.send_response(400, 'Bad Request')
        else:
            self.send_response(200, 'OK')
        self.wfile.write('\r\n')

if __name__ == '__main__':
    httpd = BaseHTTPServer.HTTPServer(('127.0.0.1', 8888), BtxSecFakeHandler)
    httpd.serve_forever()

