import time
import board
#import busio
#import adafruit_drv2605
import sys
import socket

HOST = '127.0.0.1'  # localhost
PORT = 45454        # LOCALPORT
"""
i2c = busio.I2C(board.SCL, board.SDA)
if not adafruit_drv2605._DRV2605_ADDR in i2c.scan():
	print("Dispositivo 2065 no encontrado")
	sys.exit()

drv = adafruit_drv2605.DRV2605(i2c)
"""
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        conn, addr = s.accept()
        with conn:
            print(f'Cliente conectado desde {addr}')        
            while True:
                try:
                    data = conn.recv(1)
                    if not data:
                        print("Cliente desconectado")
                        break
                    
                    mensaje = data.decode('utf-8').strip()
                    print(f"Ratón: {mensaje}")
                    """
                    drv.sequence[0] = adafruit_drv2605.Effect(mensaje)
                    drv.play()  # play the effect
                    time.sleep(0.5)  # for 0.5 seconds
                    drv.stop() 
                    """
                    
                except Exception as e:
                    print(f"Error: {e}")
                    break
