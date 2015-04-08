#include <Wire.h>
#include <Time.h>  

// constants for proximity sensor
#define VCNL4000_ADDRESS 0x13  // 0x26 write, 0x27 read
#define COMMAND_0 0x80
#define IR_CURRENT 0x83
#define PROXIMITY_RESULT_MSB 0x87
#define PROXIMITY_RESULT_LSB 0x88
#define PROXIMITY_FREQ 0x89
#define PROXIMITY_MOD 0x8A
#define IRinputAnalogPin 0

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
 * Do a second calibration of proximity sensor threshold
 * for turning.
 *******************************************************/
unsigned int calibrateTwo() {

  // calibrate proximity sensing threshold
  Serial.println("Second proximity calibation started...");
  unsigned int thresholdTwo = 0;
  for (int i = 0; i < 10; ++i) {
    unsigned int reading = readProximity();
    Serial.print("Second calibration reading is: ");
    Serial.println(reading, DEC);
    thresholdTwo += reading;
    delay(500);
  }
  thresholdTwo = thresholdTwo / 10;
  Serial.println("Second Proximity calibation completed, threshold is: ");
  Serial.println(thresholdTwo, DEC);

  return thresholdTwo;
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

boolean continueUntilObstacle(unsigned int threshold, unsigned int maxTimeInSeconds) {
  boolean retval = true;
  unsigned int startTime = now();
  unsigned int proximity = readProximity();
  Serial.println(proximity, DEC);
  while (proximity < threshold) {
    Serial.println(proximity, DEC);
    proximity = readProximity();
    if ((maxTimeInSeconds > 0) && (maxTimeInSeconds < now() - startTime)) {
      retval = false;
      break;
    }
  }
  Serial.print("Exiting continueUntilObstacle at proximity: ");
  Serial.println(proximity);
  return retval;
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

void stopMarch() {
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
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, 255);  
  analogWrite(PWM_B, 255);  
}

/*******************************************************
 * Turn Right.
 *******************************************************/

void rightMarch() {
  analogWrite(PWM_A, 0);
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 255);  
  analogWrite(PWM_B, 255);  
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


/*******************************************************
 * Move backwards until second threshold is hit.
 *******************************************************/

void backwardsMarchUntilSecondThreshold(unsigned int thresholdTwo) {
  backwardsMarch();
  unsigned int proximity = readProximity();
  while (proximity > thresholdTwo) {
    delay(10);
    proximity = readProximity();
  }
  stopMarch();
}


/*********************************************************
 * Finding direction of Sun
 *********************************************************/
void directionOfSun(int counter) {
  int turnTime = 200;
  int sampleTime = 200;
  delay(sampleTime);
  int r1 = analogRead(IRinputAnalogPin);
  if (counter % 2 == 0) {
    leftMarch();
  }
  else {
    rightMarch();
  }
  delay(turnTime);
  stopMarch();
  delay(sampleTime);
  int r2 = analogRead(IRinputAnalogPin);
  if (counter % 2 == 0) {
    leftMarch();
  }
  else {
    rightMarch();
  }
  delay(turnTime);
  stopMarch();
  int r3 = analogRead(IRinputAnalogPin);
  while (r3 >= r1 || r3 >= r2){
    if (counter % 2 == 0) {
      leftMarch();
    }
    else {
      rightMarch();
    } 
    delay(turnTime);
    stopMarch();
    r1 = r2;
    r2 = r3;
    delay(sampleTime);
    r3 = analogRead(IRinputAnalogPin);
  }
  if (counter % 2 == 0) {
    rightMarch();
  }
  else {
    leftMarch();
  }
  delay(turnTime * 2);
  stopMarch();
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

  // move backwards
  backwardsMarch();
  delay(1000); 
  stopMarch();

  // do second calibration
  unsigned int thresholdTwo = calibrateTwo();

  // five second delay between calibration and running  
  blinkLED();

  // turn on green LED to indicate run has started
  digitalWrite(GREEN_LED, HIGH);

  int counter = 0;
  while(true) {

    //Look for sun
    directionOfSun(counter);

    // go forward
    forwardMarch();  

    // keep moving until we see something in front of us
    while(continueUntilObstacle(threshold, 2) == false){
      stopMarch();
      if (analogRead(IRinputAnalogPin) >= 22) {
        while(true);
      }
      directionOfSun(++counter);
      forwardMarch();
    }

    // stop moving
    stopMarch();

    // move backwards until second threshold is reached
    backwardsMarchUntilSecondThreshold(thresholdTwo); 
    delay(200);

    if (counter % 2 == 0) {
      leftMarch();
    }
    else {
      rightMarch();
    }
    delay(500);

    // go forward
    forwardMarch();  

    // keep moving until we see something in front of us
    if (continueUntilObstacle(threshold, 2) == false) {

      // if we ran into something move backwards until second threshold is reached
      backwardsMarchUntilSecondThreshold(thresholdTwo); 
    }

    stopMarch();

    counter++;
  }

  Serial.println("Done.");

  digitalWrite(GREEN_LED, LOW);

  while(true) { 
  }
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










