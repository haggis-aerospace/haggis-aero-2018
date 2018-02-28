import SocketServer
import threading
import time


class Letter:
    def __init__(self, char, width, height, x, y, pos, avSize):
        self.letter = char
        self.width = width
        self.height = height
        self.x = x
        self.y = y
        self.pos = pos
        self.avSize = avSize
        self.time = int(round(time.time() * 1000))


letter = Letter(None, 0, 0, 0, 0, 0, 0)
client_connected = False


class Server(object):
    server = None
    global client_connected

    def __init__(self, host="0.0.0.0", port=5463):
        global server
        server = ThreadedTCPServer((host, port), ThreadedTCPRequestHandler)
        server_thread = threading.Thread(target=server.serve_forever)
        server_thread.daemon = True
        server_thread.start()

    def isClientActive(self):
        return client_connected

    def getLastLetter(self):
        global letter
        return letter


class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        global letter, client_connected
        while True:
            data = self.request.recv(100)
            if not data:
                print "Client disconnected"
                client_connected = False
                break
            print "Received: " + str(data)
            values = data.split(",")
            if len(values) != 7:
                print "Error, invalid data received"
                letter = Letter("~", 0, 0, 0, 0, 0, 0)
                time.sleep(1)
            else:
                if not client_connected:
                    client_connected = True
                letter = Letter(values[0], int(values[1]), int(values[2]), int(values[3]),
                                int(values[4]), int(values[5]), int(values[6]))


class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass
