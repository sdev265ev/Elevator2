
import time
import RPi.GPIO as GPIO
import Globals

# Allow for hystersis of the limit switches.
# It will take n steps for the limit switch to actually open.
#LimitSwitchHystersis = 300 <---- Not needed

#class StepperDriverClass(id, steps, StepMotorPins, limitSwitchBottomPin, limitSwitchTopPin):
class StepperDriverClass():
	#StepMotorPins = [6,5,4,3]
	#StepMotorPins = [31,29,7,5]
	# H-bridge sequence in manufacturers datasheet.
	# Notice that from step to step, only one bit changes.
		
    def __init__(self, id, StepMotorPins, LSBottomPin, LSTopPin ): 
        # Variables defined here are instance variables available in instances of the class only.
        # Use BCM GPIO references instead of physical pin numbers.
        GPIO.setmode(GPIO.BOARD)
        self.id = id
                
        self.stepMotorPins =  StepMotorPins	#[31,29,7,5]
        self.LSBottomPin = LSBottomPin		#26  		# = 7
        self.LSTopPin = LSTopPin		# 24  		# = 8
        self.Seq = [[1,0,0,1], [1,0,0,0], [1,1,0,0], [0,1,0,0], [0,1,1,0], [0,0,1,0], [0,0,1,1], [0,0,0,1]]

        # Set up top and bottom limit switches.
        GPIO.setup(self.LSBottomPin,GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.setup(self.LSTopPin,GPIO.IN, pull_up_down=GPIO.PUD_UP)
        
        # Set all pins as output and set to sync current.
        for pin in self.stepMotorPins:
            GPIO.setup(pin,GPIO.OUT)
            GPIO.output(pin, False)
	
# Variables defined here are called class variables and are in all instances of this class.
# Each group of 4 values (bits) are applied to the H-Bridge controller inputs by the RPi outputs pins.

    def moveMotor(self, steps):
        if Globals.StopNow == True:
            return 0

        stepDirection = 1
        if steps < 0 :
            stepDirection = -1
            steps = abs(steps)
        elif steps > 0:
            stepDirection = 1
        else:
            return steps
	
        stepCount = 0
        stepSeqCounter = 0	
        while stepCount <= steps:
            # Each loop will rotate the stepper motor one step.
            if Globals.StopNow == True:
                # Emergency Stop
                # Set both H-Bridges stepper driver to 0 volts to not draw power.
                for pin in self.StepMotorPins:
                    GPIO.output(pin, False)
                return stepCount

            elif not GPIO.input(self.LSBottomPin) and stepDirection == -1:
                # At bottom, input is low/false when switch closes.
                # Can't go lower than bottom.
                print("StepperDriveClass: bottom limit reached")

                # Set both H-Bridges to 0 volts to not draw power.
                for pin in self.stepMotorPins:
                    GPIO.output(pin, False)
                return 0

            elif not GPIO.input(self.LSTopPin) and stepDirection == 1:
                # At top, input is high/true when switch closes.
                # Can't go higher than top.
                print("StepperDriveClass: Top limit reached")

                #print (config.CarCurrentStepPosition)

                # Set both H-Bridges to 0 volts to not draw power.
                for pin in self.stepMotorPins:
                    GPIO.output(pin, False)

                return stepCount

            else:
                # Keep stepping in same direction.
                # Set the RPi 4 output pins the the values in the current sequence item.
                # Move motor one step.
                for pin in range(0, 4):
                    xpin = self.stepMotorPins[pin]
                    if self.Seq[stepSeqCounter][pin] != 0:
                        GPIO.output(xpin, True)
                    else:
                      GPIO.output(xpin, False)
                stepSeqCounter += stepDirection
					
                # When we reach the end of the sequence start again.
                if stepSeqCounter >= len(self.Seq):
                    stepSeqCounter = 0			
                elif  stepSeqCounter < 0:
                    stepSeqCounter = len(self.Seq) + stepDirection
					
            stepCount += 1
            time.sleep(Globals.CarStepWaitTime) # Wait before moving on to next step.
        return stepCount
