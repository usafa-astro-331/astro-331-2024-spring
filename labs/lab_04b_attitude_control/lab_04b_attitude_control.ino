// #define USEPID

#define SERIAL_PORT Serial
#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined

// ----- ICM 20948 IMU
  #include <ICM_20948.h>  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
  ICM_20948_I2C myICM;
  // The value of the last bit of the I2C address.
  // On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
  #define AD0_VAL 1
  #include "IMU_setup.h"


// ----- TB9051FTG Motor Carrier
  #include <TB9051FTGMotorCarrier.h>

// ------ quadrature encoder for motor position
  // Teensy is not compatible with QuadratureEncoder/Hardware Interrupt
  //   but comes with its own Encoder library
  #ifdef ARDUINO_TEENSY41
  #include <Encoder.h>
  #else  // Arduino MKR Zero
  #include <QuadratureEncoder.h>
  // NOTE: QuadratureEncoder requires <EnableInterrupt.h>
  #endif
  #include "motor_controller_pins.h"
  // Instantiate TB9051FTGMotorCarrier
    static TB9051FTGMotorCarrier driver{ pwm1Pin, pwm2Pin };
  // set up variable for pulse width modulation of motor
    static float throttlePWM{ 0.0f };

unsigned long lastMilli = 0;
long currentEncoderCount = 0;
long lastEncoderCount = 0;
float speed_rpm = 0.0;
long timeElapsed = 0;

float speed_pwm;

// ----- SD card -----
  #include <SD.h>
  #ifdef ARDUINO_TEENSY41
    const int chipSelect = BUILTIN_SDCARD;
  #else  // Arduino MKR Zero
    const int chipSelect = SDCARD_SS_PIN;
  #endif
  File dataFile;


// ----- time variables ----- for open loop motor speed control
// int print_time = 0;
// int print_delay = 500;
int current_time = 0;
int elapsed = 0;

// PID 
#include "src/PID/attitude_PID.h"
double kp=-50, ki=-5, kd=0; 
double HeadingSetpoint, HeadingInput, dHeadingInput, Output; 

PID myPID(&HeadingInput, &dHeadingInput, &Output, &HeadingSetpoint, kp, ki, kd, P_ON_E, DIRECT); //P_ON_M specifies that Proportional on Measurement be used
//                                                             //P_ON_E (Proportional on Error) is the default behavior

void setup() {
  SERIAL_PORT.begin(115200);
  //  while (!SERIAL_PORT)
  //  {
  //  };

  // ----- ICM 20948 IMU
  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  bool initialized = false;
  while (!initialized) {

    myICM.begin(WIRE_PORT, AD0_VAL);


    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok) {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    } else {
      initialized = true;
    }
  }

  // ----- TB9051FTG Motor Carrier
  driver.enable();

  // LED for state indication
  pinMode(A0, OUTPUT);

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1)
      ;
  }
  Serial.println("card initialized.");

  dataFile = SD.open("04b_attitude_control.dat", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    String write_line = "";
    write_line += "units:\n";
    write_line += "time (ms)\n";
    write_line += "heading (rad)\n";
    write_line += "gyrZ (rad/s)\n";
    write_line += "wheel speed (RPM)\n";

    dataFile.print(write_line);

    Serial.print(write_line);

    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

// initialize PID
myICM.getAGMT();
magx =  (myICM.magX() - x_bias) * x_gain; 
magy =  (myICM.magY() - y_bias) * y_gain; 
HeadingInput = atan2(magy, -magx) +PI; 
dHeadingInput = -myICM.gyrZ() * DEG_TO_RAD; // neg b/c gyrz = -magz on ICM_20948
HeadingSetpoint = HALF_PI; 
myPID.SetMode(AUTOMATIC); 
myPID.SetOutputLimits(0.1, 1.0);

// spin reaction wheel to 500 RPM, wait 5 sec
throttlePWM = 0.5;
while (millis()<5000){}

}  // end function setup

int speed;

int PID_interval = 10;
int last_PID_time;
int t;
int t0 = millis();  // set start time right before loop

// write accel data (to SD and/or serial) every `write_interval` ms
int last_wrote = 0;
int write_interval = 300;  // ms

