#include <Wire.h>
#include <LiquidCrystal_I2C.h>    // Include LCD I2C library
#include "MAX30105.h"             // MAX3010x library
#include "heartRate.h"            // Heart rate algorithm

#define LM35_PIN A0               // LM35 temperature sensor pin

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // Set the LCD I2C address & size (16x2)

void setup() {
  Wire.setClock(400000);                    // I2C speed 400kHz
  lcd.init();                              // Initialize the LCD
  lcd.backlight();                         // Turn on the backlight

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    lcd.setCursor(0, 0);
    lcd.print("MAX30105 ERROR");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Health Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BPM: 0");
  lcd.setCursor(0, 1);
  lcd.print("Temp: --");
}

void loop() {
  long irValue = particleSensor.getIR();

  // Read temperature from LM35
  int lm35Reading = analogRead(LM35_PIN);
  float temperatureC = (lm35Reading * 5.0 / 1023.0) * 100.0;
  float temperatureF = (temperatureC * 9.0 / 5.0) + 28.0;


  if (irValue > 50000) {
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BPM:");
        lcd.print(beatAvg);

        lcd.setCursor(0, 1);
        lcd.print("Temp: ");
        lcd.print(temperatureF, 0);
        lcd.print((char)223); // Degree symbol
        lcd.print("F");
    

      }
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wear The Strap");
    lcd.setCursor(0, 1);
    lcd.print("Properly...");
  }
}
