import http.server
import shutil

PORT = 8080


class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200)
        self.send_header("Content-type", "application/octet-stream")
        self.end_headers()
    def do_GET(self):
        if self.path == "/arima":
            src = open("arimamodel.fdml")
        elif self.path == "/sarimax":
            src = open("sarimaxmodel.fdml")
        else:
            print(f"Model for path {self.path} does not exist.")
            self.send_response(404, f"Model for path {self.path} does not exist.")
            return
        self.send_response(200)
        self.send_header("Content-type", "application/octet-stream")
        self.end_headers()
        print(self.wfile)
        print(src)
        shutil.copyfileobj(src, self.wfile)
        self.wfile.flush()
        self.wfile.close()


try:
    server = http.server.HTTPServer(('localhost', PORT), MyHandler)
    print('Started http server')
    server.serve_forever()
except KeyboardInterrupt:
    print('^C received, shutting down server')
    server.socket.close()
