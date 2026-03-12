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
        print("Cliente conectado:", self.client_address)
        """
        i2c = busio.I2C(board.SCL, board.SDA)
        if not adafruit_drv2605._DRV2605_ADDR in i2c.scan():
            print("Dispositivo 2065 no encontrado")
            sys.exit()

        drv = adafruit_drv2605.DRV2605(i2c)
        """
        while True:
            data = self.request.recv(16).strip()
            if not data:
                print("Cliente desconectado")
                self.server.shutdown()
                self.server.server_close()
                break

            print(data.decode(errors="ignore"))
            """
            drv.sequence[0] = adafruit_drv2605.Effect(data)
            drv.play()  # play the effect
            time.sleep(0.5)  # for 0.5 seconds
            drv.stop()
            """

if __name__ == "__main__":

    with socketserver.TCPServer((HOST, PORT), MyTCPHandler) as server:
        server.serve_forever()
