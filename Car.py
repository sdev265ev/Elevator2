# Original Author: Bonnie Clifton

import time
import Globals
# import RPi.GPIO as GPIO
import StepperDriverClass

# The stepper driver is a class. Create an instance for the lift stepper motor and one for the door stepper motor.
Car = StepperDriverClass(id, [31,29,7,5], 26, 24 ) # Create an instance of the stepper motor driver.

def MoveCar(steps):

	print ('CarManager: Moving to bottom floor')
	Car.moveMotor(steps)
	return 
	
	print ('CarManager: Moving to top floor to count steps')
	# Will stop when car reaches limit switch.
	return Car.moveMotor(1000000)

