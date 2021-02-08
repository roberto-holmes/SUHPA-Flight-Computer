# SUHPA Avionics

Currently divided into CRASH (Controls, Radio, and Avionics for Southampton HPA) that receives input from the pilot and sends it to Pod that controls the servos.

## Software
TODO:
- Varying frequency beep as trim value changes
- Accelerating trim input
- Add average update rate to display

Known issues:
- SD card doesn't reinitialize if it disconects as this would cause the device to only transmit packets every couple of seconds
- Packet loss is not representative of the number of packets being received by Pod as the ACK packages being sent back are the ones getting lost. Pod has to send them at a lower power for some reason causing this issue.

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
