import time

import board
import busio

import adafruit_vcnl4010


# Initialize I2C bus and VCNL4010 module.
i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_vcnl4010.VCNL4010(i2c)

num_sensors = 1
cup_sunk = [0]*num_sensors
thresholds = [3100]


def main():
    # Initialize I2C bus and VCNL4010 module.
    i2c = busio.I2C(board.SCL, board.SDA)
    sensor = adafruit_vcnl4010.VCNL4010(i2c)
    print("Sensors Ready:")
    print("initial value: {}".format(sensor.proximity))
    while True:
        for i in range(num_sensors):
            if not cup_sunk[i] and sensor.proximity > thresholds[i]:
                cup_sunk[i] = 1
                print("Ball in cup {}".format(i))


if __name__ == '__main__':
    main()
