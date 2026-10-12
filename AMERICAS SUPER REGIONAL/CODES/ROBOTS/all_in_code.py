# ================= IMPORTS =================

from pybricks.hubs import PrimeHub

from pybricks.pupdevices import Motor, ColorSensor

from pybricks.parameters import Port

from pybricks.tools import wait

from pybricks.parameters import Color

from usys import stdin

from uselect import poll



# ================= HUB =================

hub = PrimeHub(

    broadcast_channel=4,

    observe_channels=[4]

)



# ================= MOTORS =================

motorA = Motor(Port.A)

motorB = Motor(Port.B)

motorC = Motor(Port.C)

motorD = Motor(Port.D)



# ================= SENSOR =================

# ================= SENSOR =================



left_color_sensor = ColorSensor(Port.E)

right_color_sensor = ColorSensor(Port.F)



sensorE = ColorSensor(Port.E)

sensorF = ColorSensor(Port.F)



# ================= SETTINGS =================

WHEEL_DIAMETER_CM = 4.8

PI = 3.1416

DEGREES_PER_CM = 360 / (PI * WHEEL_DIAMETER_CM)



KP = 1.0

KI = 0.0

KD = 0.2



BASE_SPEED = 300

MAX_CORRECTION = 150



RAMP_UP_DIST = 7 * DEGREES_PER_CM

RAMP_DOWN_DIST = 12 * DEGREES_PER_CM



# ================= LINE FOLLOW =================

TARGET = 45



KP_LINE = 7

KD_LINE = 2



LINE_SPEED = 260

MAX_TURN = 400



# ================= COLOR CONSTANTS =================



BLACK = Color.BLACK

BLUE = Color.BLUE

BROWN = Color.BROWN

CYAN = Color.CYAN

GRAY = Color.GRAY

GREEN = Color.GREEN

MAGENTA = Color.MAGENTA

RED = Color.RED

YELLOW = Color.YELLOW

WHITE = Color.WHITE



# ================= PID =================

class PID:

    def __init__(self, kp, ki, kd):

        self.kp = kp

        self.ki = ki

        self.kd = kd

        self.previous_error = 0

        self.integral = 0



    def calculate(self, error):

        self.integral += error

        derivative = error - self.previous_error

        self.previous_error = error



        output = (self.kp * error) + (self.ki * self.integral) + (self.kd * derivative)



        if output > MAX_CORRECTION:

            output = MAX_CORRECTION

        elif output < -MAX_CORRECTION:

            output = -MAX_CORRECTION



        return output



# ================= BASE =================

def reset_motors():

    motorA.reset_angle(0)

    motorB.reset_angle(0)

    motorC.reset_angle(0)

    motorD.reset_angle(0)



def average_position():

    return (

        abs(motorA.angle()) +

        abs(motorB.angle()) +

        abs(motorC.angle()) +

        abs(motorD.angle())

    ) / 4



def apply_speed(motor, speed):

    speed = int(speed)

    if abs(speed) < 10:

        speed = 0

    motor.run(speed)



# ================= SENSOR =================

def saw_black():

    return color_sensor.reflection() < 35



# ================= RAMP =================

def calculate_ramp(progress, target, use_deceleration):



    x = min(progress / RAMP_UP_DIST, 1) if RAMP_UP_DIST > 0 else 1



    factor = 0.3 + (0.55 * (progress ** 0.5))



    if progress < RAMP_UP_DIST:

        factor = 0.35 + (x * x) * 0.5



    if use_deceleration and target is not None:

        remaining = target - progress



        if remaining < RAMP_DOWN_DIST:

            x2 = max(remaining / RAMP_DOWN_DIST, 0) if RAMP_DOWN_DIST > 0 else 0

            decel = 0.25 + (x2 * x2) * 0.5

            factor = min(factor, decel)



    factor = max(0.25, min(factor, 0.85))

    return factor



# ================= MOVEMENT =================

def move(vA, vB, vC, vD, distance_cm=None, speed=1, stop_on_black=False):



    reset_motors()



    target_distance = None

    if distance_cm is not None:

        target_distance = distance_cm * DEGREES_PER_CM



    pid = PID(KP, KI, KD)



    hub.imu.reset_heading(0)

    wait(50)



    target_yaw = 0



    filtered_error = 0

    ALPHA = 0.6



    while True:



        progress = average_position()



        if target_distance is not None and progress >= target_distance:

            break



        if stop_on_black and saw_black():

            break



        ramp_factor = calculate_ramp(

            progress,

            target_distance,

            use_deceleration=not stop_on_black

        )



        yaw = hub.imu.heading()

        error = target_yaw - yaw



        filtered_error = (ALPHA * filtered_error) + ((1 - ALPHA) * error)



        correction = pid.calculate(filtered_error)



        base = BASE_SPEED * speed * ramp_factor



        sA = (vA * base) - correction

        sB = (vB * base) + correction

        sC = (vC * base) - correction

        sD = (vD * base) + correction



        apply_speed(motorA, sA)

        apply_speed(motorB, sB)

        apply_speed(motorC, sC)

        apply_speed(motorD, sD)



        wait(5)



    motorA.stop()

    motorB.stop()

    motorC.stop()

    motorD.stop()



