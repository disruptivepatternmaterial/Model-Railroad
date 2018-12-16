# Model-Railroad


MQTT_Sensor_for_Railroad.ino = ESP8266 code that puts messages on MQTT of the state of sensors, in my case IR detectors. It also takes in a JSON messages to change the config. There are various ideas on this as to how to make it more auto-config, but my idea is to make it so that the EXACT same piece of code can run on multiples ESPs and I dont have to change the code.
