# CPE-301-Final-Project
# Jeremy Mamaril and Brandon YU

# Objective
The purpose of this project was to create a swamp cooler that transitions through several states depending on whether or not the temperature of the water or the water level is below or above a threshold. The states that the cooler is running in are running, idle, error, and disabled, and the current state it is running in is indicated by lighting the corresponding LED. Output is also displayed by the serial monitor and the LCD display. The main components of the cooler are the Arduino Mega 2560 and the tools in the lab kit.

# Design Overview
The two members of our team did our best to implement the appropriate software and hardware to create a swamp cooler system that transitions through several states according to the temperature and humidity levels. Along with the Arduino ATMega 2560, the components we used from the kit are: a water level sensor, a stepper and fan motor, a potentiometer for controlling the vent direction, an LCD display for outputting messages about the current state, a temperature and humidity sensor, LEDs, buttons, and a real time clock for outputting exact times to the serial monitor.

The cooler transitions to the appropriate states according to the state diagram whenever the temperature or the water level is below or above a predefined threshold. These states are disabled, idle, error, and running, and some of these states can be transitioned to by the user, and the current state is signified by a corresponding LED. At every state, the motors are either on or off, and the temperature and humidity levels are consistently displayed to the LCD display and the serial monitor. Additionally, the real-time clock module was used to record the exact time a value was read or a state changed.

Finally, the sensors we used to determine the water levels, temperature, and humidity levels were the DHT and water level sensors. The values extracted from these sensors determined which state the cooler had to be in. Lastly, in every state besides the error state, buttons could be used to control the vent direction to rotate the stepper motor to the desired angle and orientation.

# Constraints
Certain constraints for this project were the fact that we could only use the components in our lab kit and not much else and only the Arduino ATMega 2560 and not another kind of microcontroller. In addition, it was difficult not being able to use the library functions for GPIO, ADCs, and UARTS, but instead had to use the registers and bit manipulation. We therefore used the specification sheets shown below to help with this. The time limit was also quite a constraint, especially at this last stretch of the semester.
