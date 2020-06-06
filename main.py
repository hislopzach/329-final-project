import time

import board
import busio

import RPi.GPIO as GPIO

import adafruit_vcnl4010
import adafruit_tca9548a
# constants
num_sensors = 1
channel_list = [i for i in range(num_sensors)]
cup_sunk = [0]*num_sensors
thresholds = [4000]*num_sensors
# globals
sensor_array = [-1]*num_sensors
i2c = None
mux = None


def init():
    # init i2c bus
    i2c = busio.I2C(board.SCL, board.SDA)
    # initialize mux
    # mux = adafruit_tca9548a.TCA9548A(i2c)

    # setup array of sensors
    for i in range(num_sensors):
        # sensor_array[i] = adafruit_vcnl4010.VCNL4010(mux[i])
        sensor_array[i] = adafruit_vcnl4010.VCNL4010(i2c)

    # set indicator outputs to logic low
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(channel_list, GPIO.OUT)
    GPIO.output(channel_list, GPIO.LOW)


def cleanup():
    GPIO.cleanup()


def write_state():
    # write current state of sensors to gpio pins
    for i in range(num_sensors):
        GPIO.output(channel_list[i], cup_sunk[i])


def set_gpio_output(cup_ndx: int):
    GPIO.output(channel_list[cup_ndx], cup_sunk[cup_ndx])


def get_sensor_values():
    values = [0]*num_sensors
    for i in range(num_sensors):
        values[i] = sensor_array[i].proximity
    return values


def main():
    # Initialize I2C bus and VCNL4010 module.
    init()
    if not (i2c and mux):
        print("error initializing")
        exit(1)
    print("Sensors Ready:")
    print("initial values: {}".format(get_sensor_values()))
    while True:
        for i in range(num_sensors):
            if not cup_sunk[i] and sensor_array[i].proximity > thresholds[i]:
                cup_sunk[i] = 1
                print("Cup {} has been sunk".format(i))
                # set gpio pin accordingly
                set_gpio_output(i)

    cleanup()


if __name__ == '__main__':
    main()
