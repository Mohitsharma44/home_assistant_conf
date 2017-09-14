import os
import logging
import asyncio

from hbmqtt.client import MQTTClient, ClientException
from hbmqtt.mqtt.constants import QOS_0

mqtt_host = "hassio.local"
username  = os.getenv("hassio_username")
password  = os.getenv("hassio_password")
topics    = ["home/sofa_lights/switch"]

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
            print("%d:  %s => %s" % (i, packet.variable_header.topic_name, str(packet.payload.data)))
        yield from C.unsubscribe([topic for topic in topics])
        yield from C.disconnect()
    except ClientException as ce:
        logger.error("Client exception: %s" % ce)
        
if __name__ == '__main__':
    asyncio.get_event_loop().run_until_complete(uptime_coro())
