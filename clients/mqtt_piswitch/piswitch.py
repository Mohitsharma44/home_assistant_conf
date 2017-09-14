import os
import logging
import asyncio

from hbmqtt.client import MQTTClient, ClientException
from hbmqtt.mqtt.constants import QOS_0

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

##
# Custom Settings for the Pi's GPIO
# ----------------------------------
# Pin number | Device Name
# 6,19       | Sofa lights (x2)
# 13         | Sofa power
# 26         | Sofa accent lights
##

sofa_lights = [6, 19]
sofa_power  = [13]
sofa_accent = [26]

mqtt_host = "hassio.local"
username  = os.getenv("hassio_username")
password  = os.getenv("hassio_password")

topics    = ["home/livingroom/sofa_lights",
             "home/livingroom/sofa_accent",
             "home/livingroom/sofa_power"]

@asyncio.coroutine
def uptime_coro():
    C = MQTTClient()
    yield from C.connect('mqtt://{username}:{password}@{mqtt_host}'.format(username=username,
                                                                           password=password,
                                                                           mqtt_host=mqtt_host))
    yield from C.subscribe([(topic, QOS_0) for topic in topics])
    try:
        for i in range(1, 100):
            message = yield from C.deliver_message()
            packet = message.publish_packet
            print("%d:  %s => %s" % (i,
                                     packet.variable_header.topic_name,
                                     str(packet.payload.data)))
        yield from C.unsubscribe([topic for topic in topics])
        yield from C.disconnect()
    except ClientException as ce:
        logger.error("Client exception: %s" % ce)
        
if __name__ == '__main__':
    asyncio.get_event_loop().run_until_complete(uptime_coro())
