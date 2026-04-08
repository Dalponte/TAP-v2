import time
import signal
import sys
from hardware import HardwareService
from mqtt import MqttService
from tap_fsm import TapStateMachine

# Configuration
BROKER = "192.168.4.2"
PORT = 1883
CLIENT_ID = "tap_python_controller"
TAP_ID = 99
TAP_NAME = "01"

class App:
    def __init__(self):
        # 1. Initialize Hardware
        self.hw = HardwareService()
        
        # 2. Initialize MQTT
        self.mqtt = MqttService(BROKER, PORT, CLIENT_ID)
        
        # 3. Initialize State Machine
        self.fsm = TapStateMachine(self.hw, self.mqtt, TAP_ID, TAP_NAME)
        
        # 4. Bind callbacks
        self.mqtt.on_command_callback = self.handle_command
        self.hw.set_flow_callback(self.fsm.handle_flow)
        self.hw.set_button_callback(self.handle_button)

    def handle_command(self, payload):
        """Processes incoming MQTT commands."""
        # Example command: {"tapId": 99, "commandType": 1, "param": 500}
        try:
            target_id = payload.get("tapId")
            cmd_type = payload.get("commandType")
            param = payload.get("param", 0)

            if target_id != TAP_ID:
                return

            if cmd_type == 1: # CMD_POUR
                print(f"App: Received POUR command ({param} pulses)")
                self.fsm.pour_pulses = param
                self.fsm.start_pour()
            elif cmd_type == 2: # CMD_CONTINUE
                print("App: Received CONTINUE command")
                self.fsm.ready_up()
        except Exception as e:
            print(f"App: Error processing command: {e}")

    def handle_button(self):
        """Handles physical button press."""
        print("App: Button pressed")
        self.mqtt.publish_input(TAP_ID, "tag-id")

    def run(self):
        print("App: Starting...")
        self.mqtt.connect()
        
        # Simulate connection event to move from INITIALIZING to READY
        time.sleep(2) # Give some time for MQTT connection
        self.fsm.connected()

        try:
            while True:
                # Main loop tasks
                self.fsm.check_timeouts()
                time.sleep(0.1) # 100ms loop interval (similar to C++ automaton run frequency)
        except KeyboardInterrupt:
            self.shutdown()

    def shutdown(self):
        print("\nApp: Shutting down...")
        self.hw.set_valve(False)
        self.hw.board.shutdown()
        sys.exit(0)

if __name__ == "__main__":
    app = App()
    app.run()
