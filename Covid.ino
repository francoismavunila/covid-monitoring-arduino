#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>

#define EEPROM_SIZE 100
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


const char* ssid = "futuristic";
const char* password = "francois00001";

volatile int last_time;

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.43.14:1998/vaccinated/";
// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
#define RXD2 16
#define TXD2 17
String mess="";

const int barcodeButton = 35;

const int fingerprintButton = 26;

boolean enroll = false;
boolean auth = false;

void IRAM_ATTR registerPatient() {
  Serial.println("registering patient");

  enroll = true;
}

void IRAM_ATTR fingerprint() {
  int current_time = millis();

  if(current_time > (last_time + 10)){
      Serial.println("checking patient");
 
      auth = true; 
      last_time = current_time;
  }
}

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  //Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(barcodeButton, INPUT);
  pinMode(fingerprintButton, INPUT);

  attachInterrupt(digitalPinToInterrupt(barcodeButton), registerPatient, FALLING);
  attachInterrupt(digitalPinToInterrupt(fingerprintButton), fingerprint, FALLING);
    // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
    lcd.setCursor(0, 0);
  // print message
  lcd.print("waiting for next person");

    WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
    // set the data rate for the sensor serial port

  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  last_time = millis();
}
#include "enrollPassenger.h"
#include "checkPatient.h"
void loop() { 
  Serial.println("here");
  while (Serial2.available()) {
    mess = Serial2.readString();
    Serial.println(mess);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mess);
     if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String message = String(mess);
      String serverPath = serverName + "fingerprint?id="+message[0];
      serverPath = String(serverPath);
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      Serial.print(serverPath);
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      Serial.println(httpResponseCode);
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        if(httpResponseCode==200){
                  lcd.clear();
         lcd.print("authorised");
          delay(3000);
        }else{
         lcd.clear();
         lcd.print("network error");
          delay(3000);
        }

      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        lcd.print("server error");
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
        lcd.clear();
    lcd.print("waiting for next person");
  }
  if(enroll) {
    //call the authentication function
    Serial.println("call the enroll function");
   
    uint8_t patieId=  enrollPassenger();
    Serial.println("the id is");
    Serial.println(patieId);
          lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("id is:");
    lcd.setCursor(0, 1);
    lcd.print(patieId);
    uint8_t next_id=patieId+1;
    EEPROM.write(1,next_id );
    EEPROM.commit();
    enroll = false;
    delay(3000);
    lcd.clear();
    lcd.print("waiting for next person");
  }
   if(auth) {
    //call the authentication function
    Serial.println("call the fp check function");
    uint8_t vaccinatedFlag =  checkPatient() ;
    if(vaccinatedFlag!=150){
     Serial.println("vaccinated with id");
     Serial.println( vaccinatedFlag );
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("vaccinated");
      lcd.setCursor(0, 1);
      lcd.print("get in");
      if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      String serverPath = serverName + "fingerprint?id="+vaccinatedFlag;
      serverPath = String(serverPath);
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      Serial.print(serverPath);
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    }else{
     Serial.println("Not Vaccinated");   
    }

    auth = false;
        delay(3000);
    lcd.clear();
    lcd.print("waiting for next person");
  }
}
