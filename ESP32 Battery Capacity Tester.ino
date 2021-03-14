const float ADC_LUT[4096] = { }; // generate yours from https://github.com/e-tinkers/esp32-adc-calibrate

// defining pins
const int led = 2;
const int touch = 4;
const int dac1 = 25;
const int dac2 = 26;
const int adc = 35;

const int touchThreshold = 30;
const int minBatteryVoltage = 2800; // mAh

int adcValue = 0;

bool isTouched() {
  if (touchRead(touch) < touchThreshold)
  {
    delay(500);
    if (touchRead(touch) < touchThreshold)
    {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

bool needsReset() {
  // checking if pin is kept touched for 3 sec
  if (isTouched()){
    for(unsigned int i=0;i<3;i++){
      if (!isTouched()){
        return false;
      }
      delay(500);
    }
    return true;
  }
  else {
    return false;
  }
}

void blinkLed() {
  while (true) {
    // blink twice
    for(unsigned int i=0;i<2;i++){
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);
    }
    if (needsReset()){
      // resetting, blink ten times
      Serial.println("Resetting in 5 sec ...");
      for(unsigned int i=0;i<10;i++){
        digitalWrite(led, HIGH);
        delay(100);
        digitalWrite(led, LOW);
        delay(100);
      }
      // wait for 5 sec then blink for 1 sec
      delay(5000);
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      break;
    }
    delay(3000);
  }
}

int batteryVoltage() {
  int calibratedReading = 0;
  // Taking 10 adc samples for stable reading
  for(unsigned int i=0;i<10;i++){
    adcValue = analogRead(adc);
    calibratedReading = calibratedReading + (int)ADC_LUT[adcValue];
    delay(10);
  }
  int avgreading = calibratedReading/10;

  // Adc value to voltage mapping multiplied by voltage divider factor
  return (map(avgreading, 0,4095,0,3300)) * 2;
}

void testCompleted(){
  // Job is done
  // set dac to zero to stop drain and watch
  dacWrite(dac1, 0);
  dacWrite(dac2, 0);
  Serial.println("Test Completed");
  blinkLed();
}

void setup() 
{
  Serial.begin(115200);
  pinMode (led, OUTPUT);
  dacWrite(dac1, 0);
  dacWrite(dac2, 0);
  digitalWrite(led, LOW);
}
void loop() 
{
  // Wait for touch
  Serial.println("Waiting for touch...");
  while (!isTouched()) {
    delay(1000);
  }
  digitalWrite(led, HIGH);

  if (batteryVoltage() <= minBatteryVoltage){
    testCompleted();
  }
  else {
    // start draining Battery and Watch
    dacWrite(dac1, 77); // 1v to op-amp
    dacWrite(dac2, 116); // 1.5v to analog watch
    Serial.println("Test Started");
    int voltage = 0;
    while (true){
      // Wait for Battery to drain checking every 1 sec.
      voltage = batteryVoltage();
      if (voltage > minBatteryVoltage) {
        Serial.print("Battery Voltage: ");
        Serial.println((float)voltage/1000);
        delay(1000);
      }
      else {
        break;
      }
    }
    testCompleted();
  }
}
