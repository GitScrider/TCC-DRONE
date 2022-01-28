/*********
  Igor Lacerda Tomich
  
  ESP32 WEB SERVER DRONE
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include "SPIFFS.h" 
#include "SparkFun_VL53L1X.h"
#include <ESP32Servo.h> // ESP32Servo library installed by Library Manager
#include "ESC.h" // RC_ESP library installed by Library Manager
#include <MPU6050_light.h>

#define MIN_SPEED 1000 // speed just slow enough to turn motor off
#define MAX_SPEED 2000 // speed where my motor drew 3.6 amps at 12v.

#define ENGINE1_PIN 12
#define ENGINE2_PIN 14
#define ENGINE3_PIN 27
#define ENGINE4_PIN 26

#define pi 3.14159265359

// Replace with your network credentials
const char* ssid = "56 kbps 2.4GHz";
const char* password = "00000001";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

//Init Servo for ESC30A
Servo esc1;
Servo esc2;
Servo esc3;
Servo esc4;

int Speed_Engine1 = 0;
int PWM_Engine1;
int Speed_Engine2 = 0;
int PWM_Engine2;
int Speed_Engine3 = 0;
int PWM_Engine3;
int Speed_Engine4 = 0;
int PWM_Engine4;

// Timer variables
unsigned long lastTime = 0;  
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long lastTimeDistance = 0;
unsigned long lastTimesenddata = 0;

unsigned long gyroDelay = 17;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;
unsigned long distanceDelay = 1000;
unsigned long senddataDelay = 100;

// Create a  mpu sensor object
Adafruit_MPU6050 mpuAdafruit;
MPU6050 mpu(Wire);


//Create a distance sensor object
SFEVL53L1X distanceSensor;

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;
int distance;

//Gyroscope sensor deviation
float gyroXerror = 0.05;
float gyroYerror = 0.05;
float gyroZerror = 0.01;

// Init MPU6050
void initMPU(){

  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  if(status!=0){
    
  }else{
      Serial.println("MPU6050 Found!");
  }
  
}

//Init VL51L1X distance sensor
void initVL51L1X(){
  if(distanceSensor.begin() != 0){
    Serial.println("Failed to find VL51L1X chip");

  }
  else{
    Serial.println("VL51L1X Found!");
   }
}

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}


void MPU6050Calibration(){
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");  
}

void ESCCalibration(){
  Serial.println("ESC calibration process");
  Serial.println(" ");
  delay(1500);
  Serial.println("Starting...");
  delay(1000);
  Serial.println("Alert! Motor will start spinning!");
         
  Serial.print("Now writing maximum output: (");
  Serial.print(MAX_SPEED);
  Serial.print(" us in this case)");
  Serial.print("\n");
  Serial.println("Make sure power is connected to ESC, wait for 2 seconds and press any key.");
  esc1.writeMicroseconds(MAX_SPEED);
  esc2.writeMicroseconds(MAX_SPEED);
  esc3.writeMicroseconds(MAX_SPEED);
  esc4.writeMicroseconds(MAX_SPEED);
  // Wait for input
  while (!Serial.available());
  Serial.read();

  // Send min output
  Serial.println("\n");
  Serial.println("\n");
  Serial.print("Sending minimum output: (");
  Serial.print(MIN_SPEED);Serial.print(" us)");
  Serial.print("\n");
  esc1.writeMicroseconds(MIN_SPEED);
  esc2.writeMicroseconds(MIN_SPEED);
  esc3.writeMicroseconds(MIN_SPEED);
  esc4.writeMicroseconds(MIN_SPEED);
  Serial.println("ESC finished calibrating");
}


String getChartData(){
  readings["Signal1"] = String(Speed_Engine1);
  readings["Signal2"] = String(Speed_Engine2);
  readings["Signal3"] = String(Speed_Engine3);
  readings["Signal4"] = String(Speed_Engine4);
  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);
  readings["Z"] = String(distance);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}


String getGyroReadings(){
  readings["gyroX"] = String(2*pi*gyroX/360);
  readings["gyroY"] = String(2*pi*gyroY/360);
  readings["gyroZ"] = String(2*pi*gyroZ/360);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccReadings() {
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify (readings);
  return accString;
}

String getTemperature(){
  return String(temperature);
}

String getZDistance(){

  return String(distance);
}


//Main Setup

void setup() {
 
  Wire.begin();
  Serial.begin(115200);
  initVL51L1X();
  initMPU();
  initWiFi();
  initSPIFFS();

  esc1.attach(ENGINE1_PIN);
  esc2.attach(ENGINE2_PIN);
  esc3.attach(ENGINE3_PIN);
  esc4.attach(ENGINE4_PIN);
  
  esc1.writeMicroseconds(1000);
  esc2.writeMicroseconds(1000);
  esc3.writeMicroseconds(1000);
  esc4.writeMicroseconds(1000);

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    gyroY=0;
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/MPU6050Calibration",HTTP_GET,[](AsyncWebServerRequest *request){
    MPU6050Calibration();
    request->send(200, "text/plain", "OK");
  });

//Event for engines

  server.on("/ESCCalibration",HTTP_GET,[](AsyncWebServerRequest *request){
    ESCCalibration();
    request->send(200, "text/plain", "OK");
  });

  server.on("/increaseengine1",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine1 + 5 <= 100){
      Speed_Engine1 = Speed_Engine1+5;
      //Speed_Engine2 = Speed_Engine2+5;
      //Speed_Engine3 = Speed_Engine3+5;
      //Speed_Engine4 = Speed_Engine4+5;
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/decreaseengine1",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine1-5 >= 0){
      Speed_Engine1 = Speed_Engine1-5;
      //Speed_Engine2 = Speed_Engine2-5;
      //Speed_Engine3 = Speed_Engine3-5;
      //Speed_Engine4 = Speed_Engine4-5;
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/increaseengine2",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine2 + 5 <= 100){
      Speed_Engine2 = Speed_Engine2+5;
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/decreaseengine2",HTTP_GET,[](AsyncWebServerRequest *request){
     if(Speed_Engine2-5 >= 0){
      Speed_Engine2 = Speed_Engine2-5;
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/increaseengine3",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine3 + 5 <= 100){
      Speed_Engine3 = Speed_Engine3+5;
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/decreaseengine3",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine3-5 >=0){
      Speed_Engine3 = Speed_Engine3-5;
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/increaseengine4",HTTP_GET,[](AsyncWebServerRequest *request){
      if(Speed_Engine4 + 5 <= 100){
        Speed_Engine4 = Speed_Engine4+5;
      }
      request->send(200, "text/plain", "OK");
  });

  server.on("/decreaseengine4",HTTP_GET,[](AsyncWebServerRequest *request){
    if(Speed_Engine4-5 >= 0){
      Speed_Engine4 = Speed_Engine4-5;
    }
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  server.addHandler(&events);
  server.begin();
}


void GetGY521Mesures(){
    mpu.update();
    gyroX = mpu.getAngleX();
    gyroY = mpu.getAngleY();
    gyroZ = mpu.getAngleZ();
    accX = mpu.getAccX();
    accY = mpu.getAccY();
    accZ = mpu.getAccZ();
    temperature = mpu.getTemp();
} 


void GetVL51L1XMesures(){
  //Write configuration bytes to initiate measurement
  distanceSensor.startRanging(); 
  if (!distanceSensor.checkForDataReady())
  {
    return;
  }
  //Get the result of the measurement from the sensor
  distance = distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
}

void SendData(){
  
  if ((millis() - lastTime) > gyroDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getGyroReadings().c_str(),"gyro_readings",millis());
    lastTime = millis();
  }
  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getAccReadings().c_str(),"accelerometer_readings",millis());
    lastTimeAcc = millis();
  }
  if ((millis() - lastTimeTemperature) > temperatureDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getTemperature().c_str(),"temperature_reading",millis());
    lastTimeTemperature = millis();
  }
  if ((millis() - lastTimeDistance) > distanceDelay) {
    //Send Events to the Web Server with the Sensor Readings
    events.send(getZDistance().c_str(),"distance_reading",millis());
    lastTimeDistance = millis();
  }
    if ((millis() - lastTimesenddata) > senddataDelay) {
    //Send Events to the Web Server with the Sensor Readings
    events.send(getChartData().c_str(),"graph_reading",millis());
    lastTimeDistance = millis();
  }

}

void DinamicControl(){

/* 
 * |F |   | 1  1  1  1| |F1|    |F1|       | 1 -1   1 -1| |F |     
 * |M1| = | 1 -1 -1  1| |F2|    |F2| =  1/4| 1  1   1  1| |M1|     
 * |M2|   |-1 -1  1  1| |F3|    |F3|       | 1  1  -1 -1| |M2|     
 * |M3|   |-1  1 -1  1| |F4|    |F4|       | 1 -1  -1  1| |M3|     
 */

