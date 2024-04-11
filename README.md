Contents of the Repository:
1. .ino files - to run keyword spotting on the corresponding microcontrollers
2. MODEL LIBRARIES - folder containing the libraries generated from Edge Impulse 
3. bfd26d6b_nohash_0.wav - a sample wav file
4. test_output.pdm - a sample .pdm file

The microcontrollers needed:
1.  Arduino Nano 33 BLE Sense
2.  NodeMCU ESP32
3.  Raspberry Pi Pico

Dataset used:
(2018) Speech commands dataset version 2. [Online]. Available:
http://download.tensorflow.org/data/speech_commands_v0.02.tar.gz

Do the following steps to run the keyword spotting on the microcontrollers:
1.  Download Arduino IDE and the libraries needed for each microcontroller
2.  Download the libraries (Models) provided
3.  Include library in Arduino IDE (Sketch -> Include Library -> Add .ZIP Library...)
4.  Connect a microcontroller and run the provided code for that microcontroller
5.  Run the streamdatawav.py to stream the data into the microcontroller
6.  Observe output for results
