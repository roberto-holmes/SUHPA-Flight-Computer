# SUHPA Avionics

Currently divided into CRASH (Controls, Radio, and Avionics for Southampton HPA) that receives input from the pilot and sends it to Pod that controls the servos.

## Software
TODO:
- Read and store sensor data
- Store sensor calibration in EEPROM
- Display status messages
- Varying frequency beep
- Screensaver
- Invert controls
- Divide trim steps by 10

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
   * Balance connector keeps board powered (should be replaced with just measuring the voltage of V_bat)
