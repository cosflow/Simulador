import time
import board
#import lgpio
#import busio
#import adafruit_drv2605
import sys
import socketserver

HOST = '127.0.0.1'  # localhost
PORT = 45454        # LOCALPORT

class MyTCPHandler(socketserver.BaseRequestHandler):

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1).strip()
        print("{} wrote:".format(self.client_address[0]))
        print(self.data)

if __name__ == "__main__":

    """
    i2c = busio.I2C(board.SCL, board.SDA)
    if not adafruit_drv2605._DRV2605_ADDR in i2c.scan():
	    print("Dispositivo 2065 no encontrado")
	    sys.exit()

    drv = adafruit_drv2605.DRV2605(i2c)
    """

    with socketserver.TCPServer((HOST, PORT), MyTCPHandler) as server:
        server.serve_forever()