void loop() {
  t = millis();

    myICM.getAGMT();  // The values are only updated when you call 'getAGMT'

    magx =  (myICM.magX() - x_bias) * x_gain; 
    magy =  (myICM.magY() - y_bias) * y_gain; 
    HeadingInput = atan2(magy, -magx) +PI; 
    dHeadingInput = -myICM.gyrZ() * DEG_TO_RAD; // neg b/c gyrz = -magz on ICM_20948

    #ifdef USEPID
      myPID.Compute(); 
      driver.setOutput(Output);
      if (t > 20e3) HeadingSetpoint = 0; 
    #else // closed loop speed control
      speed_pwm = set_speed(); 
    #endif

  if (t - last_wrote >= write_interval) {
    String write_line = "t:";
    write_line += t;
    write_line += ", head:";
    write_line += HeadingInput;
    write_line += ", gyr:";
    write_line += dHeadingInput;

// Read current encoder count
// different libraries for Teensy/MKR Zero have different syntax
#ifdef ARDUINO_TEENSY41
    currentEncoderCount = myEnc.read();
#else  // Arduino MKR Zero
    currentEncoderCount = Encoder.getEncoderCount();
#endif

    // print wheel speed
    // Determine how much time has elapsed since last measurement
    timeElapsed = millis() - lastMilli;
    // Calculate speed in rpm
    // encoder is 64 counts per rev
    // motor is 10:1 geared
    // counts/ms * 1 rev/64 counts * 1000 ms/1 sec * 60 s/1 min * 1 rot/10 gears = rev/min
    write_line += ", ω:";
    write_line += float(currentEncoderCount - lastEncoderCount) / timeElapsed / 64 * 1000 * 60 / 10;
    // reset variables to most recent value
    lastMilli = millis();
    lastEncoderCount = currentEncoderCount;
    // Print to serial monitor
    // Serial.print("Time (ms) = ");
    //     Serial.print(lastMilli);
    //     Serial.print(", Speed (RPM) = ");
    
    Serial.println(write_line);

    File dataFile = SD.open("04b_attitude_control.dat", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(write_line);
      dataFile.close();

    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening file on SD card");
    }

    last_wrote += write_interval;
  }

}  // end loop()





float set_speed() {
  t = millis() - t0;

  if (t - t0 < 10e3) {  // hold still at half speed (10 sec)
    throttlePWM = 0.5;
  }

  else if (t - t0 < 15e3) {  // hold still at half speed (5 sec)
    throttlePWM = 0.5;
    digitalWrite(A0, HIGH);
  }

  else if (t - t0 < 17.5e3) {  // ramp up (2.5 sec)
    elapsed = t - 15e3;
    throttlePWM = 0.5 + 0.1 * elapsed / 2.5e3;
    digitalWrite(A0, LOW);
  }

  else if (t - t0 < 20e3) {  // ramp back down to half speed (2.5 sec)
    elapsed = t - 17.5e3;
    throttlePWM = 0.6 - 0.1 * elapsed / 2.5e3;
  }

  else if (t - t0 < 25e3) {  // hold new position (5 sec)
    throttlePWM = 0.5;
    digitalWrite(A0, HIGH);
  }

  else if (t - t0 < 27.5e3) {  // ramp down (2.5 sec)
    elapsed = t - 25e3;
    throttlePWM = 0.5 - .1 * elapsed / 2.5e3;
    digitalWrite(A0, LOW);
  } else if (t - t0 < 30e3) {  // ramp back up to half speed  (5 sec)
    elapsed = t - 27.5e3;
    throttlePWM = .4 + .1 * elapsed / 2.5e3;
  }

  else if (t - t0 < 35e3) {  // hold new position (5 sec)
    throttlePWM = 0.5;
    digitalWrite(A0, HIGH);
  }

  else if (t - t0 < 40e3) {  // turn off wheel
    throttlePWM = 0;
    digitalWrite(A0, LOW);
    driver.setOutput(0);

  }

  else {
    while (1) {};
  }

  driver.setOutput(throttlePWM);

  return throttlePWM;
}  // end set_speed()

