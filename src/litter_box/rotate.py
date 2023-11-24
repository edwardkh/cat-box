from machine import Pin
from litter_box.settings import *

backward = Pin(l298n_in1, Pin.OUT)
forward = Pin(l298n_in2, Pin.OUT)


def clock_wise():
    print("rotating clockwise...")
    forward.value(True ^ rotate_direction_reversed)
    backward.value(False ^ rotate_direction_reversed)


def counter_clock_wise():
    print("rotating counter clockwise...")
    forward.value(False ^ rotate_direction_reversed)
    backward.value(True ^ rotate_direction_reversed)


def stop():
    print("stopping...")
    forward.value(False ^ rotate_direction_reversed)
    backward.value(False ^ rotate_direction_reversed)
