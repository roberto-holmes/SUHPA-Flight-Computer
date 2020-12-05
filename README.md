# SUHPA Avionics

Currently divided into CRASH (Controls, Radio, and Avionics for Southampton HPA) that receives input from the pilot and sends it to Pod that controls the servos.

## Software
TODO:
- Read and store sensor data
- Store trim values and sensor calibration in EEPROM
- Monitor radio quality
- Display battery level
- Display status messages
- Asynchronous beep
- Varying frequency beep

## Hardware

### CRASH 

v1.0
- Status: Completed
- Issues:
   * Stress relief holes for battery cables are too small
   * Middle support holes are not required
   * Circle cutout is not quite aligned
   * Square cutout could have more clearance on the left

### Pod
v1.0
- Status: Completed
- Issues: 
   * Short between 5v_Servo and GND at the BEC connector
   * Insufficient spacing between buzzer and balance connector
   * Balance connector requires removal or mosfet to allow for full power off
