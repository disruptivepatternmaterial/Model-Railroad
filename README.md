# Model-Railroad


MQTT_Sensor_for_Railroad.ino = ESP8266 code that puts messages on MQTT of the state of sensors, in my case IR detectors. It also takes in a JSON messages to change the config. There are various ideas on this as to how to make it more auto-config, but my idea is to make it so that the EXACT same piece of code can run on multiples ESPs and I dont have to change the code.

Output

MQTT Topic: [trains/sensors/84:F3:EB:0D:26:0F/14]
Payload: {"type":"sensor","data":{"name":"84:F3:EB:0D:26:0F,"state":OFF}} 

The pattern is trains/sensors/MAC/PIN# and MAC.PIN# in the JSON. I am working on the node-red part of parsing this to send it to websocket since the MQTT interface is nascent.

Then on trains/sensors/ESP/84:F3:EB:0D:26:0F I push a JSON of {"type":"sensor","mac":"84:F3:EB:0D:26:0F","delay":100,"bounce":"asdfad"} to change the delay. Bounce is not implemented, but I want to think through how I use that to control the reporting state.
