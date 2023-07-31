#include <DHT.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <Wire.h>

#define DHT_PIN 38

#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

#define DS1307_ADDRESS 0x68

DHT dht(DHT_PIN, DHT11);
Servo myservo;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//Fan numbers
const int speedPin = 6;
const int dir1 = 7;
const int dir2 = 8;
int mSpeed = 255;

//LED lights
const int yellowLED = 40;
const int greenLED = 42;
const int redLED = 44;
const int blueLED = 46;

//servo val
int servoVal;

//water level variables
int waterVal = 0;
int waterPin = A9;

//start button
const int buttonPin = 30;

//temp
float temp;

void turnOnFan(int control);
void printTemp();
void yellowLight(int control);
void greenLight(int control);
void redLight(int control);
void blueLight(int control);
void servoControl();
void getWaterLevel();
void displayTimeDate();
void displayErrorState(const char* errorMessage);

enum State{
  DISABLED,
  IDLE,
  ERROR_STATE,
  RUNNING
};

State currentState = DISABLED;
bool isFanMotorOn = false;

void setup() {
  // Initialize the DHT sensor
  pinMode(speedPin, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);
  Wire.begin();

  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  Serial.begin(9600);
  myservo.attach(22);

  dht.begin();

  // Initialize the LCD screen
  lcd.begin(16, 2);

  // Print a welcome message on the LCD
  lcd.print("DHT11 Sensor");
  lcd.setCursor(0, 1);
  lcd.print("Reading...");

  delay(2000);
  lcd.clear();
  yellowLight(0);
  greenLight(0);
  redLight(0);
  blueLight(0);

}

void loop() {
  int buttonState = digitalRead(buttonPin);
  switch (currentState){
    case DISABLED: 
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, HIGH);
      digitalWrite(blueLED, HIGH);
      if(buttonState == LOW){
        digitalWrite(yellowLED, HIGH);
        currentState = IDLE;
      }
      break;

    case IDLE:
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
      digitalWrite(blueLED, HIGH);
      servoControl();
      printTemp();
      getWaterLevel();
      if(waterVal <= 50){
        currentState = ERROR_STATE;
        displayErrorState("water level low");
      }
      if(temp > 26){
        turnOnFan(1);
        isFanMotorOn = true;
        displayTimeDate();
        currentState = RUNNING;
      }
      
      if(buttonState == HIGH){
        currentState = DISABLED;
      }
      break;

    case ERROR_STATE:
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
      digitalWrite(blueLED, HIGH);
      servoControl();
      turnOnFan(0);
      if (buttonState == HIGH) {
        digitalWrite(redLED, HIGH);
        currentState = IDLE;
      }
      break;

    case RUNNING:
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, HIGH);
      digitalWrite(blueLED, LOW);
      printTemp();
      servoControl();
      turnOnFan(1);
      if(temp <= 26){
        turnOnFan(0);
        isFanMotorOn = false;
        currentState = IDLE;
      }
      if(waterVal <= 50){
        //currentState = ERROR_STATE;
        displayErrorState("water level low");
      }
      break;
  }

}

//1 for on and 0 for off
void turnOnFan(int control){
  digitalWrite(dir1, HIGH);
  digitalWrite(dir2, LOW);
  if(control == 1){
    analogWrite(speedPin, mSpeed);
  }else{
    analogWrite(speedPin, 0);
  }
}

void printTemp(){
  temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any read errors occurred
  if (isnan(temp) || isnan(humidity)) {
    lcd.clear();
    lcd.print("Error reading");
    lcd.setCursor(0, 1);
    lcd.print("DHT11 sensor");
    delay(2000);
    lcd.clear();
    return;
  }

  // Display temperature and humidity on the LCD screen
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  delay(2000);
}

void yellowLight(int control){
  if(control == 1){
    digitalWrite(yellowLED, LOW);
  }else{
    digitalWrite(yellowLED, HIGH);
  }
}

void greenLight(int control){
  if(control == 1){
    digitalWrite(greenLED, LOW);
  }else{
    digitalWrite(greenLED, HIGH);
  }
}

void redLight(int control){
  if(control == 1){
    digitalWrite(redLED, LOW);
  }else{
    digitalWrite(redLED, HIGH);
  }
}

void blueLight(int control){
  if(control == 1){
    digitalWrite(blueLED, LOW);
  }else{
    digitalWrite(blueLED, HIGH);
  }
}

void servoControl(){
  servoVal = analogRead(10);
  servoVal = map(servoVal, 0, 1023, 0, 180);
  myservo.write(servoVal);
  delay(15);
}

void getWaterLevel(){
  waterVal = analogRead(waterPin);
}

void displayTimeDate() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = Wire.read();
  int minute = Wire.read();
  int hour = Wire.read();
  int dayOfWeek = Wire.read();
  int dayOfMonth = Wire.read();
  int month = Wire.read();
  int year = Wire.read();

 
  second = ((second & 0xF0) >> 4) * 10 + (second & 0x0F);
  minute = ((minute & 0xF0) >> 4) * 10 + (minute & 0x0F);
  hour = (((hour & 0x30) >> 4) * 10 + (hour & 0x0F)) % 12;
  dayOfWeek = dayOfWeek & 0x07; 
  dayOfMonth = ((dayOfMonth & 0xF0) >> 4) * 10 + (dayOfMonth & 0x0F);
  month = ((month & 0xF0) >> 4) * 10 + (month & 0x0F);
  year = ((year & 0xF0) >> 4) * 10 + (year & 0x0F);

  
  bool isPM = (hour >= 12);
  if (hour == 0) hour = 12;
  
  
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) Serial.print("0");
  Serial.print(minute);
  Serial.print(":");
  if (second < 10) Serial.print("0");
  Serial.print(second);

  Serial.print(" ");
  Serial.print(isPM ? "PM" : "AM");

  Serial.print(" | ");
  
  Serial.print(dayOfMonth);
  Serial.print("/");
  Serial.print(month);
  Serial.print("/");
  Serial.println(year);
}

void displayErrorState(const char* errorMessage) {
  lcd.clear();
  lcd.print("ERROR:");
  lcd.setCursor(0, 1);
  lcd.print(errorMessage);
}
