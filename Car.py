# Original Author: Bonnie Clifton

import time
import config
import RPi.GPIO as GPIO

from StepperDriverClass import StepperDriverClass

def CarManager():
	# The stepper driver is a class. Create an instance for the lift stepper motor and one for the door stepper motor.
	Car = StepperDriverClass(id, [31,29,7,5], 26, 24 ) # Create an instance of the stepper motor driver.
	
	
	# Begin car intialization to find the stepper motor steps required to move the car to the top floor
	# Bottom floor is like a reference position
	print ('CarManager: Moving to bottom floor')
	Car.moveMotor(-1000000)
	time.sleep(.5)
	
	print ('CarManager: Moving to top floor to count steps')
	# Will stop when car reaches limit switch.
	totalSteps = Car.moveMotor(1000000)
	
	#the total steps is a measure of the distance from the bottom to the top floor
	#  Used to find the number of steps to a given floor (no detection device at each floor)
	print ("CarManager: Total steps: ", totalSteps)
	time.sleep(1)		# Pause may not be needed
		
	#print ('CarManager: Moving to bottom floor')
	Car.moveMotor(-1000000)
		
	# Setting parameters for directions, height (in steps) of elevator, and initial floor.
	floor = 1
	direction = 1
	stepsPerFloor = totalSteps / (topFloor - 1)
	
	# tell the master controller where this car is currently loacated (which will be on the current floor)
	UpdateMaster(config.CarFloorStopList)
	
	# ====================== MAIN LOOP ===============================
	print ('CarManager: Starting main loop')
	currentFloor = 1

	while True:
		# Poll the floor stop list continuously.
		# The floor poll will look ahead for floors to stop at.
		# If a floor stop is found, the car is "pulled" to that floor - up or sown.
		# Along the way,  at each floor is checked to see if a new stop has come in,
		#    either from inside the car or from the master controller.

		if config.CarFloorStopList[floor] == 1:
			# We are scanning the call list looking for a floor call (floor =  1 value)
			# We must physically move the car to this floor
			# The floor being checked may not be where the car is actually currently located.
			# It may be above or below the checked floor.
			while currentFloor != floor:
				# Move the car until a stop floor is reached.
				if (floor - currentFloor) > 0:
					# Move car up toward logical floor.
					moveDirection = 1
				else:
					#  otherwise we new o move it downward
					moveDirection = -1

				# Wait for the door to close
				Car.moveMotor(stepsPerFloor * moveDirection)		# Move one floor.
				currentFloor += moveDirection				# Now moved, update floor.
				config.CarFloorStopList[0] = currentFloor * direction	# Update list for new floor and direction.
			config.CarFloorStopList[currentFloor] = 0			# Clear list entry for this floor.
			clm.CarLampManager(currentFloor, 0) 				# Car lamp turned off for this floor
			UpdateMaster(config.CarFloorStopList)				# Tell master the floor where now located
	
			# Change direction if top or bottom floor reached.
		if floor > topFloor - 1:
			direction = -1
		if floor < bottomFloor + 1: 
			direction = 1
		time.sleep(1)
