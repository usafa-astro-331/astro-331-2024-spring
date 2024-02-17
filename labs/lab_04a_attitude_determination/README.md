# Lab 4: attitude

In this lab you will build and test part of FlatSAT's attitude system including an attitude sensor and reaction wheel. (FlatSAT's design calls for 3 reaction wheels but you will only test 1.) You will measure system performance to ensure that FlatSAT can meet it's orbital torque and momentum requirements. 



Prelab instructions: 
https://www.overleaf.com/read/prdzpknpdtgf



## equipment

- 1 laptop
- micro USB cable 
- string



## hardware

- FlatSAT
  - components from previous labs
  - SparkFun 9 DOF IMU
  - reaction wheel assembly
  - 3 cell Li-ion battery
  - BEC
  - wago wire connectors



## documentation

- motor datasheet (Pololu 37D metal gearmotor)
- motor driver datasheet (Pololu TB905FTG motor driver carrier)
- IMU datasheet (SparkFun 9DoF IMU)



## software

- Arduino IDE

- Arduino SAMD drivers (already installed)
- ensure these Arduino libraries are installed

  - SparkFun 9DoF IMU

- `lab_04a_attitude_determination.ino`



## setup

For today’s lab, FlatSAT will be powered by a 12 V lithium ion battery. The (future) motor driver and motor are powered directly from the battery’s 12 V supply. The Arduino itself is powered via a 12 V $\rightarrow$ 5 V BEC (battery eliminator circuit). Everything else is powered by Arduino’s Vcc pin (3.3 V). 



Copy the setup below, but **do not place the 3rd (middle) cell into the battery holder yet**. 



**Note:** the connection between the Arduino and the 5V rail has moved from previous labs. It’s now connected to Vin. 

![attitude_bb](../../fritzing_diagrams/04_attitude_bb.svg)

- Add the red 9 DOF IMU (red square) to your FlatSAT as in the diagram.
  - use a QWIIC cable to connect via the current sensor
- Add 4 photocell voltage dividers. The photocell is R1. 
  - ![](../../minilabs/06_metrology/sources/Resistive_divider2.svg)
  - Vin: 3 V
  - Ground: ground
  - Vout: see `sun_sensor_pins.h`
    - use pinouts defined for Arduino MKR Zero
    - align +x (px), +y (py), -x (nx), -y (ny) with the IMU’s magnetometer coordinate frame
    



## sensor checkout

Ensure you are collecting meaningful data from FlatSAT’s sensors. 



Install SD card. Connect FlatSAT via USB and upload `lab_04a_attitude_determination.ino`. Open the serial plotter. 

Rotate FlatSat as you shine a light at it. 

- x- and y-magnetometer data should wander up and down with orientation
- gyroscope data should spike as you rotate
- sun sensors should each rise when pointed toward light and fall when covered



## integrate sun sensor

Now you will modify the code to obtain a rough sun direction from the photocells. 

Create an equation/algorithm to find the magnitude of sun’s brightness in the north/anti-north direction and the east/anti-east direction. 

Uncomment the bottom lines of this code segment and add your algorithm.

Comment out the “output raw sun sensor data” section. (Highlight all, `ctrl`-`/`.)

Upload your modified code and test with a flashlight. 

``` c++
    // read sun sensors
    sunpx_reading = analogRead(sunpx_pin);
    sunpy_reading = analogRead(sunpy_pin);
    sunnx_reading = analogRead(sunnx_pin);
    sunny_reading = analogRead(sunny_pin);    
    
    // output raw sun sensor data
    write_line += ", sunpx:"; 
    write_line += sunpx_reading; 
    write_line += ", sunpy:"; 
    write_line += sunpy_reading; 
    write_line += ", sunnx:"; 
    write_line += sunnx_reading; 
    write_line += ", sunny:"; 
    write_line += sunny_reading; 

    // // find sun direction
    // north =  ; // you fill in here--remember to end line with ;
    // east =  ; // you fill in here--remember to end line with ;
    // sun_direction = atan2(east*1.0, -north) * RAD_TO_DEG + 180; 
    // write_line += ", sun:"; 
    // write_line += sun_direction; 
```




## attitude determination

Using a string loop, hang the motor platform from the hook above your workstation. Place FlatSAT and the battery holder on the platform and secure with tape. Insert all battery cells and connect the BEC’s 5V output to FlatSAT’s Vin pin. 

Gently spin FlatSAT several times. Ensure you get at least 3 complete revolutions uninterrupted. Also spin it it back and forth several times. 

Spin FlatSAT quickly (at least 1 RPM). 



## Lab station cleanup

- Transfer saved data to your group's storage location
- Disconnect devices from FlatSAT and all computers
- Replace all items at your lab station the way you found them. 
- Have your instructor check off your lab station before you depart.



## Post-lab data analysis

For the next lab the magnetometer data will be an input to the spacecraft’s attitude system. To use it, you must calibrate the magnetometer data. 

Use the magnetometer data you gathered to find the gain and bias of your magnetometers. 
