/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example prints the distance to an object.

  Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
*/

#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
SFEVL53L1X distanceSensor;

void setup(void)
{
  Wire.begin();

  Serial.begin(115200);
  Serial.println("VL53L1X Qwiic Test");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1){
      delay(10);
    }
  }
  Serial.println("Distance Sensor!");
    if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
    Serial.println("MPU!");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(500);
}

void loop(void)
{

  int distance;
  //Write configuration bytes to initiate measurement
  distanceSensor.startRanging(); 
  if (distanceSensor.checkForDataReady())
  {
     //Get the result of the measurement from the sensor
      distance = distanceSensor.getDistance();
  }

  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();


    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
     mpu.getEvent(&a, &g, &temp);


  /* Print out the values */
  Serial.print("Ax: ");
  Serial.print(a.acceleration.x);
  Serial.print(",  ");
  Serial.print("Ay: ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.print("Az: ");
  Serial.print(a.acceleration.z);
  Serial.print(", ");
  
  Serial.print("Gx: ");
  Serial.print(g.gyro.x);
  Serial.print(",  ");
  Serial.print("Gy: ");
  Serial.print(g.gyro.y);
  Serial.print(",  ");
  Serial.print("Gz: ");
  Serial.print(g.gyro.z);
  Serial.print(",  ");
  
  Serial.print("D(mm): ");
  Serial.print(distance);
  Serial.print(",  ");


  Serial.println();
}
