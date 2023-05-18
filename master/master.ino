
/** @file 
 * @brief This is what sent instructens to the car from the user gestures. This code is inspried by: A library for SeeedStudio Grove FM (Website: www.seeed.cc) by Steve Chang and Grove - Serial Bluetooth v3.0 examples (website: https://wiki.seeedstudio.com/Grove-Serial_Bluetooth_v3.0/)
 
 */ 

///include librays for the 9DOF
#include "AK09918.h" // for error codes
#include "ICM20600.h"// for reading the sensore
#include <Wire.h>
#include <SoftwareSerial.h>  // Software Serial Port

//Code for the bluetooth device
#define RxD 2/**< pin for the bluetooth component */
#define TxD 3/**< pin for the bluetooth component */

//code for the 9DOF sensor
SoftwareSerial blueToothSerial(RxD, TxD);/**< create the the softwareSerial */
AK09918_err_type_t err;/**< The 9DOF error type that get used when starting the sensor to check that it is running */
AK09918 ak09918;/**< A instance of the 9DOF sensor that checks if it is up and running*/
ICM20600 icm20600(true);/**< A instance of the 9DOF sensor where the sensor can be read */

//9DOF sensor output/**< Detailed description after the member */
int16_t acc_x;/**< The x value of the acceleration messured by the 9dof */
int16_t acc_y;/**< The y value of the acceleration messured by the 9dof  */
int16_t acc_z;/**< The z value of the acceleration messured by the 9dof  */
int16_t gyro_x;/**< The x value of the Gyroscope messured by the 9dof  */
int16_t gyro_y;/**< The y value of the Gyroscope messured by the 9dof  */
int16_t gyro_z;/**< The z value of the Gyroscope messured by the 9dof  */


//To controll the motor
double roll; /**< The calculated roll of the glove from accelaeration*/
double pitch;/**< The calculated pitch of the glove from accelaeration */


/**
 * @brief This function when the bord start and setup the bluetooth connection with Grove - Serial Bluetooth v3.0 and calibrate the 9DOF sensor
 * 
 */
void setup()
{
  //Code for bluetooth device
  Serial.println("Start the bluetooth device:");
  Serial.begin(9600);
  Serial.print(".");
  //Set the pinmodes
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  //print debug message to show how long it its
  Serial.print("...");
  //setup the bluetooth connection
  setupBlueToothConnection();
  //print debug message to show how long it its
  Serial.print("......");
  //wait 1s and flush the serial buffer
  delay(1000);
  Serial.print("...");
  Serial.flush();
  blueToothSerial.flush();
  //print debug message to show how long it its and print that it is complete
  Serial.print("......");
  Serial.println("master set up complete");

  //code for the 9DOF sensor
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  //initialize the ak09918
  err = ak09918.initialize();
  icm20600.initialize();
  //set the swithmode
  ak09918.switchMode(AK09918_POWER_DOWN);
  ak09918.switchMode(AK09918_CONTINUOUS_100HZ);
  Serial.begin(9600);

  //wating for the sensory to start
  err = ak09918.isDataReady();
  while (err != AK09918_ERR_OK) {
    Serial.println("Waiting Sensor");
    delay(100);
    err = ak09918.isDataReady();
  }

  //calibrate the sensor
  Serial.println("Start figure-8 calibration after 12 seconds.");
  delay(2000);
}


/**
 * @brief This read the Accelaeration from the 9DOF.
 * 
 */
void updateAcc() {
  acc_x = icm20600.getAccelerationX();
  acc_y = icm20600.getAccelerationY();
  acc_z = icm20600.getAccelerationZ();
}

/**
 * @brief This read the Gyroscope from the 9DOF.
 * 
 */
void updateGyro() {
  gyro_x = icm20600.getGyroscopeX();
  gyro_y = icm20600.getGyroscopeY();
  gyro_z = icm20600.getGyroscopeZ();
}

/**
 * @brief This function claculate the roll of the glove from the accelaition.
 * 
 */
void calcRoll() {
  roll = atan2((float)acc_y, (float)acc_z);
}
/**
 * @brief This function calculate the picht of the glove from the accelaration
 * 
 */
void calcPitch() {
  pitch = atan2(-(float)acc_x, sqrt((float)acc_y * acc_y + (float)acc_z * acc_z));
}


/**
 * @brief This is the loop that is run everything.
 * 
 */
void loop()
{
  //update the accealertions
  updateAcc();
  //update the gyro
  updateGyro();
  //calculate the roll
  calcRoll();
  //calculate the pithc
  calcPitch();
  //take the roll and picth and package and send it
  blueToothTransfore();
  //create a 1 secon delay
  delay(1000);
}

/**
 * @brief This function packages the picht and roll and send it to the slave.
 * 
 */
void blueToothTransfore() {
  //create an empty string 
  String SerialData = "";
  //the 2 speration chars
  char dl1 = 0x11;
  char dl2 = 0x12;
  //converting roll and picht to string while calculateing it from -10 to 10.
  String r = String((constrain(roll * 57.3, -90, 90) / 9), 1);
  String p = String((constrain(pitch * 57.3 * (-1), -60, 60) / 6), 1);
  //Taking all the data and package it
  SerialData = dl1 + r + dl2 + p;
  //Write out the data
  Serial.println(SerialData);
  //put the data in the bluetooth buffer
  blueToothSerial.print(SerialData);
  blueToothSerial.print("\n");
}

/**
 * @brief inspried by the example code called:Master_button setup code
website: https://wiki.seeedstudio.com/Grove-Serial_Bluetooth_v3.0/
 * 
 */
void setupBlueToothConnection()

{
  blueToothSerial.begin(9600);
  blueToothSerial.print("AT");
  delay(400);
  blueToothSerial.print("AT+DEFAULT");  // Restore all setup value to factory setup
  delay(2000);
  blueToothSerial.print("AT+NAMEmyMaster");  // set the bluetooth name as "SeeedMaster" ,the length of bluetooth name must less than 12 characters.
  delay(400);
  Serial.println("Bluetooth slave name st as myMaster");
  blueToothSerial.print("AT+ROLEM");  // set the bluetooth work in Master mode
  delay(400);
  blueToothSerial.print("AT+PIN0000");  // set the pair code to connect
  delay(400);
  Serial.println("pin set to 0000");
  blueToothSerial.print("AT+AUTH1");
  delay(400);
  blueToothSerial.print("AT+CLEAR");  // Clear connected device mac address
  delay(400);
  blueToothSerial.flush();
  Serial.println("Ready to go");
}