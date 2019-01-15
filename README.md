# Model-Railroad

I put this together on a Sunday morning...so it is rough. But I will work to refine it and put update with an explaintion so people can find it more useful.

MQTT_Sensor_for_Railroad.ino = ESP8266 code that puts messages on MQTT of the state of sensors, in my case IR detectors. It also takes in a JSON messages to change the config. There are various ideas on this as to how to make it more auto-config, but my idea is to make it so that the EXACT same piece of code can run on multiples ESPs and I dont have to change the code.

Output

MQTT Topic: [trains/sensors/84:F3:EB:0D:26:0F/14]
Payload: {"type":"sensor","data":{"name":"84:F3:EB:0D:26:0F,"state":OFF}} 

The pattern is trains/sensors/MAC/PIN# and MAC.PIN# in the JSON. I am working on the node-red part of parsing this to send it to JMRI via websocket since the MQTT interface is nascent.

Then on trains/sensors/ESP/84:F3:EB:0D:26:0F I push a JSON of {"type":"sensor","mac":"84:F3:EB:0D:26:0F","delay":100,"bounce":"5"} to change the delay in the reads to 100 and the bounce to 5 loops. This will send ON as soon as a read happens, but will not send OFF until 5 loops have happened. In case that is not clear, if delay is 100 and bounce is 5, that is half a second.

I am going to test this debouncing style for a while and see what happens.

Remeber, you can pass in changes to this once the sensor is deployed using correct topic and JSON mentioned above. I have a sense that IR sensors low and on curves will have a lot of noise...I also dont need to count the cars, which if you got "crazy" you might be able to do by counting the offs before the final offs, then subtracting the bounce #. Not sure where you'd put that into JMRI and what you'd do with it, but maybe you could define a train as having 5 cars and wait for all 5 to go by before declaring a block change complete. I am not that advanced yet!

Nod RED Flow

1 - take in the MQTT, check that it is formed right, then change on/off to 4/2 for JMRI, post to websocket.

2 - a ping messages is sent every 13 seconds to keep the websocket open.

![node red image](/MQTT_Sensor_for_Railroad/node-red-layout.png)

Tools:
1 - MQTTbox
