import time
import board
import busio
import adafruit_drv2605
import sys

i2c = busio.I2C(board.SCL, board.SDA)
print("I2C devices found: ", [hex(i) for i in i2c.scan()])

drv = adafruit_drv2605.DRV2605(i2c)

effect_id = 1
while True:
    print(f"Playing effect #{effect_id}")
    drv.sequence[0] = adafruit_drv2605.Effect(effect_id)  # Set the effect on slot 0.
    # You can assign effects to up to 8 different slots to combine
    # them in interesting ways. Index the sequence property with a
    # slot number 0 to 7.
    # Optionally, you can assign a pause to a slot. E.g.
    # drv.sequence[1] = adafruit_drv2605.Pause(0.5)  # Pause for half a second
    drv.play()  # play the effect
    time.sleep(0.5)  # for 0.5 seconds
    drv.stop()  # and then stop (if it's still running)
    # Increment effect ID and wrap back around to 1.
    effect_id += 1
    if effect_id > 123:
        effect_id = 1
