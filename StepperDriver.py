import time
#import TestRPi as GPIO
import RPi.GPIO as GPIO
import Globals

# Allow for hystersis of the limit switches.
# It will take n steps for the limit switch to actually open.
#LimitSwitchHystersis = 300 <---- Not needed

# H-bridge sequence in manufacturers datasheet.
# Notice that from step to step, only one bit changes.

# Variables defined here are instance variables available in instances of the class only.
# Use BCM GPIO references instead of physical pin numbers.

stepMotorPins =  [31,29,7,5]
LSBottomPin = 26  	
LSTopPin = 24  	
stepCount = 0	
Seq = [[1,0,0,1], [1,0,0,0], [1,1,0,0], [0,1,0,0], [0,1,1,0], [0,0,1,0], [0,0,1,1], [0,0,0,1]]

def SetUp():
    GPIO.setmode(GPIO.BOARD)

    # Set up top and bottom limit switches.
    GPIO.setup(LSBottomPin,GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(LSTopPin,GPIO.IN, pull_up_down=GPIO.PUD_UP)

    # Set all pins as output and set to sync current.
    for pin in stepMotorPins:
        GPIO.setup(pin,GPIO.OUT)
        GPIO.output(pin, False)

# Variables defined here are called class variables and are in all instances of this class.
# Each group of 4 values (bits) are applied to the H-Bridge controller inputs by the RPi outputs pins.
def resetMotor():
    # Set both H-Bridges stepper driver to 0 volts to not draw power.
    for pin in stepMotorPins:
        GPIO.output(pin, False)

def MoveMotor(steps):
    # SetUp()
    print('MoveMotor: ', steps)
    stepDirection = 1
    if steps < 0 :
        stepDirection = -1
        steps = abs(steps)

    stepCount = 0
    stepSeqCounter = 0	
    print("sd: " , steps)
    while stepCount <= steps:
        # Each loop will rotate the stepper motor one step.
        if Globals.StopNow == True:
            # Emergency Stop
            resetMotor()
            return stepCount
          
        elif not GPIO.input(LSBottomPin) and stepDirection == -1:
            # At bottom, input is low/false when switch closes.
            # Can't go lower than bottom.
            print("StepperDriveClass: bottom limit reached")
            
            # Set both H-Bridges to 0 volts to not draw power.
            resetMotor()
            return stepCount

        elif not GPIO.input(LSTopPin) and stepDirection == 1:
            # At top, input is high/true when switch closes.
            # Can't go higher than top.
            print("StepperDriver: Top limit reached")

            # Set both H-Bridges to 0 volts to not draw power.
            resetMotor()

            return stepCount

        else:
            # Keep stepping in same direction.
            # Set the RPi 4 output pins the the values in the current sequence item.
            # Move motor one step.
            for pin in range(0, 4):
                xpin = stepMotorPins[pin]
                if Seq[stepSeqCounter][pin] != 0:
                    GPIO.output(xpin, True)
                else:
                    GPIO.output(xpin, False)
            stepSeqCounter += stepDirection
                
            # When we reach the end of the sequence start again.
            if stepSeqCounter >= len(Seq):
                stepSeqCounter = 0			
            elif  stepSeqCounter < 0:
                stepSeqCounter = len(Seq) + stepDirection
                
        stepCount += 1
        time.sleep(Globals.CarStepWaitTime) # Wait before moving on to next step.
    return stepCount
