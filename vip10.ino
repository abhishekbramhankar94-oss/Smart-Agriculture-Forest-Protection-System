#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 (or 0x3F) for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int moisturePin = A0;   // Soil moisture sensor analog pin
const int relayPin = 7;       // Relay control pin for the pump
const int ledPin = 6;         // Red LED pin
const int buzzerPin = 8;      // Buzzer pin

// Threshold value for dry soil (adjust this based on your sensor calibration)
const int thresholdValue = 600; 

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Initial state
  digitalWrite(relayPin, HIGH); // Note: Many relay modules are Active LOW
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Initialize LCD
  lcd.init();                      
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Soil Monitor Sys");
  delay(2000);
  lcd.clear();
}

void loop() {
  int moistureValue = analogRead(moisturePin); // Read sensor value (0 to 1023)
  
  // Display moisture level on LCD
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moistureValue);
  lcd.print("   "); // Clear trailing characters

  lcd.setCursor(0, 1);
  
  // Check if soil is dry
  if (moistureValue > thresholdValue) {
    // Soil is DRY -> Turn ON pump, LED, and Buzzer alert
    lcd.print("Status: DRY (ON) ");
    digitalWrite(relayPin, LOW);  // Turn relay ON (change to HIGH if your relay is Active HIGH)
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
  } else {
    // Soil is WET -> Turn OFF pump, LED, and Buzzer
    lcd.print("Status: WET (OFF)");
    digitalWrite(relayPin, HIGH); // Turn relay OFF
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
  
  delay(1000); // Wait 1 second before next reading
}
