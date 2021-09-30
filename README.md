# WaterPump
Low flow high static head water pump to empty my dehumidifier reservoir and water plants
This pump uses a 60ml plastic syringe, two one-way valves, a 12V gearmotor, and a microcontroller

## Motivation
There were two motivations for this project:
1. I dislike having to go down to the basement to empty the dehumidifier reservoir all summer
2. plants outside need to be watered regularly

I spent many unhappy hours searching online for an off-the-shelf pump to do this.

## Requirements / Design Goals
* high static head (water needs to be pumped from the basement floor to hanging plants over the deck)
* self-priming
* can be run dry without damage
* automatically turn on and off
* IoT capable
* low voltage
* relatively quiet

## Design
* A 60ml medical syringe and two one-way valves for the water handling itself.
* The plunger of the syringe is pushed and pulled by a 1/4-20 threaded rod mounted on a ball bearing.
* A DC gearmotor turns the threaded rod
