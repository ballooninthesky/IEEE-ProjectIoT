#include <Wire.h>
#include <Servo.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

#define trig 12
#define echo 13
#define DHTPIN 4
#define DHTTYPE DHT11
#define BUTTON_PIN 3 
#define PIR_PIN 7

unsigned long pirPreviousMillis = 0;
unsigned long previousMillis = 0;
const long interval = 1000;
const long pirInterval = 500;

long duration;
float distance;

const int IDLE = 0;
const int MEASURE = 1;
const int DISTANCE = 2;
int state;
int displayMode = MEASURE; // โหมดการแสดงผลเริ่มต้น
volatile bool flag_change_mode = false; // ตัวแปรสำหรับการขัดจังหวะ

Servo myservo;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);


// ISR สำหรับการขัดจังหวะของปุ่มเปลี่ยนโหมด
void handleButtonInterrupt() { 
    flag_change_mode = true;
}

void setup() {
    pinMode(PIR_PIN, INPUT_PULLUP);
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // ตั้งค่าให้ปุ่มเป็น INPUT พร้อมใช้งาน PULLUP
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, RISING); // ตั้งค่าการขัดจังหวะ
    Serial.begin(9600);
    myservo.attach(9);
    lcd.begin();
    lcd.backlight();
    dht.begin();
    state = IDLE;
}

void loop() {
    unsigned long currentMillis = millis(); // อัปเดตเวลาปัจจุบันทุกครั้งใน loop
    // ตรวจสอบสถานะ
    if (state == IDLE) {
      if (flag_change_mode) {
          flag_change_mode = false; // รีเซ็ตตัวแปรการขัดจังหวะ
          // เปลี่ยนโหมดการแสดงผล
          if (displayMode == MEASURE) {
              displayMode = DISTANCE;
          } else {
              displayMode = MEASURE;
          }
      }
        state = displayMode;
    }

    else if (state == MEASURE) {
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis; // บันทึกเวลาใหม่

            float h = dht.readHumidity();
            float t = dht.readTemperature();
            Serial.print("Humidity: ");
            Serial.print(h);
            Serial.print("%, Temperature: ");
            Serial.print(t);
            Serial.println("°C");

            // แสดงผลบน LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Humidity: ");
            lcd.print(h);
            lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print("Temp: ");
            lcd.print(t);
            lcd.print(" C");

        }
        state = IDLE;
    }
    
    else if (state == DISTANCE) {
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis; // บันทึกเวลาใหม่

            digitalWrite(trig, LOW);
            delayMicroseconds(2);
            digitalWrite(trig, HIGH);
            delayMicroseconds(10);
            digitalWrite(trig, LOW);

            duration = pulseIn(echo, HIGH);
            distance = (duration * 0.034) / 2;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Distance: ");
            lcd.print(distance);
            lcd.print(" cm");

            if (distance < 10) {
                myservo.write(0);
            } else {
                myservo.write(90);
            }
        }
        state = IDLE;
    }
    1

}
