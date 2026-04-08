import logging
from transitions import Machine
import time

class TapStateMachine:
    states = ['INITIALIZING', 'READY', 'POURING', 'DONE', 'DISCONNECTED']

    def __init__(self, hardware_service, mqtt_service, tap_id, tap_name):
        self.hw = hardware_service
        self.mqtt = mqtt_service
        self.tap_id = tap_id
        self.tap_name = tap_name
        
        self.pour_pulses = 0
        self.remaining_pulses = 0
        self.current_id = 0
        self.last_flow_time = 0
        self.initial_timeout = 10.0  # seconds
        self.continue_timeout = 3.0  # seconds

        # Initialize the state machine
        self.machine = Machine(
            model=self,
            states=TapStateMachine.states,
            initial='INITIALIZING'
        )

        # Transitions
        self.machine.add_transition('connected', ['INITIALIZING', 'DISCONNECTED'], 'READY')
        self.machine.add_transition('start_pour', 'READY', 'POURING')
        self.machine.add_transition('stop_pour', 'POURING', 'DONE')
        self.machine.add_transition('pour_finished', 'POURING', 'DONE')
        self.machine.add_transition('ready_up', 'DONE', 'READY')
        self.machine.add_transition('disconnect', '*', 'DISCONNECTED')

    def on_enter_INITIALIZING(self):
        print("FSM: Entering INITIALIZING state")
        self.hw.red()
        self.publish_state()

    def on_enter_READY(self):
        print("FSM: Entering READY state")
        self.hw.blue()
        self.publish_state()

    def on_enter_POURING(self):
        print(f"FSM: Entering POURING state (Target: {self.pour_pulses} pulses)")
        self.remaining_pulses = self.pour_pulses
        self.last_flow_time = time.time()
        self.hw.green()
        self.hw.set_valve(True)
        self.publish_state()

    def on_enter_DONE(self):
        print(f"FSM: Entering DONE state (Poured: {self.pour_pulses - self.remaining_pulses})")
        self.hw.set_valve(False)
        self.hw.red()
        self.publish_state()
        self.mqtt.publish_done(self.tap_id, self.pour_pulses - self.remaining_pulses, self.remaining_pulses)

    def on_enter_DISCONNECTED(self):
        print("FSM: Entering DISCONNECTED state")
        self.hw.all_on() # Simulated disconnected indicator
        self.publish_state()

    def publish_state(self):
        self.mqtt.publish_state(self.tap_id, self.tap_name, self.state)

    def handle_flow(self):
        if self.state == 'POURING':
            self.remaining_pulses -= 1
            self.last_flow_time = time.time()
            print(".", end="", flush=True)
            
            # Periodically publish flow status (like the 500ms timer in C++)
            # For simplicity, we can do it here or in a separate thread
            self.mqtt.publish_flow(self.tap_id, self.remaining_pulses, self.pour_pulses - self.remaining_pulses)

            if self.remaining_pulses <= 0:
                print("\nFSM: Pour target reached")
                self.pour_finished()

    def check_timeouts(self):
        if self.state == 'POURING':
            elapsed = time.time() - self.last_flow_time
            timeout = self.initial_timeout if self.remaining_pulses == self.pour_pulses else self.continue_timeout
            if elapsed > timeout:
                print(f"\nFSM: Pour timeout ({elapsed:.1f}s)")
                self.stop_pour()
