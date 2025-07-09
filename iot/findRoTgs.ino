float MAX = 0;

void setup() {
  Serial.begin(9600);
  pinMode(34, INPUT);
}

void loop() {
  float analog_value;
  float VRL;
  float RS;
  float RO;

  analog_value = analogRead(34);
  VRL = analog_value * (5 / 1023.0);
  RS = ((5 / VRL) - 1) * 10;
  RO = RS / 19;
  if (RO > MAX) {
    MAX = RO;
    Serial.println(RO);
  } 

  delay(1000);
}

// vin
