#!/usr/bin/env pybricks-micropython

from pybricks.hubs import EV3Brick
from pybricks.ev3devices import Motor
from pybricks.parameters import Port
from pybricks.tools import wait


ev3 = EV3Brick()

left_motor = Motor(Port.B)
right_motor = Motor(Port.C)


# =========================================
# SETTINGS
# =========================================

MAX_SPEED = 300
TURN_SPEED = 120

ACCELERATION_TIME = 1500     # 1.5 seconds
DECELERATION_TIME = 1500     # 1.5 seconds

TURN_TIME = 5000


# =========================================
# FUNCTION: MOVE FORWARD SMOOTHLY
# =========================================

def move_forward_smoothly(straight_time):

    # -------------------------
    # ACCELERATION
    # -------------------------

    steps = 20
    delay = ACCELERATION_TIME // steps

    for i in range(1, steps + 1):

        speed = int(MAX_SPEED * i / steps)

        left_motor.run(speed)
        right_motor.run(speed)

        wait(delay)


    # -------------------------
    # CONSTANT SPEED
    # -------------------------

    left_motor.run(MAX_SPEED)
    right_motor.run(MAX_SPEED)

    wait(straight_time)


    # -------------------------
    # DECELERATION
    # -------------------------

    for i in range(steps, 0, -1):

        speed = int(MAX_SPEED * i / steps)

        left_motor.run(speed)
        right_motor.run(speed)

        wait(delay)


    left_motor.brake()
    right_motor.brake()


# =========================================
# FUNCTION: TURN LEFT ON ITS OWN AXIS
# =========================================

def turn_left():

    left_motor.run(-TURN_SPEED)
    right_motor.run(TURN_SPEED)

    wait(TURN_TIME)

    left_motor.brake()
    right_motor.brake()


# =========================================
# FUNCTION: TURN RIGHT ON ITS OWN AXIS
# =========================================

def turn_right():

    left_motor.run(TURN_SPEED)
    right_motor.run(-TURN_SPEED)

    wait(TURN_TIME)

    left_motor.brake()
    right_motor.brake()


# =========================================
# MAIN PROGRAM
# =========================================

ev3.speaker.beep()

move_forward_smoothly(2000)   # 2 seconds at maximum speed

wait(100)

turn_right()

# 1.5 seconds at maximum speed

ev3.speaker.beep()
