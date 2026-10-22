#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// I2C LCD 16x2 (SDA -> Pin 20, SCL -> Pin 21)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// पिन्स की जानकारी (Pin Definitions)
#define DHTPIN 4          // DHT11 Sensor Pin
#define DHTTYPE DHT11     // DHT Sensor Type
#define VIBRATION_PIN 2   // Vibration Sensor Digital Pin
#define RAIN_PIN A0       // Raindrop Sensor Analog Pin
#define MQ135_PIN A1      // MQ135 Gas Sensor Analog Pin
#define BUZZER_PIN 8      // Buzzer Pin
#define LED_PIN 9         // LED Pin

DHT dht(DHTPIN, DHTTYPE);

// वेरिएबल्स (Variables)
int vibrationCount = 0;
unsigned long previousMillis = 0;
unsigned long vibrationStartTime = 0; 
const long interval = 5000;          // ब्लूटूथ अपडेट इंटरवल (5 सेकंड)
const long alertDuration = 5000;      // वाइब्रेशन वार्निंग का समय (5 सेकंड)
bool isAlertActive = false;
bool alertSentOverBT = false;         

// खतरे की लिमिट (Threshold Limits)
const int TEMP_LIMIT = 36;   // 39°C से ज़्यादा होने पर वार्निंग
const int AQI_LIMIT = 500;   // 400 से ज़्यादा AQI होने पर वार्निंग

void setup() {
  Serial.begin(9600);    // सीरियल मॉनिटर
  Serial2.begin(9600);   // ब्लूटूथ अब Mega के Pin 16 (RX2) और Pin 17 (TX2) पर काम करेगा

  lcd.init();
  lcd.backlight();
  dht.begin();

  pinMode(VIBRATION_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Forest Monitor");
  lcd.setCursor(0, 1);
  lcd.print("System Ready...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. सेंसर्स से डेटा पढ़ना
  float temp = dht.readTemperature();
  int rainVal = analogRead(RAIN_PIN);
  int mq135Val = analogRead(MQ135_PIN);

  // 2. वाइब्रेशन सेंसर की स्थिति चेक करना
  int vibrationState = digitalRead(VIBRATION_PIN);
  
  if (vibrationState == HIGH) {
    vibrationCount++;
    if (vibrationCount >= 5 && !isAlertActive) {
      isAlertActive = true;
      vibrationStartTime = millis(); 
      alertSentOverBT = false; 
    }
    delay(100);
  }

  // 3. वार्निंग ऑटो-ऑफ करना (5 सेकंड बाद)
  if (isAlertActive && (millis() - vibrationStartTime >= alertDuration)) {
    isAlertActive = false;
    vibrationCount = 0; 
  }

  // 4. अन्य लिमिट चेक करना
  bool hazardDetected = isAlertActive || (temp >= TEMP_LIMIT) || (mq135Val >= AQI_LIMIT);
  String alertMessage = "";

  if (isAlertActive) {
    alertMessage = "Vib Detected!";
  } else if (temp >= TEMP_LIMIT) {
    alertMessage = "High Temp!";
  } else if (mq135Val >= AQI_LIMIT) {
    alertMessage = "Bad Air Quality!";
  }

  // 5. LCD डिस्प्ले और बज़र/एलईडी को कंट्रोल करना
  lcd.clear();

  if (hazardDetected) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    
    lcd.setCursor(0, 0);
    lcd.print("WARNING!");
    lcd.setCursor(0, 1);
    lcd.print(alertMessage);
    
    // ब्लूटूथ पर सिर्फ एक बार अलर्ट भेजें (Serial2 के जरिए)
    if (!alertSentOverBT) {
      Serial2.print("WARNING: ");
      Serial2.println(alertMessage);
      alertSentOverBT = true;
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // पहली लाइन: Tem और AQI नंबर
    lcd.setCursor(0, 0);
    lcd.print("Tem:");
    lcd.print((int)temp);
    lcd.print((char)223); // डिग्री सिंबल (°)
    lcd.print("C AQI:");
    lcd.print(mq135Val);

    // दूसरी लाइन: Rain Status
    lcd.setCursor(0, 1);
    if (rainVal < 500) {
      lcd.print("Rain: Yes");
    } else {
      lcd.print("Rain: No");
    }
  }

  // 6. ब्लूटूथ (Phone) पर नियमित अंतराल पर डेटा भेजना (Serial2 के जरिए)
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Serial2.println("--- Forest Status Update ---");
    Serial2.print("Temperature : "); Serial2.print(temp); Serial2.println(" C");
    Serial2.print("AQI Value   : "); Serial2.println(mq135Val);
    if (rainVal < 500) {
      Serial2.println("Rain Status : Rain Detected!");
    } else {
      Serial2.println("Rain Status : No Rain");
    }
    Serial2.println("----------------------------");
  }

  delay(1000);
}
