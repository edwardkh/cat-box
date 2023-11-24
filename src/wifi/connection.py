import asyncio
import time
import network

from wifi.secrets import wifi_ssid, wifi_password, wifi_ip_config
from wifi.settings import connection_backoff, connection_retries

try:
    import usocket as socket
except:
    import socket

wlan = network.WLAN(network.STA_IF)
last_connection_attempt = 0
connection_attempt = 0


def configured():
    return wifi_ssid and wifi_password


def init():
    if configured():
        wlan.active(True)
        if wifi_ip_config:
            wlan.ifconfig(wifi_ip_config)
        connect()


def ready_to_connect():
    global connection_attempt
    if last_connection_attempt + connection_backoff < time.time():
        connection_attempt = 0
    return connection_attempt <= connection_retries


def connect():
    global connection_attempt, last_connection_attempt
    if configured() and ready_to_connect() and not wlan.isconnected():
        last_connection_attempt = time.time()
        print('connecting to network...')
        connection_attempt = connection_attempt + 1
        wlan.connect(wifi_ssid, wifi_password)
        while not wlan.isconnected():
            pass
        print('Successfully connected to ' + wifi_ssid)
        print(wlan.ifconfig())


async def stay_connected():
    connect()
    await asyncio.sleep(10)
