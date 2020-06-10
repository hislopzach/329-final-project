import time
import signal
import board
import busio

import RPi.GPIO as GPIO

import adafruit_vcnl4010
import adafruit_tca9548a
# constants
num_sensors = 6
cup_sunk = [0]*num_sensors
thresholds = [4000]*num_sensors
# globals
sensor_array = [-1]*num_sensors
i2c = None
mux = None

RESTART_PIN = 17
channel_list = [19, 21, 23, 22, 24, 26]


def init():
    # init i2c bus
    i2c = busio.I2C(board.SCL, board.SDA)
    # initialize mux
    mux = adafruit_tca9548a.TCA9548A(i2c)

    # setup array of sensors
    for i in range(num_sensors):
        sensor_array[i] = adafruit_vcnl4010.VCNL4010(mux[i])
        # sensor_array[i] = adafruit_vcnl4010.VCNL4010(i2c)

    # set indicator outputs to logic low
    # GPIO.setmode(GPIO.BOARD)
    # GPIO.output(channel_list, GPIO.LOW)
    GPIO.setup(channel_list, GPIO.OUT)
    GPIO.output(channel_list, GPIO.LOW)

    # setup restart pin to reset array on the rising
    GPIO.setup(RESTART_PIN, GPIO.IN)
    GPIO.add_event_detect(RESTART_PIN, GPIO.RISING)
    GPIO.add_event_callback(RESTART_PIN, reset_pin_handler)
    # for i in range(num_sensors):
    #     print("cup ({}): ({})".format(i,sensor_array[i].proximity))
    # catch sigint and cleanup
    signal.signal(signal.SIGINT, sigint_handler)


def sigint_handler(signum, frame):
    cleanup()
    exit(0)


def reset_pin_handler(channel):
    reset_cups()


def reset_cups():
    cup_sunk = [0]*num_sensors


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


def show_state(cup_sunk):
    strs = ["x" if c == 1 else " " for c in cup_sunk]
    # print(strs)
    print("\n")
    print("({}) ({}) ({})".format(strs[3], strs[4], strs[5]))
    print("  ({}) ({})  ".format(strs[1], strs[2]))
    print("    ({})     ".format(strs[0]))


def main():
    init()
    # if not (i2c and mux):
    #    print("error initializing")
    #    exit(1)
    print("Sensors Ready:")
    show_state(cup_sunk)
    # print("initial values: ({})".format(get_sensor_values()))
    while True:
        for i in range(num_sensors):
            if not cup_sunk[i] and sensor_array[i].proximity > thresholds[i]:
                cup_sunk[i] = 1
                show_state(cup_sunk)
                # set gpio pin accordingly
                set_gpio_output(i)

    cleanup()


if __name__ == '__main__':
    main()
