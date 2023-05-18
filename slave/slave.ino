
/** 
 * @file 
 * @brief This is what control the car and recive the data from the glove. This code is inspried by:  
 * - Grove - Serial Bluetooth v3.0 examples button (website: https://wiki.seeedstudio.com/Grove-Serial_Bluetooth_v3.0/)
 * - Servo Motor Basics with Arduino: https://docs.arduino.cc/learn/electronics/servo-motors
 * - L9110S H-bridge Dual DC Stepper Motor Driver Controller: https://arduinotech.dk/shop/l9110s-h-bridge-dual-dc-stepper-motor-driver-controller/
 */ 

#include <SoftwareSerial.h>  //Software Serial Port
#include <Servo.h> //For controlling the Servo

#define RxD 2 /**< pin for the bluetooth component */
#define TxD 3 /**< pin for the bluetooth component */


// Motor A pins 
#define A1 5/**< pin for Motor A */
#define A2 6/**< pin for Motor A */
// Motor B pins
#define B1 10/**< pin for Motor B */
#define B2 11/**< pin for Motor B */
//pin for axis/Servo
#define A3 9 /**< pin for controlling the servo*/

//for controlling the car
double forward = 0; /**< The value that controll the forward momententiom. It controll the if it should move forward, backwards or stand still*/
double turn = 0; /**< In degrees how much the front of the car have turned*/
double min = 2;/**< if foward is less then min the car dont move at all*/

Servo axisServo; /**< Instance of the Servo that controll the front of the car*/

SoftwareSerial blueToothSerial(RxD, TxD);/**< create the the softwareSerial */

const unsigned int MAX_MESSAGE_LENGTH = 12;/**< The max length of chacters of the bluetooth message */

/**
 * @brief This is the setup function that is run when the board is started and it setup the outputs,input and create a connection to the master bord
 * 
 */
void setup() {
  Serial.begin(9600);
  //set input and output pins for bluetooth
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  //Set pin for the servo motor
  axisServo.attach(A3);
  //get a bluetooth connect to the masterbord
  setupBlueToothConnection();
}

/**
 * @brief This is the function that update the bluetooth data and controlls the car
 * 
 */
void loop() {
  if (blueToothSerial.available()) {  //check if there's any data sent from the remote bluetooth shield
    // recieve the bluetooth data
    recieveBluetoothData();
    //control the motor that move the car forward
    feedForwardControl();
    // controll the turning servo
    turnControl();
  }
}

/**
 * @brief recieve data from the master.ino bord with bluetooth. The data is then read and turn and forward values is calulated
 * 
 */
void recieveBluetoothData() {

  ///have a static char array with a length of MAX_MESSAGE_LENGTH
  // a static char array that have the message from the bluettoht buffer
  static char message[MAX_MESSAGE_LENGTH];
  //the index of the current char
  static unsigned int message_pos = 0;

  //Read the next available byte in the serial receive buffer
  char inByte = blueToothSerial.read();
  ///read every symbol of the message
  //Message coming in (check not terminating character)
  if (inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)) {
    //Add the incoming byte to our message
    message[message_pos] = inByte;
    message_pos++;
  }
  ///If full message received
  else {
    //print the message
    Serial.print("The message:");
    //Print the message (or do other things)
    Serial.println(message);
    //the char of the serpperation chars
    char dl1 = 0x11;
    char dl2 = 0x12;
    char indexC = 0x13;
    //The roll value and picht value as strings
    String r_v = "", p_v = "";
    ///read the message and save it in two strings
    for (int i = 0; i < MAX_MESSAGE_LENGTH - 1; i++) {
      char current = message[i];
      if (current == 00) {
      } else if (current == dl2 || current == dl1) {
        indexC = current;
      } else if (indexC == dl2) {
        p_v += current;
      } else if (indexC == dl1) {
        r_v += current;
      }
    }
    ///converting the strings to doubles and calculate the forward and turn values
    double roll = atof(r_v.c_str());
    double picht = atof(p_v.c_str());
    forward = 25.5 * picht;
    turn = 180-(roll+10)*9;

    //print the foward and turn values and the roll and picht values
    Serial.println("turn: " + String(roll) +" , " + String(forward)+ " forward: " + String(picht)+" , " + String(turn));
    ///Add null character to string
    message[message_pos] = '\0';
    //rest the index of the postion of the message
    message_pos = 0;
  }
}
/**
 * @brief turns the car according to the turn variable by using the servo motor
 * 
 */
void  turnControl(){
  axisServo.write(turn);
}

/**
 * @brief Controll the motors that makes the car move arrcording to the forward variable
 * 
 */
void feedForwardControl() {
  ///if the forward is bigger then the minimum value for forward (min) then the car can move
  if (abs(forward) > min) {
    ///if foward is postive the car move forward else it move backwards
    if (forward > 0) {
      analogWrite(A1, forward);
      analogWrite(A2, 0);
      analogWrite(B1, forward);
      analogWrite(B2, 0);
    } else {
      analogWrite(A1, 0);
      analogWrite(A2, forward);
      analogWrite(B1, 0);
      analogWrite(B2, forward);
    }
  /// if the values of foward is less the car dont move it wheels.  
  } else {
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
    digitalWrite(B1, LOW);
    digitalWrite(B2, LOW);
  }
}

/**
 * @brief inspried by the example code called:Master_button setup code
website: https://wiki.seeedstudio.com/Grove-Serial_Bluetooth_v3.0/
 *It initilizing bluetooth connction
*/
void setupBlueToothConnection() {
  blueToothSerial.begin(9600);
  blueToothSerial.print("AT");
  delay(400);

  blueToothSerial.print("AT+DEFAULT");  // Restore all setup value to factory setup
  delay(2000);

  blueToothSerial.print("AT+NAMESeeedBTSlave");  // set the bluetooth name as "SeeedBTSlave" ,the length of bluetooth name must less than 12 characters.
  delay(400);

  blueToothSerial.print("AT+PIN0000");  // set the pair code to connect
  delay(400);

  Serial.println("pin set to 0000"); //

  blueToothSerial.print("AT+AUTH1");  //
  delay(400);

  blueToothSerial.flush(); //Flushes the blueToothSerial

  Serial.println("Ready to go"); //write it is ready to go 
}
