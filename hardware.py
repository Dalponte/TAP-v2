from pymata4 import pymata4
import time

class HardwareService:
    PIN_RED = 45
    PIN_GREEN = 46
    PIN_BLUE = 44
    PIN_VALVE = 22
    PIN_FLOWMETER = 21
    PIN_BUTTON = 23

    def __init__(self, board=None):
        self.board = board if board else pymata4.Pymata4()
        
        # Setup Pins
        self.board.set_pin_mode_digital_output(self.PIN_RED)
        self.board.set_pin_mode_digital_output(self.PIN_GREEN)
        self.board.set_pin_mode_digital_output(self.PIN_BLUE)
        self.board.set_pin_mode_digital_output(self.PIN_VALVE)
        
        # Pullups/Inputs
        self.board.set_pin_mode_digital_input_pullup(self.PIN_BUTTON)
        self.board.set_pin_mode_digital_input(self.PIN_FLOWMETER)

    def set_red(self, state):
        self.board.digital_write(self.PIN_RED, 1 if state else 0)

    def set_green(self, state):
        self.board.digital_write(self.PIN_GREEN, 1 if state else 0)

    def set_blue(self, state):
        self.board.digital_write(self.PIN_BLUE, 1 if state else 0)

    def red(self):
        self.set_green(False)
        self.set_blue(False)
        self.set_red(True)

    def green(self):
        self.set_red(False)
        self.set_blue(False)
        self.set_green(True)

    def blue(self):
        self.set_red(False)
        self.set_green(False)
        self.set_blue(True)

    def all_on(self):
        self.set_red(True)
        self.set_green(True)
        self.set_blue(True)

    def set_valve(self, state):
        self.board.digital_write(self.PIN_VALVE, 1 if state else 0)

    def set_flow_callback(self, callback):
        def _cb(data):
            if data[2] == 1: # On Rising edge
                callback()
        self.board.set_pin_mode_digital_input(self.PIN_FLOWMETER, callback=_cb)

    def set_button_callback(self, callback):
        def _cb(data):
            if data[2] == 0: # Pressed (Pullup, so 0 is active)
                callback()
        self.board.set_pin_mode_digital_input_pullup(self.PIN_BUTTON, callback=_cb)