# ================= LINE FOLLOWER =================



def follow_line_left(cm=None, vel=1, color=RED):



    reset_motors()



    target = None



    if cm is not None:

        target = cm * DEGREES_PER_CM



    previous_error = 0



    while True:



        # ================= STOP CONDITIONS =================



        if target is not None and average_position() >= target:

            break



        if color is not None:

            if sensorE.color() == color or sensorF.color() == color:

                break



        # ================= SENSOR READING =================



        reading = sensorE.reflection()



        # Keeps the same logic as the robot that already works

        error = reading - TARGET



        # ================= PD CONTROLLER =================



        proportional = error * KP_LINE

        derivative = (error - previous_error) * KD_LINE



        correction = proportional + derivative



        previous_error = error



        # ================= LIMITER =================



        if correction > MAX_TURN:

            correction = MAX_TURN



        elif correction < -MAX_TURN:

            correction = -MAX_TURN



        # ================= SPEED =================



        base = LINE_SPEED * vel



        curve = abs(error) * 3

        speed = base - curve



        if speed < 120:

            speed = 120



        left = speed - correction

        right = speed + correction



        # ================= MOTORS =================

        # Inverted directions for the second robot assembly



        motorA.run(left)

        motorB.run(-right)



        motorC.run(left)

        motorD.run(-right)



        wait(5)



    stop_motors()



# ================= LINE FOLLOWER =================



def follow_line_right(cm=None, vel=1, color=RED):



    reset_motors()



    target = None



    if cm is not None:

        target = cm * DEGREES_PER_CM



    previous_error = 0



    while True:



        # ================= STOP CONDITIONS =================



        if target is not None and average_position() >= target:

            break



        if color is not None:

            if sensorE.color() == color or sensorF.color() == color:

                break



        # ================= SENSOR READING =================



        reading = sensorF.reflection()



        # Keeps the same logic as the robot that already works

        error = TARGET - reading



        # ================= PD CONTROLLER =================



        proportional = error * KP_LINE

        derivative = (error - previous_error) * KD_LINE



        correction = proportional + derivative



        previous_error = error



        # ================= LIMITER =================



        if correction > MAX_TURN:

            correction = MAX_TURN



        elif correction < -MAX_TURN:

            correction = -MAX_TURN



        # ================= SPEED =================



        base = LINE_SPEED * vel



        curve = abs(error) * 3

        speed = base - curve



        if speed < 120:

            speed = 120



        left = speed - correction

        right = speed + correction



        # ================= MOTORS =================

        # Inverted directions for the second robot assembly



        motorA.run(left)

        motorB.run(-right)



        motorC.run(left)

        motorD.run(-right)



        wait(5)



    stop_motors()

# ================= TURN =================

# ================= TURN =================

def stop_motors():

    motorA.stop()

    motorB.stop()

    motorC.stop()

    motorD.stop()





def turn_right(degrees, vel=1):



    hub.imu.reset_heading(0)

    wait(200)



    while True:



        yaw = hub.imu.heading()



        error = degrees - abs(yaw)



        # ===== STOP =====

        if error <= 1:

            break



        # ===== DECELERATION =====

        vmax = BASE_SPEED * vel

        vmin = 90



        factor = error / degrees



        if factor > 1:

            factor = 1



        speed = (vmax * factor)



        # ===== MINIMUM SPEED =====

        if speed < vmin:

            speed = vmin



        # ===== TURN RIGHT =====

        motorA.run(-speed)

        motorB.run(-speed)

        motorC.run(-speed)

        motorD.run(-speed)



        wait(10)



    stop_motors()





def turn_left(degrees, vel=1):



    hub.imu.reset_heading(0)

    wait(200)



    while True:



        yaw = hub.imu.heading()



        error = degrees - abs(yaw)



        # ===== STOP =====

        if error <= 1:

            break



        # ===== DECELERATION =====

        vmax = BASE_SPEED * vel

        vmin = 90



        factor = error / degrees



        if factor > 1:

            factor = 1



        speed = (vmax * factor)



        # ===== MINIMUM SPEED =====

        if speed < vmin:

            speed = vmin



        # ===== TURN LEFT =====

        motorA.run(speed)

        motorB.run(speed)

        motorC.run(speed)

        motorD.run(speed)



        wait(10)



    stop_motors()



# ================= MOVEMENTS =================

def forward(cm=None, vel=1, black=False):

    move(1, -1, 1, -1, cm, vel, black)



def backward(cm=None, vel=1, black=False):

    move(-1, 1, -1, 1, cm, vel, black)



def left(cm=None, vel=1, black=False):

    move(-1, -1, 1, 1, cm, vel, black)



def right(cm=None, vel=1, black=False):

    move(1, 1, -1, -1, cm, vel, black)



def diag_forward_right(cm=None, vel=1, black=False):

    move(1, 0, 0, -1, cm, vel, black)



def diag_forward_left(cm=None, vel=1, black=False):

    move(0, -1, 1, 0, cm, vel, black)



def diag_backward_right(cm=None, vel=1, black=False):

    move(0, 1, -1, 0, cm, vel, black)



