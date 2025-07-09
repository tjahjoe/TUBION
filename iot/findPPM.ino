#define RL 1       
#define m -0.417
#define b -0.858
#define Ro 7
#define MQ_sensor 32

const int numReadings = 5;
float readings[numReadings];
int readIndex = 0;
float total = 0;
float average = 0;

void setup() {
  Serial.begin(9600);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  for (int times = 1; times <= 10; times++) {
    Serial.println(times);
    delay(1000);
  }
}

void loop() {
  float VRL;
  float RS;
  float ratio;

  VRL = analogRead(MQ_sensor) * (3.3 / 4095.0); // untuk ESP32 12-bit ADC
  RS = (3.3 / VRL - 1) * RL;
  ratio = RS / Ro;

  float ppm = pow(10, ((log10(ratio) - b) / m));

  total = total - readings[readIndex];
  readings[readIndex] = ppm;
  total = total + readings[readIndex];

  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  average = total / numReadings;
  Serial.println(average);

  delay(1000);
}
