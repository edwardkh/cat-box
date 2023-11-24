from machine import Pin, ADC
from litter_box.settings import *

hall_sensor1 = Pin(hall_pin1, Pin.IN)
hall_sensor2 = Pin(hall_pin2, Pin.IN)

old_hall_sensor1 = False
old_hall_sensor2 = False

cat_sensor = ADC(Pin(load_sensor_pin))


def hall_sensor_triggered():
    global old_hall_sensor1, old_hall_sensor2
    hall_sensor1_triggered = hall_sensor1.value() and not old_hall_sensor1
    hall_sensor2_triggered = hall_sensor2.value() and not old_hall_sensor2
    old_hall_sensor1 = hall_sensor1.value()
    old_hall_sensor2 = hall_sensor2.value()
    return hall_sensor1_triggered or hall_sensor2_triggered


def cat_in_the_box():
    load = cat_sensor.read_u16()
    print("load=" + str(load))
    return load > load_sensor_threshold
