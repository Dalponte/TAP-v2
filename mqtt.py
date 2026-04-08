import paho.mqtt.client as mqtt
import json

class MqttService:
    def __init__(self, broker, port, client_id):
        self.client = mqtt.Client(client_id)
        self.broker = broker
        self.port = port
        self.on_command_callback = None

    def connect(self):
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.connect(self.broker, self.port, 60)
        self.client.loop_start()

    def _on_connect(self, client, userdata, flags, rc):
        print(f"MQTT: Connected with result code {rc}")
        client.subscribe("tap/command")

    def _on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            if self.on_command_callback:
                self.on_command_callback(payload)
        except Exception as e:
            print(f"MQTT: Error parsing message: {e}")

    def publish_state(self, tap_id, tap_name, state):
        payload = {
            "state": state,
            "id": tap_id,
            "name": tap_name
        }
        self.client.publish("tap/state", json.dumps(payload))

    def publish_done(self, tap_id, poured, remaining):
        payload = {
            "data": [tap_id, poured, remaining]
        }
        self.client.publish("tap/done", json.dumps(payload))

    def publish_flow(self, tap_id, remaining, poured):
        # Binary or JSON - the C++ used both, let's stick to JSON for simplicity here
        payload = {
            "id": tap_id,
            "remaining": remaining,
            "poured": poured
        }
        self.client.publish("tap/flow", json.dumps(payload))

    def publish_input(self, tap_id, tag_id):
        payload = {
            "data": [tap_id, tag_id]
        }
        self.client.publish("tap/input", json.dumps(payload))
