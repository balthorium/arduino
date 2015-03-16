#include <Wire.h>

// constants for proximity sensor
#define VCNL4000_ADDRESS 0x13  // 0x26 write, 0x27 read
#define COMMAND_0 0x80
#define IR_CURRENT 0x83
#define PROXIMITY_RESULT_MSB 0x87
#define PROXIMITY_RESULT_LSB 0x88
#define PROXIMITY_FREQ 0x89
#define PROXIMITY_MOD 0x8A

// constants for motor pins
const int PWM_A   = 3;
const int DIR_A   = 12;
const int BRAKE_A = 9;
const int PWM_B   = 11;
const int DIR_B   = 13;
const int BRAKE_B = 8;

// constant for LED pin
const int GREEN_LED = 2;

// constant for proximity error
const int MARGIN_OF_ERROR = 20;

/**
 * This function is called once, at the beginning of the application and
 * after each "reset" of the arduino.  In this function, we do things like
 * configure the proximity sensor, initialize the console serial port, and
 * configure each of the pins.
 */
void setup() 
{
  delay(2000);
  
  // configure proximity sensor
  Wire.begin();
  writeByte(IR_CURRENT, 20);
  writeByte(PROXIMITY_FREQ, 2);
  writeByte(PROXIMITY_MOD, 0x81);
  
  //initialize serial port to console
  Serial.begin(9600);

  // initialize indicator LED
  pinMode(GREEN_LED, OUTPUT);
  
  // initialize motor pins
  pinMode(DIR_A, OUTPUT);
  pinMode(BRAKE_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  pinMode(BRAKE_B, OUTPUT);  
}


/*******************************************************
 * Do calibration of proximity sensor threshold.
 *******************************************************/
 unsigned int calibrate() {
  
  // calibrate proximity sensing threshold
  Serial.println("Proximity calibation started...");
  unsigned int threshold = 0;
  for (int i = 0; i < 10; ++i) {
      unsigned int reading = readProximity();
      Serial.print("Calibration reading is: ");
      Serial.println(reading, DEC);
      threshold += reading;
      delay(500);
  }
  threshold = threshold / 10;
  Serial.println("Proximity calibation completed, threshold is: ");
  Serial.println(threshold, DEC);

  return threshold;
}


/*******************************************************
 * Move forward.
 *******************************************************/

void forwardMarch() {
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, 255);
  analogWrite(PWM_B, 255);
}


/*******************************************************
 * Continue with previous instructions until an
 * obstacle is detected.
 *******************************************************/

void continueUntilObstacle(unsigned int threshold) {
  unsigned int proximity = readProximity();
  Serial.println(proximity, DEC);
  while (proximity < threshold) {
    Serial.println(proximity, DEC);
    proximity = readProximity();
  }
  Serial.print("Exiting continueUntilObstacle at proximity: ");
  Serial.println(proximity);
}


/*******************************************************
 * Blink green LED.
 *******************************************************/

void blinkLED() {
  Serial.println("Main loop will start in 2 seconds.");
  for (int i = 0; i < 20; ++i) {
      digitalWrite(GREEN_LED, (i%2 ? HIGH : LOW));
      delay(250);
  }
}


/*******************************************************
 * Stop moving.
 *******************************************************/

void stopMoving() {
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);
  digitalWrite(BRAKE_A, HIGH);
  digitalWrite(BRAKE_B, HIGH);
}


/*******************************************************
 * Turn left.
 *******************************************************/

void leftMarch() {
  analogWrite(PWM_A, 0);
  digitalWrite(BRAKE_A, HIGH);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_B, 255);  
}


/*******************************************************
 * Compute average.
 *******************************************************/

double computeAverate(unsigned int lastTenPRs[], int length) {
  int sum = 0;
  int i = 0;
  while (i < length) {
    sum = sum + lastTen[i];
    i++;
  }
  return sum / length;
}

/*******************************************************
 * Compute standard deviation.
 *******************************************************/

double computeStandardDeviation(unsigned int lastTenPRs[], int length, double average) {
  int numerator = 0;
  int i = 0;
  while (i < length) {
    numerator = numerator + pow(lastTen[i] - average, 2);
  }
  double sigma = sqrt(numerator / length);
}

/*******************************************************
 * Continue until least obstructed.
 *******************************************************/
void continueUntilLeastObstructed() {
  unsigned int lastTenPRs[10];
  
  // just read first 10 proximity readings
  int i = 0;
  while (i < 10) {
    lastTen[i] = readProximity();
    i++;
    delay(10);
  } 
  
  double average = computeAverage(lastTen, 10);
  double sigma = computeStandardDeviation(lastTen, 10, average);
}


/*******************************************************
 * Move backwards.
 *******************************************************/
 
 void backwardsMarch() {
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 255);
  analogWrite(PWM_B, 255);
}
 
/******************************************************************
 * This is the main loop function.  This will be called over and
 * over and over again for as long as the program is running.
 ******************************************************************/
void loop() {  
  
  // turn on green LED to indicate main loop has started
  digitalWrite(GREEN_LED, HIGH);

  // do initial calibration
  unsigned int threshold = calibrate();
  
   // five second delay between calibration and running  
  blinkLED();

  // turn on green LED to indicate run has started
  digitalWrite(GREEN_LED, HIGH);

  // go forward
  forwardMarch();  

  // keep moving until we see something in front of us
  continueUntilObstacle(threshold);
  
  // stop moving
  stopMoving();
 
  backwardsMarch();
  delay(500); 

  leftMarch();

  continueUntilLeastObstructed();
  
  stopMoving();

  Serial.println("Done.");
 
  digitalWrite(GREEN_LED, LOW);

  while(true) { }
}


//////////////////////////////////////////////////////////////

unsigned int readProximity()
{
  unsigned int data;
  byte originalValue;
  byte newValue;
  byte updateValue;
  
  // tell proximity sensor to start reading
  originalValue = readByte(COMMAND_0);
  newValue = originalValue | 0x08;
  writeByte(COMMAND_0, newValue);

  delay(10);

  // wait until proximity sensor has a reading  
  updateValue = readByte(COMMAND_0);
  while( !(updateValue & 0x20)) {
    updateValue = readByte(COMMAND_0);
  }
  
  // assign to data the value read from proximity sensor
  data = readByte(PROXIMITY_RESULT_MSB) << 8;
  data |= readByte(PROXIMITY_RESULT_LSB);
  
  return data;
}


void writeByte(byte address, byte data)
{
  Wire.beginTransmission(VCNL4000_ADDRESS);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

byte readByte(byte address)
{
  byte data;
  
  Wire.beginTransmission(VCNL4000_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(VCNL4000_ADDRESS, 1);
  while(!Wire.available()) {
    // do nothing
  }
  data = Wire.read();
  return data;
}
 
