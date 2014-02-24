# You can't touch this!

![title](http://i.imgur.com/acHM30xl.jpg)

This is my entry for the [Connected Home Project Contest](http://makezine.com/connected-home-project-contest/) by Make Magazine

Check the [Youtube Video](https://www.youtube.com/watch?v=a-asHQZwfVU)!

This project consists in a movement sensor (PIR) plus a camera and a wifi connected device (Arduino Yún was my choice, but it can be easily substitued with a RaspberryPi). Optionally, speakers can be connected to reproduce an alarm sound.

The purpose is clear. You don't want your kids to steal your food from the cupboard, or from the fridge, or someone to open your locket, or you want to take pictures to your pet stealing food, or you are Dwight Schrute and you want to finally unmask the coworker that puts your stuff into jelly... so you hide the device into the cupboard/fridge/locket, and when the device detects some movement, it will take a picture and post it right to your email! And if you use IFTTT, then you can automatically post the picture of the thief to Facebook, Twitter and show the thief's face to the entire world!

The project is very simple, it doesn't require any soldering/electronics skills and it can be assembled in minutes.

As I said before, my platform choice is Arduino Yún, because it's very easy to use, and it's easy to configure the wifi settings. It's a bit more expensive than the RaspberryPi, but if you use the Yún you can give you project to someone else as a gift, and anyone can configure the wifi settings, while this may be a difficult task with the RaspberryPi, without connecting it to a monitor/mouse/keyboard. But if you like it, I can easily make a Raspberry version.

So, here are the ingredients

- [Arduino Yún](http://www.makershed.com/Arduino_Yun_p/mksp24.htm)
- [PIR Sensor](http://www.makershed.com/product_p/mkpx6.htm)
- UVC compatible USB Camera (don't worry: almost all the USB cameras are UVC compatible)
- Micro USB power adapter or USB Battery pack
- Recommended: micro SD card 
- Optional: [USB Sound card](http://www.manhattan-products.com/hi-speed-usb-3-d-sound-adapter)
- Optional: USB HUB (if you want both the USB Camera and the USB Sound card)

![totalone](http://i.imgur.com/c4Kj6Uil.jpg)

Ok, let's start!

## Step 1

**Arduino basic configuration**

First of all, you need to configure your Arduino Yún network settings. This should be pretty easy [following this guide](http://arduino.cc/en/Guide/ArduinoYun#toc13)

Then test the connection to your Arduino: in your browser type [http://arduino.local](http://arduino.local), you will see the Arduino web interface

If it works, open SSH session

	$ssh root@arduino.local

the default password is "arduino"

Then let's install some useful packages

	$opkg update
	$opkg install openssh-sftp-server

Why the openssh-sftp-server? Because it would be easier for you to upload and download files from/to the Arduino: now you can use a generic SFTP client (filezilla, transmit or cyberduck) instead of SCP command

Then, we have have to install the SSL support package for Python (thanks [sbkirby](https://github.com/sbkirby)! Because it's not included in the default packages

	$opkg update
	$opkg install python-openssl


If you have one, I highly recommend to put a micro SD card into the Arduino Yún. It will be automatically mounted in /mnt/sda1 

Then, we will install the USB Camera and USB Soundcard on the Yún

## Step 2

**Installing and testing the USB Camera**

The UVC package is already available fot Linino via opkg, so installing the Camera is a very easy step, just connect to your Yún via ssh and type

	$ opkg update
	$ opkg install kmod-video-uvc

We also need a software for taking pictures, I used fswebcam that is supersmall and very easy to use

	$ opkg install fswebcam

## Step 3

**Take your first picture**

First of all, be sure to use and SD card for storage or you will quickly fill the Arduino internal storage, and you don't want to do that. If you are using an SD card, you should see it mounted in /mnt/sda1. If it's not, put a FAT32 formatted micro SD card into the SD Card Slot, and reboot your Arduino

Now plug the camera to the USB host port, and type

	$ cd /mnt/sda1
	$ fswebcam test.png

If everything is ok, you took your first picture! Take a look to your SD card contents :)

This means that now we can take pictures from our Arduino sketch, via the Yún's Bridge library, in this way

	Process.runShellCommand("fswebcam /mnt/sda1/test.png");

## Step 4 (optional)

**Install the sound card**

Open an ssh session to the yun, and type:

	$opkg install kmod-usb-audio
	$opkg install madplay

End of the story. Now you have sound support on the Arduino Yún. I only tried MP3 playback, I didn't try to record some audio yet, but I'll try soon!

To test it just copy an mp3 audio file to the SD card, and type
	
	$cd /mnt/sda1
	$madplay yoursound.mp3	

This means that now we can take play sounds from our Arduino sketch, via the Yún's Bridge library, in this way

	Process.runShellCommand("madplay /mnt/sda1/test.mp3");

We're learning a lot of interesting things!! :)


## Step 5

**The Email script**

Now we can take pictures and play sounds... But we want more from our Arduino! We want to send them via email.

So, how can we do? The answer is very easy... python script! Even if the Arduino Yún integrates the Temboo library, I wanted this project to be as much portable as possible, so we will use a very simple python script to encode the image file and send it to our email.

I'm assuming that you are using GMail, but the script can be easily adapted to any SMTP server

Create a new file, call it "sendemail.py" and paste this code into it

```python
# coding=utf-8

# Copyright (C) 2014  Stefano Guglielmetti

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import smtplib, os, sys
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders

#From address, to address, subject and message body
from_address    = 'FROM_ADDRESS@EMAIL.COM'
to_address      = ['YOUR_ADDRESS@EMAIL.COM']
email_subject   = 'Alert!!! Zombies!!! Ahead!!!'
email_body      = 'A non dead intruder has been detected and needs to be eliminated!'

# Credentials (if needed)
username = 'EMAIL_LOGIN'
password = 'EMAIL_PASSWORD'

# The actual mail send
server = 'smtp.gmail.com:587'

def send_mail(send_from, send_to, subject, text, files=[], server="localhost"):
    assert type(send_to)==list
    assert type(files)==list

    msg = MIMEMultipart()
    msg['From'] = send_from
    msg['To'] = COMMASPACE.join(send_to)
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = subject

    msg.attach( MIMEText(text) )

    for f in files:
        part = MIMEBase('application', "octet-stream")
        part.set_payload( open(f,"rb").read() )
        Encoders.encode_base64(part)
        part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(f))
        msg.attach(part)

    smtp = smtplib.SMTP(server)
    smtp.starttls()
    smtp.login(username,password)
    smtp.sendmail(send_from, send_to, msg.as_string())
    smtp.close()

send_mail(from_address, to_address, email_subject, email_body, [sys.argv[1]], server) #the first command line argument will be used as the image file name


```

now you have to customise the email settings

```python

#From address, to address, subject and message body
from_address    = 'FROM_ADDRESS@EMAIL.COM'
to_address      = ['YOUR_ADDRESS@EMAIL.COM']
email_subject   = 'Alert!!! Zombies!!! Ahead!!!'
email_body      = 'A non dead intruder has been detected and needs to be eliminated!'

# Credentials (if needed)
username = 'EMAIL_LOGIN'
password = 'EMAIL_PASSWORD'

# The actual mail send
server = 'smtp.gmail.com:587'

```

so change the from address, to address and so on to match your settings


Now you can upload the script to the Arduino Yún SD card via SFTP (or SCP, if you prefer), then open an SSH session to the Yún and test it typing

	$cd /mnt/sda1
	$python sendemail.py test.png

And in a few seconds you will receive an email with the picture attachment... amazing!

 

## Step 6

**Let's build the circuit!**

Don't worry, this is supereasy, there is nothing to solder, just assemble few parts

I used [this PIR sensor](http://www.makershed.com/product_p/mkpx6.htm), because it's reliable and easy to use.

Just follow this scheme

![image](http://i.imgur.com/DyqvJZbl.png)

We've connected the PIR sensor, and a LED that will turn on when the device detects some movement


## Step 7

**Now it's time to add the Arduino Sketch**

Copy this code to the Arduino IDE and upload it to your Yún


```c
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

```

After 10 seconds (calibration time) it will start working and making pictures!!!

This is the assembled project

![assembled](http://i.imgur.com/9Vi2xzEl.jpg)

All the files of this project are available in my GitHub account

...and check the [Youtube Video](https://www.youtube.com/watch?v=a-asHQZwfVU) again!

I really hope you enjoyed this project!

Cheers, Stefano




