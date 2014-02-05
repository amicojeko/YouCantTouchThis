/* 
 * Switches a LED, takes a picture and sends it via email
 * according to the state of the sensors output pin.
 * Determines the beginning and end of continuous motion sequences.
 *
 * @author: Stefano Guglielmetti / stefano (at) mikamai (dot) com / http://jeko.net
 * @date:   feb 5, 2014  
 *
 * based on the example by Kristian Gohlke / krigoo (_) gmail (_) com / http://krx.at
 * http://playground.arduino.cc/Code/PIRsense
 *
 * stefano guglielmetti (cleft) 2014 
 *
 * released under a creative commons "Attribution-NonCommercial-ShareAlike 2.0" license
 * http://creativecommons.org/licenses/by-nc-sa/2.0/de/
 *
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * (http://www.parallax.com/detail.asp?product_id=555-28027)
 *
 * The sensor's output pin goes to HIGH if motion is present.
 * However, even if motion is present it goes to LOW from time to time, 
 * which might give the impression no motion is present. 
 * This program deals with this issue by ignoring LOW-phases shorter than a given time, 
 * assuming continuous motion is present during these phases.
 *  
 */

#include <Bridge.h>

/////////////////////////////
//VARS
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;        

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 6;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;

Process p;
String imageName;

/////////////////////////////
//SETUP
void setup(){
  Bridge.begin();
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);

  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  for(int i = 0; i < calibrationTime; i++){
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
}

////////////////////////////
//LOOP
void loop(){

  if(digitalRead(pirPin) == HIGH){
    digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
    if(lockLow){  
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;            
      Serial.println("---");
      Serial.print("motion detected at ");
      Serial.print(millis()/1000);
      Serial.println(" sec"); 

      imageName = uniqueFileName("png"); //generate a new, uniqe file name

      p.runShellCommand("fswebcam /mnt/sda1/" + imageName); //takes the picture
      while(p.running()); //wait till the process ends
      p.runShellCommand("madplay /mnt/sda1/sounds/sirena.mp3"); //play the siren sound
      while(p.running()); //wait till the process ends
      p.runShellCommand("python /mnt/sda1/sendemail.py /mnt/sda1/" + imageName); //sends the picture via email
      while(p.running()); //wait till the process ends

      delay(50);
    }         
    takeLowTime = true;
  }

  if(digitalRead(pirPin) == LOW){       
    digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

    if(takeLowTime){
      lowIn = millis();          //save the time of the transition from high to LOW
      takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause, 
    //we assume that no more motion is going to happen
    if(!lockLow && millis() - lowIn > pause){  
      //makes sure this block of code is only executed again after 
      //a new motion sequence has been detected
      lockLow = true;                        
      Serial.print("motion ended at ");      //output
      Serial.print((millis() - pause)/1000);
      Serial.println(" sec");
      delay(50);
    }
  }
}


/* A simple function to generate unique timestamp based filenames */

String uniqueFileName(String ext){
  String filename = "";
  p.runShellCommand("date +%s");
  while(p.running()); 

  while (p.available()>0) {
    char c = p.read();
    filename += c;
  } 

  filename.trim();
  filename += "." + ext;

  return filename;
}  

