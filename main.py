import time

import board
import busio

import adafruit_vcnl4010


# Initialize I2C bus and VCNL4010 module.
i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_vcnl4010.VCNL4010(i2c)

num_sensors = 1
thresholds = [3100]


def main():
    # Initialize I2C bus and VCNL4010 module.
    i2c = busio.I2C(board.SCL, board.SDA)
    sensor = adafruit_vcnl4010.VCNL4010(i2c)

    while True:
        for i in range(num_sensors):
            if sensor.proximity > thresholds[i]:
                print("Ball in cup")
