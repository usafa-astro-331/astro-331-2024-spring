// IMU sensor connects with I2C via a QWIIC cable

    // Black = GND.
    // Red = 3.3V.
    // Blue = SDA.
    // Yellow = SCL.


// INA219 pin = Arduino pin

// SDA = SDA
// SCL = SCL
// if not using QWIIC connection, see datasheet for power and ground connections

// gain and bias parameters for magnetometer
float x_bias = 0; 
float x_gain = 1; 
float y_bias = 0; 
float y_gain = 1; 

float magx, magy, heading; 