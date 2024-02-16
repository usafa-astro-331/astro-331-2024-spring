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
- Add 4 photocell voltage dividers
  - ![](../../minilabs/06_metrology/sources/Resistive_divider2.svg)
  - Vin: 3 V
  - Ground: ground
  - Vout: see `sun_sensor_pins.h`
    - use pinouts defined for Arduino MKR Zero

  - point 4 photocells in 4 different directions (in the x-y plane)




## attitude determination

Now you will test FlatSAT's attitude sensor as the spacecraft attitude changes. 

Install an SD card into FlatSAT. Open and upload `lab_04a_attitude_determination.ino`. Open the serial plotter. Cover the photocells one at a time and observe the sun sensors change. Rotate FlatSAT about the z-axis and observe the z-axis gyroscope data and the x- and y-axis magnetometer data. Ensure they all change. Disconnect the USB cable from FlatSAT. 

Using a string loop, hang the motor platform from the hook above your workstation. Place FlatSAT and the battery holder on the platform and secure with tape. Insert all battery cells and connect the 5V output to FlatSAT’s Vin pin. 

Darken the room and shine a flashlight at FlatSAT from any point in the x-y plane. Gently spin FlatSAT several times. Rotate it back and forth and ensure you get at least 3 complete revolutions uninterrupted. 

## calibration

Now you will modify the code to obtain a rough sun direction from the photocells. 



``` c++
// read sun sensors
    sunpx_reading = analogRead(sunpx_pin);
    sunpy_reading = analogRead(sunpy_pin);
    sunnx_reading = analogRead(sunnx_pin);
    sunny_reading = analogRead(sunny_pin);    
    
// find direction 


    write_line += ", sunpx:"; 
    write_line += sunpx_reading; 
    write_line += ", sunpy:"; 
    write_line += sunpy_reading; 
    write_line += ", sunnx:"; 
    write_line += sunnx_reading; 
    write_line += ", sunny:"; 
    write_line += sunny_reading; 
```



## Lab station cleanup

- Transfer saved data to your group's storage location
- Disconnect devices from FlatSAT and all computers
- Remove the ESD wrist straps and replace them in the bag at your lab station.
- Replace all items at your lab station the way you found them. 
- Close Arduino IDE, and log out of the laptop.
- Have your instructor check off your lab station before you depart.

## Post-lab data analysis

Use your measured mass properties and the provided max speed/acceleration data to determine reaction wheel torque. 

Use your recorded attitude data to determine the MOI of the entire FlatSAT assembly. Do this with the wheel speed data and IMU data immediately before and after the wheel turned off. 

Include graphs of IMU data (magnetometer AND rate gyro) in your final report. 

