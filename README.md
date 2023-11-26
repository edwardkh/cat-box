# Litter Box
 
## Prepare the esp32s2 mini.

### Find the connection for board
Run `ls /dev/tty*` with the board unplugged and again with it plugged in.
Find the path to the added device and set it to `DEVICE_CONNECTION`, example:
```shell
export DEVICE_CONNECTION=/dev/ttyACM1
```

### Download the firmware
```shell
curl -o micropython.bin https://micropython.org/resources/firmware/LOLIN_S2_MINI-20231005-v1.21.0.bin 
````

### Flash micropython onto the board
Make S2 boards into Device Firmware Upgrade (DFU) mode.
* Hold on Button 0 
* Press Button Reset 
* Release Button 0 When you hear the prompt tone on usb reconnection

```shell
esptool.py --chip esp32s2 --port $DEVICE_CONNECTION erase_flash
esptool.py --chip esp32s2 --port $DEVICE_CONNECTION write_flash -z 0x1000 micropython.bin
```

### Customize the software (optional)
#### Configure web connection
In the `src/wifi` directory copy `default_secrets.py` to `secrets.py` and enter values for your network connection.
If `base_url` is none the devices ip address will be used directly.

#### Configure alerts
Alerts require a web connection. In the `src/alert` directory copy `default_secrets.py` to `secrets.py` and enter values for 
your email.

### Upload the software
Load the contents of the src directory to the board.

## Wire it up

[//]: # (Todo add a digram)

## Run it
