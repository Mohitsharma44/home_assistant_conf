import os
import logging
import asyncio
import atexit
import RPi.GPIO as GPIO
from hbmqtt.client import MQTTClient, ClientException
from hbmqtt.mqtt.constants import QOS_0

##
# Custom Settings for the Pi's GPIO
# ----------------------------------
# Pin number | Device Name
# 6,19       | Sofa lights (x2)
# 13         | Sofa power
# 26         | Sofa accent lights
##

GPIO.setwarnings(False)

# Setting up logger
logger = logging.getLogger('root')
FORMAT = '[%(levelname)1s]: [%(asctime)-15s %(filename)s:%(lineno)s %(funcName)20s()] %(message)s'
logging.basicConfig(format=FORMAT)
logger.setLevel(logging.DEBUG)

switch_mode = {
    "on": GPIO.LOW,
    "off": GPIO.HIGH
}

gpio_config = {
    "sofa_lights": [6, 19],
    "sofa_power": [13],
    "sofa_accent": [26]
}

mqtt_host = "hassio.local"
username  = os.getenv("hassio_username")
password  = os.getenv("hassio_password")
topics    = ["home/livingroom/sofa_lights",
             "home/livingroom/sofa_accent",
             "home/livingroom/sofa_power"]

C = MQTTClient()

@asyncio.coroutine
def uptime_coro():
    """
    Subcribe to MQTT and keep listening for updates
    """
    yield from C.connect('mqtt://{username}:{password}@{mqtt_host}'.format(username=username,
                                                                           password=password,
                                                                           mqtt_host=mqtt_host))
    yield from C.subscribe([(topic, QOS_0) for topic in topics])
    try:
        while True:
            message = yield from C.deliver_message()
            packet = message.publish_packet
            topic_name = packet.variable_header.topic_name.split('/')[-1]
            gpio_channels = gpio_config.get(topic_name)
            if gpio_channels:
                logger.debug("{} => {}".format(packet.variable_header.topic_name,
                                               str(packet.payload.data)))
                GPIO.output(gpio_channels,
                            switch_mode.get(packet.payload.data.lower().decode('utf-8')))
    except ClientException as ce:
        logger.error("Client exception: {}".format(ce))
    except KeyboardInterrupt as kbe:
        cleanup()
        
@atexit.register
def cleanup():
    """
    Cleanup method to unsubscribe from a topic and disconnect
    gracefully
    """
    try:
        yield from C.unsubscribe([topic for topic in topics])
        yield from C.disconnect()
    except ClientException as ce:
        logger.error("Client exception in cleanup: {}".format(ce))
    
if __name__ == '__main__':
    # Set GPIO board type to BCM a.k.a "Broadcom SOC channel"
    # (the actual pin numbers next to the GPIO pins)
    GPIO.setmode(GPIO.BCM)
    # Set GPIO channel mode to OUT
    [GPIO.setup(channels, GPIO.OUT) for channels in list(gpio_config.values())]
    asyncio.get_event_loop().run_until_complete(uptime_coro())