int kx = 5;
int ky = 5;
int kz = 5;
int kh = 2;

//malha altitude
float eh = 2*(100-distance);
//malha giroscópio rolamento
float ex = 5*(gyroX);
//malha giroscópio arfagem
float ey = 5*(gyroY);
//malha giroscópio guinada
float ez = 5*(gyroZ);

Speed_Engine1 = 0.25*(1*eh - 1*ex + 1*ey - 1*ez);
Speed_Engine2 = 0.25*(1*eh + 1*ex + 1*ey + 1*ez);
Speed_Engine3 = 0.25*(1*eh + 1*ex - 1*ey - 1*ez);
Speed_Engine4 = 0.25*(1*eh - 1*ex - 1*ey + 1*ez);

//1
if(Speed_Engine1<0){
  Speed_Engine1=0;
}
else if(Speed_Engine1>100){
  Speed_Engine1=100;
}

//2
if(Speed_Engine2<0){
  Speed_Engine2=0;
}
else if(Speed_Engine2>100){
  Speed_Engine2=100;
}

//3
if(Speed_Engine3<0){
  Speed_Engine3=0;
}
else if(Speed_Engine3>100){
  Speed_Engine3=100;
}

//4
if(Speed_Engine4<0){
  Speed_Engine4=0;
}
else if(Speed_Engine4>100){
  Speed_Engine4=100;
}

  Serial.print("distance: ");
  Serial.print(distance);
  Serial.print(" errod: ");
  Serial.print(eh);
  Serial.print(" ; ");
  
  Serial.print("gyroX: ");
  Serial.print(gyroX);
  Serial.print(" errox: ");
  Serial.print(ex);
  Serial.print(" ; ");
  
  Serial.print("gyroY: ");
  Serial.print(gyroY);
  Serial.print(" erroy: ");
  Serial.print(ey);
  Serial.print(" ; ");
  
  Serial.print("gyroZ: ");
  Serial.print(gyroZ);
  Serial.print(" erroz: ");
  Serial.print(ez);
  Serial.print(" ; ");
  Serial.print(Speed_Engine1);
  Serial.print(" ; ");
  Serial.print(Speed_Engine2);
  Serial.print(" ; ");
  Serial.print(Speed_Engine3);
  Serial.print(" ; ");
  Serial.print(Speed_Engine4);
  
  Serial.println("");

}

void ConvertSpeedSignalToPWM(){

  PWM_Engine1 = map(Speed_Engine1, 0, 100, MIN_SPEED, MAX_SPEED); 
  PWM_Engine2 = map(Speed_Engine2, 0, 100, MIN_SPEED, MAX_SPEED); 
  PWM_Engine3 = map(Speed_Engine3, 0, 100, MIN_SPEED, MAX_SPEED); 
  PWM_Engine4 = map(Speed_Engine4, 0, 100, MIN_SPEED, MAX_SPEED); 

  esc1.writeMicroseconds(PWM_Engine1);
  esc2.writeMicroseconds(PWM_Engine2);
  esc3.writeMicroseconds(PWM_Engine3);
  esc4.writeMicroseconds(PWM_Engine4);

}

void loop() {
  
  GetGY521Mesures();
  GetVL51L1XMesures();
  DinamicControl();
  ConvertSpeedSignalToPWM();
  SendData();

  // Serial.print(Speed_Engine1);
  // Serial.print(" ; ");
  // Serial.print(Speed_Engine2);
  // Serial.print(" ; ");
  // Serial.print(Speed_Engine3);
  // Serial.print(" ; ");
  // Serial.print(Speed_Engine4);
  // Serial.println("");
  
}
