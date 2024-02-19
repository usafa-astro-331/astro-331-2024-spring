// IMU sensor connects with I2C via a QWIIC cable

    // Black = GND.
    // Red = 3.3V.
    // Blue = SDA.
    // Yellow = SCL.


// INA219 pin = Arduino pin

// SDA = SDA
// SCL = SCL
// if not using QWIIC connection, see datasheet for power and ground connections



float x_bias = -33.677; 
float x_gain = 0.0609; 
float y_bias = -20.252; 
float y_gain = 0.0687; 

float magx, magy, heading; 