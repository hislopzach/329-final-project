import time
import signal
import board
import busio

import random
import RPi.GPIO as GPIO

import adafruit_vcnl4010
import adafruit_tca9548a
# constants
num_sensors = 6
cup_sunk = [0]*num_sensors
thresholds = [3600]*num_sensors
# globals
sensor_array = [-1]*num_sensors
i2c = None
mux = None

RESTART_PIN = 22
channel_list = [10, 9, 11, 25, 8, 7]
reset_mode = False

def init():
    # init i2c bus
    i2c = busio.I2C(board.SCL, board.SDA)
    # initialize mux
    mux = adafruit_tca9548a.TCA9548A(i2c)

    # setup array of sensors
    for i in range(num_sensors):
        sensor_array[i] = adafruit_vcnl4010.VCNL4010(mux[i])

    # set indicator outputs to logic low
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(channel_list, GPIO.OUT)
    GPIO.output(channel_list, GPIO.LOW)

    # setup restart pin to reset array on the rising
    GPIO.setup(RESTART_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.add_event_detect(RESTART_PIN, GPIO.RISING)
    GPIO.add_event_callback(RESTART_PIN, reset_pin_handler)
    # catch sigint and cleanup
    signal.signal(signal.SIGINT, sigint_handler)
    pin_status()


def sigint_handler(signum, frame):
    GPIO.output(channel_list, GPIO.LOW)
    cleanup()
    exit(0)


def reset_pin_handler(channel):
    global reset_mode
    reset_mode = True
    reset_cups()

def pin_status():
    status = []
    for pin in channel_list:
        status.append(GPIO.input(pin))
    print(status)

def reset_cups():
    global cup_sunk
    GPIO.output(channel_list, GPIO.LOW)
    print("reset_mode: {}".format(reset_mode))
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
    global reset_mode
    init()
    # if not (i2c and mux):
    #    print("error initializing")
    #    exit(1)
    print("Sensors Ready:")
    pin_status()
    show_state(cup_sunk)
    # print("initial values: ({})".format(get_sensor_values()))
    while True:
        # if reset signal has been received, wait for signal to go low before continuing
        if reset_mode:
            print("waiting for reset to go low")
            pin_value = GPIO.input(RESTART_PIN)
            while pin_value:
                pin_value = GPIO.input(RESTART_PIN)
            reset_mode = False
            print("reset is low")
            time.sleep(2)
        else:
            # if random.randint(1,3) % 2 == 0:
            #     print(sensor_array[0].proximity)
            for i in range(num_sensors):
                if not cup_sunk[i] and sensor_array[i].proximity > thresholds[i]:
                    cup_sunk[i] = 1
                    show_state(cup_sunk)
                    # set gpio pin accordingly
                    set_gpio_output(i)
                    pin_status()
    
    cleanup()


if __name__ == '__main__':
    main()
