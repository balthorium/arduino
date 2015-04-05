
#define inputPin 2 

void setup()
{
  pinMode(inputPin, INPUT);
  Serial.begin(9600);
}

void loop()
{
  int val = digitalRead(inputPin);
  if (val == HIGH) {
    Serial.println("It is very still here...");
  }
  else {
    Serial.println("Someone is MOVING!!!");
  }
  
  delay(100);
}

