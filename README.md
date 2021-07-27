# C64USBKeyboard2
A hack of Pyrofers C64USBKeyboard to support a C128 keyboard on MiSTer

Modified by YellPeril for the following :
 + Support Arduino Micro
 + Do not use RX and TX pins
 + Support C128 Keyboard ( remove joystick and alternate keyboard code )
 + Pressing "Help" key will switch + and = keys

WARNING: Uploading this sketch makes the Arduino non responsive to re-uploading.
          To re-upload : Open and compile the example blink sketch
                         Reset the Arduino
                         Upload the sketch
          Now you can reupload the keyboard sketch.
