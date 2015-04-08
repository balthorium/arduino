
#define IRoutputPin 2 
#define IRinputAnalogPin 0

void setup()
{
  pinMode(IRoutputPin,OUTPUT);
  digitalWrite(IRoutputPin,HIGH);
  Serial.begin(9600);
}

void loop()
{
  int val = analogRead(IRinputAnalogPin);
  String outputString = "Signal Strength: ";
 outputString += val; 
  Serial.println(outputString);
  delay(100);
}

