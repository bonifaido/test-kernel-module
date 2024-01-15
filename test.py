import http.server
import ssl

server_address = ('', 4433)
httpd = http.server.HTTPServer(server_address, http.server.SimpleHTTPRequestHandler)
ctx = ssl.SSLContext(protocol=ssl.PROTOCOL_TLS_SERVER)
ctx.load_cert_chain(certfile="cert.pem", keyfile="key.pem")
httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
httpd.serve_forever()

