# WaterPump
Low flow high static head water pump to empty my dehumidifier reservoir and water plants.
This pump uses a 60ml plastic syringe, two one-way valves, a DC gearmotor, and a microcontroller.
![Water Pump](/images/WaterPumpFigure.png)
 
## Motivation
There were a few motivations for this project:
1. I dislike having to go down to the basement to empty the dehumidifier reservoir all summer
2. Plants outside need to be watered regularly
3. I like projects that involve microcontrollers

## Requirements / Design Goals
* high static head (water needs to be pumped from the basement floor to hanging plants over the deck)
* self-priming
* can be run dry without damage
* automatically turn on and off
* IoT capable - can be connected to my home automation network
* low voltage
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
* A microcontroller reads 

### How it pumps
When the plunger is pulled out water is drawn in through one of the one-way valves, while the other blocks back flow. When the plunger is pushed in water is pushed throughone of the one-way valves while the other blocks flow back to the intake.

![Water Flow](/images/WaterFlow.png)

### Motor Control
The Pololu Baby Orangutan B robot controller has two built-in DC motor drivers. We use one of them to drive the DC gearmotor.

We need to know where the plunger is, and for that we use a simple tachometer / odometer consisting of a Pololu QTR-1A reflectance sensor and a piece of aluminum mounted on the gearmotor's extended shaft:
![Tachometer / Odometer](/images/TachometerOdometer.png)
