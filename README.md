# BlueFruit Autostart

Here is my work on a home built bluetooth autostart that uses a BlueFruit nRF52 board at it's core.

The entire build process, my thoughts, and actions are can be read here:
https://thealaskalinuxuser.wordpress.com/2018/05/15/bluefruit-project-vehicle-autostart/

![ScreenShot](https://github.com/alaskalinuxuser/BlueFruit_Autostart/blob/master/pictures/img_20180424_122308.jpg)

# Programming Folder

Here you will find the program sketch that I made in Arduino IDE to program the nRF52 Adafruit BlueFruit board. You will need to add the Adafruit nRF52 boards through the board manager in the IDE to utilize this sketch.


You will also find the Android program that I modified from Adafruit to make my autostart app. This is written for Android Studio 2.3. Newer versions of Android Studio will require some modification to the files.

# Video and Pictures Folders

Here you can see pictures throughout the process, as well as a video of the final result.

# Schematics Folder

In this folder is the original concept schematic for the autostart. There is also the schematics for the 1993 GMC K3500 pickup truck that I installed this in. Those drawings have been marked for which wires I connected to.

# Info Folder

This folder includes information on the Adafruit BlueFruit board I used. This is not my work, only shared here so one can see how the bluetooth board works.

# How it works

After programming the board and wiring it up per the schematics. It works as follows:

+ On the Android app, pair with the board to connect to it.
+ Press the start button to send the start signal to the board.
+ The board recognizes the start signal and initiates the start timer of 5 seconds, during which it turns on the relays for accessories, fuel pump, ignition, lights, etc.
+ After the 5 second wait, the board checks voltage on the battery (through a voltage divider) and that the brakes are not being pressed. It will then initiate the cranking relay for 3 seconds (this time can be adjusted).
+ It will crank until voltage comes up (due to starting the vehicle, alternator now running) or the crank time has expired.
+ The board will check the voltage to see if the alternator has increased the voltage, which will determine if the engine is running. If yes, it starts a 10 minute run timer, after which the vehicle will shut down. If no, then it will make up to 2 more start attempts.

At any time: 
+ Pressing the brakes will stop the process and turn off cranking relays, as well as accessory/ignition/lights/fuelpump relays.
+ Pressing the stop button on the Android app will turn off cranking relays, as well as accessory/ignition/lights/fuelpump relays.
+ Voltage that is above 13 volts will prevent further cranking.

Special conditions:
+ Run Timer of 10 minutes reaching 0 will turn off all relays.
+ 3 total failed start attempts will turn off all relays.
