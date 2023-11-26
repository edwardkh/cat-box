from machine import Pin, ADC
from litter_box.settings import get_hall_pin1, get_hall_pin2, get_load_sensor_pin, get_load_sensor_threshold

hall_sensor1 = Pin(get_hall_pin1(), Pin.IN)
hall_sensor2 = Pin(get_hall_pin2(), Pin.IN)

old_hall_sensor1 = True
old_hall_sensor2 = True

cat_sensor = ADC(Pin(get_load_sensor_pin()))

average_load = 44000
rolling_load = 44000


def hall_sensor_triggered():
    global old_hall_sensor1, old_hall_sensor2
    print("Hall1: "+str(hall_sensor1.value())+", Hall2: "+str(hall_sensor2.value()))
    hall_sensor1_triggered = not hall_sensor1.value() and old_hall_sensor1
    hall_sensor2_triggered = not hall_sensor2.value() and old_hall_sensor2
    old_hall_sensor1 = hall_sensor1.value()
    old_hall_sensor2 = hall_sensor2.value()
    return hall_sensor1_triggered or hall_sensor2_triggered


def calibrate_load():
    global average_load
    count = 10
    load = 0
    for i in range(count):
        load = load + cat_sensor.read_u16()
    average_load = load / 10
    print("Recalibrated to: " + str(average_load))


def cat_in_the_box():
    global rolling_load
    load = cat_sensor.read_u16()
    rolling_load = (2 * rolling_load + load) / 3
    print("load=" + str(load) + ", rolling:" + str(rolling_load))
    return rolling_load + get_load_sensor_threshold() < average_load
