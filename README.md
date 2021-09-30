# WaterPump
Low flow high static head water pump to empty my dehumidifier tank and water plants.
This pump uses a 60ml plastic syringe, two one-way valves, a DC gearmotor, and a microcontroller.

![Water Pump](/images/WaterPumpFigure.png)
 
## Motivation
There were a few motivations for this project:
1. I dislike having to go down to the basement to empty the dehumidifier tank all summer
2. Plants outside need to be watered regularly
3. I like projects that involve microcontrollers

## Requirements / Design Goals
* high static head (water needs to be pumped from the basement floor to hanging plants over the deck)
* self-priming
* can be run dry without damage
* automatically turn on and off
* IoT capable - can be connected to my home automation network
* low voltage
* low cost / use stuff I have laying around
* relatively quiet

I spent many unhappy hours searching online for an off-the-shelf pump to do this, but didn't find anything that met all my requirements. And, as I mentioned above, I like to design and build things like this.

## Design
The principal design elements:
* A 60ml medical syringe and two one-way valves for the water handling itself.
* The plunger of the syringe is pushed and pulled by a 1/4-20 threaded rod mounted on a ball bearing.
* A DC gearmotor turns the threaded rod
* Reflectance sensors that are used for
   1. gearmotor tachometer/odometer
   2. syringe plunger home position sensor
* Microcontroller with motor driver

The microcontroller reads a water level sensor in the tank that tells when the tank is full. When full the pump pumps a predetermined amount of water out of the tank (2L in this case)

### How it pumps
When the plunger is pulled out water is drawn in through the intake one-way valve, while the outflow one-way valve blocks back flow frome the output. When the plunger is pushed in water is pushed through the outflow one-way valves while the intake valve blocks flow back to the intake.

![Water Flow](/images/WaterFlow.png)

### Motor Control
The Pololu Baby Orangutan B robot controller has two built-in DC motor drivers. We use one of them to drive the DC gearmotor.

We need to know where the plunger is, and for that we use a simple tachometer / odometer consisting of a Pololu QTR-1A reflectance sensor and a piece of aluminum mounted on the gearmotor's extended shaft:

![Tachometer / Odometer](/images/TachometerOdometer.png)

This produces two pulses for every rotation of the motor shaft (input to the gearhead). Counting these pulses tells us how many times the threaded rod is rotated. The threaded rod moves the syringe plunger 1/20" for each rotation of the output shaft (output of the gearhead).

There is also a "home position" sensor that tells when the plunger is pulled out to a particular position or more. This allows us to know the absolute position of the plunger. We reset the position count to zero when the sensor detects the reflector.
