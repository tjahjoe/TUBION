float MAX = 0;

void setup() {
  Serial.begin(9600);
  pinMode(32, INPUT);
}

void loop() {
  float analog_value;
  float VRL;
  float RS;
  float RO;

  analog_value = analogRead(32);
  VRL = analog_value * (3.3 / 1023.0);
  RS = ((3.3 / VRL) - 1) * 10;
  RO = RS / 3.6;
  if (RO > MAX) {
    MAX = RO;
    Serial.println(RO);
  } 

  delay(1000);
}

// 3v3