def diag_backward_left(cm=None, vel=1, black=False):

    move(0, -1, 1, 0, cm, vel, black)



# ================= BLE =================



def send_ble(signal):

    print("Sending BLE:", signal)



    hub.ble.broadcast(signal)



    # Gives other devices time to receive the broadcast

    wait(300)



    print("BLE sent:", signal)



def wait_to_start():



    print("Waiting for BLE signal 2...")



    hub.light.on(Color.ORANGE)



    while True:



        signal = hub.ble.observe(7)



        if signal == "COMEÇAR":



            print("BLE signal 2 received!")



            hub.light.on(Color.GREEN)

            hub.speaker.beep()



            # Short wait to confirm reception

            wait(300)



            break



        wait(100)



def wait_for_jorgen():



    stop_motors()



    print("Waiting for BLE JORGEN signal...")



    hub.light.on(Color.ORANGE)



    while True:



        signal = hub.ble.observe(7)



        if signal == 71:



            print("START signal received!")



            hub.light.on(Color.GREEN)

            hub.speaker.beep()



            wait(300)



            break



        wait(100)



def forward_until_yellow(vel=1):



    print("Moving forward until blue is found...")



    hub.light.on(Color.ORANGE)



    speed = BASE_SPEED * vel



    while True:



        colorE = sensorE.color()

        colorF = sensorF.color()



        # Either sensor detects the yellow line

        if colorE == Color.YELLOW or colorF == Color.YELLOW:



            stop_motors()



            print("Blue line recognized!")

            print("Sending BLE INICIAR signal...")



            hub.light.on(Color.BLUE)

            hub.speaker.beep()



            hub.ble.broadcast("INICIAR")



            # Keeps the broadcast active so the computer can receive it

            wait(1500)



            print("INICIAR signal sent!")



            break



        # Forward movement

        motorA.run(speed)

        motorB.run(-speed)

        motorC.run(speed)

        motorD.run(-speed)



        wait(20)



    stop_motors()



def feedback(era):



    # ===== MATRIX DRAWINGS =====



    color_value = None



    if era == "Egito":

        color_value = Color.YELLOW

        drawing = [

            [0, 0, 1, 0, 0],

            [0, 1, 1, 1, 0],

            [0, 1, 1, 1, 0],

            [1, 1, 1, 1, 1],

            [1, 1, 1, 1, 1]

        ]



    elif era == "Media":

        color_value = Color.GREEN

        drawing = [

            [1, 0, 1, 0, 1],

            [1, 1, 1, 1, 1],

            [1, 1, 0, 1, 1],

            [1, 0, 0, 0, 1],

            [1, 0, 0, 0, 1]

        ]



    elif era == "80":

        color_value = Color.BLUE

        drawing = [

            [1, 1, 1, 1, 1],

            [1, 0, 0, 0, 1],

            [1, 0, 0, 0, 1],

            [1, 1, 1, 1, 1],

            [1, 0, 0, 1, 1]

        ]



    elif era == "2026":

        color_value = Color.WHITE

        drawing = [

            [1, 1, 0, 1, 1],

            [0, 1, 0, 1, 0],

            [1, 1, 0, 1, 1],

            [1, 0, 0, 1, 1],

            [1, 1, 0, 1, 1]

        ]



    # ===== MATRIX =====



    hub.display.icon(

        [

            [100 if pixel else 0 for pixel in row]

            for row in drawing

        ]

    )



    # ===== HUB COLOR =====



    hub.light.on(color_value)



    # ===== DEFAULT SOUND =====



    hub.speaker.beep(800, 200)



    wait(500)



# ================= MAIN =================

def main():

    send_ble(1)

    # TIME MACHINE

    follow_line_right(None, 1, RED)

    send_ble(2)

    forward(30, 1.5)

    turn_right(110, 1.3)

    wait(1000)

    send_ble(3)

    # EGYPT

    forward(20, 1.5) 

    feedback("Egito")

    follow_line_left(None, 1, RED)

    send_ble(4)

    turn_left(180, 2)

    # TIME MACHINE

    follow_line_right(None, 1, YELLOW)

    send_ble(5)

    forward(30, 1.5)

    turn_right(110, 1.3)

    wait(1000)

    send_ble(6)

    # MIDDLE AGES

    forward(20, 1.5)

    feedback("Media")

    follow_line_left(None, 1, RED)

    send_ble(7)

    turn_left(180, 2)

    # TIME MACHINE

    follow_line_right(None, 1, GREEN)

    send_ble(8)

    forward(30, 1.5)

    turn_right(110, 1.3)

    wait(1000)

    send_ble(9)

    # 80s

    forward(20, 1.5)

    feedback("80")

    follow_line_left(None, 1, RED)

    send_ble(10)

    turn_left(180, 2)

    # TIME MACHINE

    follow_line_right(None, 1, BLUE)

    send_ble(11)

    forward(30, 1.5)

    turn_right(110, 1.3)

    wait("ACABOU")

    #diag_forward_right(15, 1.7)

    #diag_forward_left(15, 1.5)

    #wait(3000)

    #forward(10, 1.5)









main()




