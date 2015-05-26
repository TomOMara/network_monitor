import SimpleHTTPServer
import SocketServer
import socket
# minimal web server.  serves files relative to the
# current directory.

def local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('8.8.8.8', 0))
    return s.getsockname()[0]

PORT = 8080
Handler = SimpleHTTPServer.SimpleHTTPRequestHandler


httpd = SocketServer.TCPServer((local_ip(), PORT), Handler)

print "http://%s:%s/networkv2.html" % (local_ip(), PORT)
httpd.serve_forever()

