from machine import Pin
from litter_box.settings import get_l298n_in1, get_l298n_in2, get_rotate_direction_reversed

backward = Pin(get_l298n_in1(), Pin.OUT)
forward = Pin(get_l298n_in2(), Pin.OUT)


def clock_wise():
    print("  rotating clockwise...")
    forward.value(True ^ get_rotate_direction_reversed())
    backward.value(False ^ get_rotate_direction_reversed())


def counter_clock_wise():
    print("  rotating counter clockwise...")
    forward.value(False ^ get_rotate_direction_reversed())
    backward.value(True ^ get_rotate_direction_reversed())


def stop():
    print("  stopping...")
    forward.value(False ^ get_rotate_direction_reversed())
    backward.value(False ^ get_rotate_direction_reversed())
