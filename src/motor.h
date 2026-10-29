// motor — litter_box/rotate.py parity (L298N via in1/in2, direction XOR).
#pragma once

// Reads l298n_in1/in2 + rotate_direction_reversed at boot (Python read them
// at import time; runtime PATCH takes effect after reboot).
void motor_setup(void);
void motor_clock_wise(void);
void motor_counter_clock_wise(void);
void motor_stop(void);
