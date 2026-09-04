// =====================================================
//          SMART BEDSORE DETECTION SYSTEM
// =====================================================

#define BLYNK_TEMPLATE_ID "TMPL3tWEG24sh"
#define BLYNK_TEMPLATE_NAME "BedSore Detection"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
char ssid[] = "realme";
char pass[] = "12345678";

char auth[] = "z7cOy2BofK3_S71SgqtXxebeFsfGRtHN";

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

TwoWire OLED_I2C = TwoWire(0);
TwoWire BME_I2C  = TwoWire(1);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &OLED_I2C,
  -1
);

// ================= BME =================
Adafruit_BME280 bme;

// ================= PINS =================
const int FSR1_PIN = 34;
const int FSR2_PIN = 35;
const int FSR3_PIN = 32;
const int FSR4_PIN = 33;

// OLED
const int OLED_SDA = 18;
const int OLED_SCL = 19;

// BME280
const int BME_SDA = 21;
const int BME_SCL = 22;

// OUTPUTS
const int RED_LED = 25;
const int BUZZER_PIN = 26;

// ================= CONSTANTS =================

// Baseline skin temperature
const float BASELINE_TEMP = 34.5;

// Temperature rise threshold
const float TEMP_DELTA_RISK = 1.0;

// Force scaling
const float MAX_FORCE_N = 25.0;

// Effective contact area
const float CONTACT_AREA_CM2 = 20.0;

const float CONTACT_AREA_M2 =
      CONTACT_AREA_CM2 / 10000.0;

// ================= REALISTIC TIME WINDOWS =================

// Moderate
const unsigned long MODERATE_TIME =
      15UL * 60UL;

// High
const unsigned long HIGH_TIME =
      5UL * 60UL;

// Critical prolonged
const unsigned long CRITICAL_TIME =
      60UL * 60UL;

// ================= VARIABLES =================
unsigned long forceStart = 0;

bool forceDetected = false;

// ================= HELPERS =================
float clampValue(float x, float low, float high)
{
  if (x < low) return low;

  if (x > high) return high;

  return x;
}

// ADC → Force
float rawToForce(int raw)
{
  raw = constrain(raw, 0, 4095);

  return ((float)raw / 4095.0) * MAX_FORCE_N;
}

// Buzzer OFF
void buzzerOff()
{
  noTone(BUZZER_PIN);
}

// HIGH risk buzzer
void highRiskBuzzer(unsigned long now)
{
  if ((now / 300) % 2 == 0)
  {
    tone(BUZZER_PIN, 2200);
  }
  else
  {
    noTone(BUZZER_PIN);
  }
}

// CRITICAL risk buzzer
void criticalRiskBuzzer(unsigned long now)
{
  if ((now / 120) % 2 == 0)
  {
    tone(BUZZER_PIN, 3200);
  }
  else
  {
    noTone(BUZZER_PIN);
  }
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  // ================= OLED =================
  OLED_I2C.begin(OLED_SDA, OLED_SCL, 100000);

  bool oledOK = display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
  );

  if (!oledOK)
  {
    oledOK = display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3D
    );
  }

  if (oledOK)
  {
    display.clearDisplay();

    display.setTextColor(WHITE);

    display.setTextSize(2);

    display.setCursor(0,0);
    display.println("BEDSORE");

    display.setCursor(0,25);
    display.println("SYSTEM");

    display.display();

    delay(2000);
  }

  // ================= BME280 =================
  BME_I2C.begin(BME_SDA, BME_SCL, 100000);

  bme.begin(0x76, &BME_I2C);

  // ================= OUTPUTS =================
  pinMode(RED_LED, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RED_LED, LOW);

  buzzerOff();

  // ================= ADC =================
  analogReadResolution(12);

  // ================= BLYNK =================
  Blynk.begin(auth, ssid, pass);

  Serial.println("SYSTEM READY");
}

