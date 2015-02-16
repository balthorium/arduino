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

// One-time setup code
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
  Serial.println("Hi there, I've started! :D");
  
  // initialize motor pins
  pinMode(DIR_A, OUTPUT);
  pinMode(BRAKE_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  pinMode(BRAKE_B, OUTPUT);    
}



void loop() {  
  
  // start moving!!
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(BRAKE_B, LOW);
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, 255);
  analogWrite(PWM_B, 255);

  // keep moving until we see something in front of us
  unsigned int proximity = readProximity();
  Serial.println(proximity, DEC);
  while (proximity < 2200) {
    Serial.println(proximity, DEC);
    proximity = readProximity();
  }

  // stop moving!!
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);
  digitalWrite(BRAKE_A, HIGH);
  digitalWrite(BRAKE_B, HIGH);
  
  while(true) { }
  
}


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
 