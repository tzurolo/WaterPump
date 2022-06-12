# WaterPump
High static head / low flow water pump to empty my dehumidifier tank and water my plants.
This pump uses a 60ml plastic syringe, two one-way valves, a DC gearmotor, and a microcontroller.

![Water Pump](/images/WaterPumpFigure.png)
 
## Motivation
There were a few motivations for this project:
1. I dislike having to go down to the basement to empty the dehumidifier tank all summer
2. Plants outside need to be watered regularly
3. I like projects that involve microcontrollers

## Requirements / Design Goals
* high static head (water needs to be pumped from the basement floor up to hanging plants over the deck)
* self-priming
* can be run dry without damage
* automatically turn on and off
* IoT capable - can be connected to my home automation network
* low voltage
* low cost / build mainly out of stuff I have laying around
* relatively quiet

I spent many unhappy hours searching online for an off-the-shelf pump that meets most of these requirements, but didn't find anything suitable. And, as I alluded to above, I like to design and build things like this.

## Design
The principal design elements:
* A 60ml medical syringe and two one-way valves for the water handling itself.
* The plunger of the syringe is pushed and pulled by a threaded coupler on a 1/4-20 threaded rod. The rod is held in place by a ball bearing.
* A DC gearmotor that turns the threaded rod which pushes and pulls the threaded coupler
* Reflectance sensors that are used for
   1. gearmotor tachometer/odometer
   2. syringe plunger home position sensor
* Microcontroller with motor driver

The microcontroller reads a water level sensor in the tank that tells when the tank is full. When full the pump pumps a predetermined amount of water out of the tank (2L in this case)

### How it pumps
When the plunger is pulled out water is drawn in through the intake one-way valve, while the outflow one-way valve blocks back flow from the output. When the plunger is pushed in water is pushed through the outflow one-way valves while the intake valve blocks flow back to the intake.

![Water Flow](/images/WaterFlow.png)

This is of course very slow, but it has no trouble keeping up with the dehumidifier, which produces water a drip at a time.

### Motor Control
The Pololu Baby Orangutan B robot controller has two built-in DC motor drivers. We use one of them to drive the DC gearmotor.

We need to know where the plunger is. To get relative position we use a simple tachometer / odometer consisting of a Pololu QTR-1A reflectance sensor and a piece of aluminum mounted on the gearmotor's extended shaft:

![Tachometer / Odometer](/images/TachometerOdometer.png)

This produces two pulses for every rotation of the motor shaft (input to the gearhead). Counting these pulses tells us how many times the threaded rod is rotated. The threaded rod moves the syringe plunger 1/20" for each rotation of the output shaft (output of the gearhead). We don't get direction with this approach, so we always bring the motor to a full stop before changing direction.

For absolute position there is also a reflectance sensor that tells when the plunger has reached the edge of the "home position". We reset the position count to zero when the sensor detects the reflector, and count motor shaft pulses from there.

## Main Parts
* Hsiang Neng HN-GH7.2-2414T gearmotor
* Pololu Baby Orangutan B robot controller
* (2) Pololu QTR-1A reflectance sensor
* DEPEPE 60ml Plastic Syringe
* (2) Check Valve, 1/4 Inch 6mm PVDF Wear-Resistant One-Way Check Valve for Fuel Gas Liquid Air

## Tank sensor
![Tank sensor](/images/TankSensor.png)
This sensor is mounted in the dehumidifier tank and detects when the tank becomes full.
The sensor consists of a Pololu QTR-1A reflectance sensor and a ping pong ball in an acrylic cage.
When the ball floats up to the reflectance sensor a signal goes to the microcontroller telling it the tank is full.

## Control program
The code running on the microcontroller monitors the input from the tank-full reflectance sensor. When the tank is full the program
runs the gearmotor to pull the plunger out of the syringe until it reaches the fully-out position. Then it runs the gearmotor in
the opposite direction to push the water out. This cycle is repeated until the preset volume of water has been pumped.
Odometer values for the fully-out and fully-in plunger positions are stored in EEPROM, as well as the motor PWM speed and how much water to pump when the tank is full.
When it completes one syringe cycle (drawing in and then pushing out) it emits a message (on TX of the UART) reporting how many milliliters of water it pumped.

## Results
The pump was put into operation mid-July 2021 and has been emptying the dedumidifier tank ever since.
One thing that falls short of meeting all the requirements is that it is not as quiet as I would like. It's not exactly loud, at least not louder than the dehumidifier fan, but I wanted it to be almost inaudible.
The noise comes from the DC gearmotor. I think using a brushless DC gearmotor might make it run quieter.