// ================= LOOP =================
void loop()
{
  Blynk.run();

  unsigned long now = millis();

  // =================================================
  //                  READ FSR
  // =================================================

  int raw1 = analogRead(FSR1_PIN);
  int raw2 = analogRead(FSR2_PIN);
  int raw3 = analogRead(FSR3_PIN);
  int raw4 = analogRead(FSR4_PIN);

  // =================================================
  //                  FORCE
  // =================================================

  float f1 = rawToForce(raw1);
  float f2 = rawToForce(raw2);
  float f3 = rawToForce(raw3);
  float f4 = rawToForce(raw4);

  float avgForce =
      (f1 + f2 + f3 + f4) / 4.0;

  // =================================================
  //               PRESSURE
  // =================================================

  float avgPressurePa =
      avgForce / CONTACT_AREA_M2;

  float avgPressureMmHg =
      avgPressurePa / 133.322f;

  // =================================================
  //               TEMPERATURE
  // =================================================

  float temperatureC =
      bme.readTemperature();

  // =================================================
  //               PRESSURE DURATION
  // =================================================

  if (avgPressureMmHg >= 32)
  {
    if (!forceDetected)
    {
      forceDetected = true;

      forceStart = now;
    }
  }
  else
  {
    forceDetected = false;
  }

  float forceDurationMin = 0;

  if (forceDetected)
  {
    forceDurationMin =
      (now - forceStart) / 60000.0;
  }

  // =================================================
  //                  RISK LEVEL
  // =================================================

  String riskLevel = "LOW";

  int displayRisk = 0;

  // LOW
  if (avgPressureMmHg < 32)
  {
    riskLevel = "LOW";

    displayRisk = 0;
  }

  // MODERATE
  if (
      avgPressureMmHg >= 32 &&
      avgPressureMmHg < 40 &&
      forceDurationMin >= 15
     )
  {
    riskLevel = "MODERATE";

    displayRisk = 25;
  }

  // HIGH
  if (
      avgPressureMmHg >= 40 &&
      avgPressureMmHg < 60 &&
      forceDurationMin >= 5
     )
  {
    riskLevel = "HIGH";

    displayRisk = 50;
  }

  // CRITICAL HIGH PRESSURE
  if (avgPressureMmHg >= 60)
  {
    riskLevel = "CRITICAL";

    displayRisk = 100;
  }

  // CRITICAL PROLONGED
  if (
      avgPressureMmHg >= 32 &&
      forceDurationMin >= 60
     )
  {
    riskLevel = "CRITICAL";

    displayRisk = 100;
  }

  // =================================================
  //                  ALERTS
  // =================================================

  if (displayRisk == 0)
  {
    digitalWrite(RED_LED, LOW);

    buzzerOff();
  }

  else if (displayRisk == 25)
  {
    digitalWrite(RED_LED, HIGH);

    buzzerOff();
  }

  else if (displayRisk == 50)
  {
    digitalWrite(RED_LED, HIGH);

    highRiskBuzzer(now);
  }

  else
  {
    digitalWrite(RED_LED, HIGH);

    criticalRiskBuzzer(now);

    Blynk.logEvent(
      "bedsore_alert",
      "CRITICAL BEDSORE RISK!"
    );
  }

  // =================================================
  //                    BLYNK
  // =================================================

  Blynk.virtualWrite(V0, temperatureC);

  Blynk.virtualWrite(V1, riskLevel);

  Blynk.virtualWrite(V2, displayRisk);

  Blynk.virtualWrite(V3, avgPressureMmHg);

  Blynk.virtualWrite(V4, avgForce);

  // =================================================
  //                  OLED DISPLAY
  // =================================================

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);

  display.setCursor(0,0);
  display.println("BEDSORE ALERTS");

  display.setCursor(0,12);
  display.print("Avg Pressure:");
  display.print(avgPressureMmHg,0);
  display.println(" mmHg");

  display.setCursor(0,24);
  display.print("Avg Force:");
  display.print(avgForce,1);
  display.println(" N");

  display.setCursor(0,36);
  display.print("Temperature:");
  display.print(temperatureC,1);
  display.println(" C");

  display.setCursor(0,48);
  display.print("Risk Score:");
  display.print(displayRisk);
  display.println("%");

  display.setCursor(0,58);
  display.print("Risk Status:");
  display.print(riskLevel);

  display.display();

  // =================================================
  //                    SERIAL
  // =================================================

  Serial.print("Pressure(mmHg): ");
  Serial.print(avgPressureMmHg);

  Serial.print(" Force(N): ");
  Serial.print(avgForce);

  Serial.print(" Temp(C): ");
  Serial.print(temperatureC);

  Serial.print(" Duration(min): ");
  Serial.print(forceDurationMin);

  Serial.print(" Risk: ");
  Serial.print(riskLevel);

  Serial.print(" Score: ");
  Serial.println(displayRisk);

  delay(500);
}