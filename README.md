# SUHPA Avionics

Currently divided into CRASH (Controls, Radio, and Avionics for Southampton HPA) that receives input from the pilot and sends it to Pod that controls the servos.

## Software
TODO:
- Store sensor calibration in EEPROM (most calibration is being done on the IMU, but altitude is not being calibrated)
- Varying frequency beep
- Invert controls
- Divide trim steps by 10
- Fix lag when SD card disconnects

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
