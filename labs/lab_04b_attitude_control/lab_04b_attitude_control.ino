#define USEPID

#define SERIAL_PORT Serial
#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined

// ----- ICM 20948 IMU
  #include <ICM_20948.h>  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
  ICM_20948_I2C myICM;
  // The value of the last bit of the I2C address.
  // On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
  #define AD0_VAL 1
  #include "IMU_setup.h"


 // #include "set_speed.ino"
 // arduino IDE sees all *.ino files in the directory--it's automatically included


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
  #include <SPI.h>
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
double kattp=50.0, katti=5.0, kattd=0.0; 
double HeadingSetpoint, HeadingInput, dHeadingInput, Output; 
attitudePID myattitudePID(&HeadingInput, &dHeadingInput, &Output, &HeadingSetpoint, kattp, katti, kattd, P_ON_M, DIRECT); //P_ON_M specifies that Proportional on Measurement be used
//                                                             //P_ON_E (Proportional on Error) is the default behavior


#include <PID_v1.h>
double kp=50.0, ki=5.0, kd=0.0; 
double  SpeedInput, PWMOutput; 
double SpeedSetpoint = 0.0; 
PID myspeedPID(&SpeedInput, &PWMOutput, &SpeedSetpoint, kp, ki, kd, P_ON_M, DIRECT); //P_ON_M specifies that Proportional on Measurement be used


void setup() {

  // spin reaction wheel to 500 RPM, wait 5 sec
throttlePWM = 0.5;
driver.setOutput(throttlePWM);
delay(5000); 


  Serial.begin(115200);
  Serial1.begin(9600); 
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

  Serial1.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial1.println("Card failed, or not present");
    // don't do anything more:
    while (1)
      ;
  }
  Serial1.println("card initialized.");

  dataFile = SD.open("04b_att.dat", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    String write_line = "";
    write_line += "units:\n";
    write_line += "time (ms)\n";
    write_line += "magx (uT)\n";
    write_line += "magy (uT)\n";
    write_line += "heading (rad)\n";
    write_line += "gyrZ (rad/s)\n";
    write_line += "commanded wheel speed (RPM)\n";
    write_line += "measured wheel speed (RPM)\n";

    dataFile.print(write_line);

    Serial1.print(write_line);

    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial1.println("error opening datalog.txt");
  }



// initialize PID
myICM.getAGMT();
magx =  (myICM.magX() - x_bias) /x_range; 
magy =  (myICM.magY() - y_bias) /y_range; 
HeadingInput = atan2(magy, -magx) +PI; 
dHeadingInput = -myICM.gyrZ() * DEG_TO_RAD; // neg b/c gyrz = -magz on ICM_20948
HeadingSetpoint = HALF_PI; 
Output = 0.5; 
myattitudePID.SetOutputLimits(100, 1000);
myattitudePID.SetMode(AUTOMATIC); 

myspeedPID.SetOutputLimits(-0.01, 0.01);
myspeedPID.SetMode(AUTOMATIC); 

}  // end function setup

int speed;

int PID_interval = 10;
int last_PID_time;
int t;
int t0 = millis();  // set start time right before loop

// write accel data (to SD and/or serial) every `write_interval` ms
int last_wrote = 0;
int write_interval = 300;  // ms

#define NSAMPLES 30
double gyr[NSAMPLES]; 
int gyrindex = 0; 
double gyrvalue = 0.0; 
double gyrsum = 0.0; 

void loop() {
  t = millis();

    myICM.getAGMT();  // The values are only updated when you call 'getAGMT'

    magx =  (myICM.magX() - x_bias) /x_range; 
    magy =  (myICM.magY() - y_bias) /y_range; 
    HeadingInput = atan2(magy, -magx) +PI; 
    dHeadingInput = -myICM.gyrZ() * DEG_TO_RAD; // neg b/c gyrz = -magz on ICM_20948
    SpeedInput = -myICM.gyrZ() * DEG_TO_RAD; // neg b/c gyrz = -magz on ICM_20948

    gyrsum = gyrsum - gyr[gyrindex]; 
    gyrvalue = -myICM.gyrZ(); 
    gyr[gyrindex] = gyrvalue; 
    gyrsum += gyrvalue; 

    gyrindex = (gyrindex+1) % NSAMPLES; 

    SpeedInput = gyrsum / NSAMPLES; 

    kp = analogRead(A15)/100.0; 
    ki = analogRead(A16)/500.0; 
    kd = analogRead(A17)/10.0; 
    myattitudePID.SetTunings(kattp, katti, kattd);

    HeadingSetpoint = analogRead(A14)/1024.* TWO_PI; 



    #ifdef USEPID
      myattitudePID.Compute(); 

      myspeedPID.Compute();
      speed_pwm += PWMOutput; 
      if (speed_pwm > 1){
        speed_pwm = 1;
      }
      else if (speed_pwm < 0.1){
        speed_pwm = 0.1; 
      }
      driver.setOutput(speed_pwm);
      // if (t > 20e3) HeadingSetpoint = 0; 
    #else // closed loop speed control
      speed_pwm = set_speed(); 
    #endif

  if (t - last_wrote >= write_interval) {
    
    
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

    String write_line = "";
    // String write_line = "t:";
    write_line += t;
    write_line += "\t";
    // write_line += ", magx:";
    write_line += magx;
    write_line += "\t";
    // write_line += ", magy:";
    write_line += magy;
    write_line += "\t";
    // write_line += ", head:";
    write_line += HeadingInput;
    write_line += "\t";
    // write_line += ", gyr:";
    write_line += SpeedInput;
    write_line += "\t";
    // write_line += ", ω_cmd:";
    write_line += speed_pwm * 1000; // speed in RPM
    write_line += "\t";
    // write_line += ", ω_meas:";
    write_line += float(currentEncoderCount - lastEncoderCount) / timeElapsed / 64 * 1000 * 60 / 10;
    write_line += "\t";
    write_line += kp;
    write_line += "\t";
    write_line += ki;
    write_line += "\t";
    write_line += kd;
    write_line += "\t";
    write_line += HeadingSetpoint;
    write_line += "\t";
    
    // Print to serial monitor and file
    Serial1.println(write_line);

    File dataFile = SD.open("04b_att.dat", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(write_line);
      dataFile.close();

    }
    // if the file isn't open, pop up an error:
    else {
      Serial1.println("error opening file on SD card");
    }


    // reset variables to most recent value
    lastMilli = millis();
    lastEncoderCount = currentEncoderCount;
    

    last_wrote += write_interval;
  }

}  // end loop()



