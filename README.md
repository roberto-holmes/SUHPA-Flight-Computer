# SUHPA Avionics

Currently divided into BaseCom that receives input from the pilot and sends it to Pod that controls the servos.

BaseCom is currently a stripboard design

Pod v1.0
- Status: Completed
- Issues: 
    * Short between 5v_Servo and GND at the BEC connector
    * Insufficient spacing between buzzer and balance connector
    * Mosfet required on balance connector to allow for full power off

Pod v1.1
- Status: Development
- Changes: Fixed short