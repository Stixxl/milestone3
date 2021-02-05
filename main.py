import http.server
import shutil
import os

PORT = 8080


class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200)
        self.send_header("Content-type", "application/octet-stream")
        self.end_headers()
    def do_GET(self):
        if self.path == "/arima":
            src = open("models/arimamodel.fdml", "rb")
        elif self.path == "/markov":
            src = open("models/markovmodel.fdml", "rb")
        elif self.path == "/linreg":
            src = open("models/linregmodel.fdml", "rb")
        elif self.path == "/rnn":
            src = open("models/rnnmodel.zip", "rb")
        else:
            print(f"Model for path {self.path} does not exist.")
            self.send_response(404, f"Model for path {self.path} does not exist.")
            return
        self.send_response(200)
        self.send_header('Content-Disposition',  f'inline; filename=\"{os.path.basename(src.name)}\"')
        self.send_header("Content-type", "application/octet-stream")
        self.end_headers()
        shutil.copyfileobj(src, self.wfile)
        self.wfile.flush()


try:
    server = http.server.HTTPServer(('localhost', PORT), MyHandler)
    print('Started http server')
    server.serve_forever()
except KeyboardInterrupt:
    print('^C received, shutting down server')
    server.socket.close()
