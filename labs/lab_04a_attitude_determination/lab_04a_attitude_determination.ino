// ----- ICM 20948 IMU
#include <ICM_20948.h>  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU

#define SERIAL_PORT Serial
#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 1

ICM_20948_I2C myICM;

#include "IMU_pins.h"
#include "./sun_sensor_pins.h"

// ----- SD card -----
#include <SD.h>

#ifdef ARDUINO_TEENSY41
const int chipSelect = BUILTIN_SDCARD;
#else  // Arduino MKR Zero
const int chipSelect = SDCARD_SS_PIN;
#endif

File dataFile;

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

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1)
      ;
  }
  Serial.println("card initialized.");

  dataFile = SD.open("04a_attitude_determination.dat", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    String write_line = ""; 
    write_line += "units:\n" ;
    write_line += "time (ms)\n"; 
    write_line += "gyr (dps)\n"; 
    write_line += "mag (uT)\n"; 
    write_line += "sun detector (count)\n"; 
    write_line += "sun angle (deg)\n";

    dataFile.print(write_line);

    Serial.print(write_line);

    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else { Serial.println("error opening datalog.txt"); 
  }

}  // end function setup


int t;
int t0 = millis();  // set start time right before loop

// write accel data (to SD and/or serial) every `write_interval` ms
int last_wrote = 0;
int write_interval = 100;  // ms


String printFormattedFloat(float val, uint8_t leading, uint8_t decimals) {
  String write_line = "";
  float aval = abs(val);
  if (val < 0) {
    write_line += "-";
    // SERIAL_PORT.print("-");
  } else {
    write_line += "+";
    // SERIAL_PORT.print(" ");
  }
  for (uint8_t indi = 0; indi < leading; indi++) {
    uint32_t tenpow = 0;
    if (indi < (leading - 1)) {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++) {
      tenpow *= 10;
    }
    if (aval < tenpow) {
      write_line += "0";
      // SERIAL_PORT.print("0");
    } else {
      break;
    }
  }
  if (val < 0) {
    write_line += -val;
    write_line += decimals;
    // SERIAL_PORT.print(-val, decimals);
  } else {
    write_line += val;
    write_line += decimals;
    // SERIAL_PORT.print(val, decimals);
  }
  return write_line;
}  //end printformattedfloat()

String printScaledAGMT(ICM_20948_I2C *sensor) {
  String write_line = ", gyrz:";
  write_line += printFormattedFloat(sensor->gyrZ(), 5, 2);
  write_line += ", magx:";
  write_line += printFormattedFloat(sensor->magX(), 5, 2);
  write_line += ", magy:";
  write_line += printFormattedFloat(sensor->magY(), 5, 2);

  return write_line;
}

void loop() {
  t = millis();

  if (t - last_wrote > write_interval) {
    String write_line = "t:";
    write_line += t;

    myICM.getAGMT();                        // The values are only updated when you call 'getAGMT'
    write_line += printScaledAGMT(&myICM);  // This function takes into account the scale settings from when the measurement was made to calculate the values with units

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
    // sun_direction = atan2(-east*1.0, -north) * RAD_TO_DEG + 180; 
    // write_line += ", sun:"; 
    // write_line += sun_direction; 
    
    Serial.println(write_line);

    File dataFile = SD.open("04a_attitude_determination.dat", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(write_line);
      dataFile.close();

    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening attitude.csv");
    }

    last_wrote += write_interval;
  }

}  // end loop()
