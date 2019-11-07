#!/usr/bin/env python3
import struct
import time
import keyboard
import argparse
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='A', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
args = parser.parse_args()
addr = args.addr.lower()
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = "46070149-f65e-4d59-a9ff-84420d87a4ca"# ["0149"] # TODO: add your characteristics

#
UP_VAL = 1
DOWN_VAL = 2
LEFT_VAL = 3
RIGHT_VAL = 4
STOP_VAL = 0


class RobotController():

    def __init__(self, address):

        self.robot = Peripheral(addr)
        print("connected")

        # keep state for keypresses
        self.pressed = {"up": False, "down": False, "right": False, "left": False}
        # TODO get service from robot
        # TODO get characteristic handles from service/robot
        # TODO enable notifications if using notifications
        # Get service
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        # Get characteristic
        self.ch = self.sv.getCharacteristics(CHAR_UUIDS)[0]

        keyboard.hook(self.on_key_event)

    def on_key_event(self, event):
        # print key name
        print(event.name)
        # if a key unrelated to direction keys is pressed, ignore
        if event.name not in self.pressed: return
        # if a key is pressed down
        if event.event_type == keyboard.KEY_DOWN:
            # if that key is already pressed down, ignore
            if self.pressed[event.name]: return
            # set state of key to pressed
            self.pressed[event.name] = True

            if event.name == "down":
                self.ch.write(bytes([DOWN_VAL]))
            elif event.name == "up":
                self.ch.write(bytes([UP_VAL]))
            elif event.name == "right":
                self.ch.write(bytes([RIGHT_VAL]))
            elif event.name == "left":
                self.ch.write(bytes([LEFT_VAL]))
        else:
            # set state of key to released
            self.pressed[event.name] = False
            # TODO write to characteristic to stop moving in this direction
            self.ch.write(bytes([STOP_VAL]))
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Use arrow keys to control robot')
