// C64 USB Keyboard mod 
// Original by Pyofer 
// See original thread @ 
// http://www.lemon64.com/forum/viewtopic.php?t=55650 
// 
// Modified to Support restore key & US/EU keymaps by dabone. 
//
// Joystick support for keycodes added, and support for second fire button.
//
// Wiring is as follows 
// 64 Keyboard connector has 20 Pins with a key Pin @ Pin 2. 
// Arduino Pro Micro Pinout 
// https://cdn.sparkfun.com/assets/9/c/3/c/4/523a1765757b7f5c6e8b4567.png 
// Board type should be Arduino Leonardo (or the Pro mini clones) 
//
// Modified by YellPeril for the following :
//  + Support Arduino Micro
//  + Do not use RX and TX pins
//  + Support C128 Keyboard ( remove joystick and alternate keyboard code )
//  + Pressing "Help" key will switch + and = keys
//
// WARNING: Uploading this sketch makes the Arduino non responsive to re-uploading.
//          To re-upload : Open and compile the example blink sketch
//                         Reset the Arduino
//                         Upload the sketch
//          Now you can reupload the keyboard sketch.
//


//KeyBoard Arduino Pro Micro 			Micro
//  Pin     Pin     Label         Label
//   25                           SS / D17
//   24                           12
//   23                           13
//   22                           A4 / D22
//   21                           A5 / D23
//   20       5       2 - SDA		  2
//   19       6       3 - SCL		  3
//   18       7       4 - A6		  4
//   17       8       5 -			    5
//   16       9       6 - A7		  6
//   15       10      7 - 			  7	
//   14       11      8 - A8 		  8
//   13       12      9 - A9 		  9
//   12       13      10 - A10 		10
//   11       14      16 - MOSI 	MOSI /D16
//   10       15      14 - MISO 	MISO / D14
//   9        16      15 - SCLK 	SCK / D15
//   8        17      18 - A0 		A0 / D18
//   7        18      19 - A1 		A1 / D19
//   6        19      20 - A2 		A2 / D20		
//   5        20      21 - A3 		A3 / D21		
//   4        N/C
//   3        1       1 - TX		  11		Change 1 to 11. ( try not to use TX and RX pins if we can help it ).
//   2        N/C
//   1        11      8 - A8 		  8
//					  0						26		  12    Change 0 to 12. ( try not to use TX and RX pins if we can help it ). Missing line added from Joystick.


// The following pins are optional and not required for keyboard only usage.

//   Joysticks
//  Pins 1,2,3,4,6 Are Both Joysticks to the same input.
//   1        5       2 - SDA
//   2        6       3 - SCL
//   3        7       4 - A6
//   4        8       5
//   6        9       6 - A7
//   9        10      7 - 
//  Joy 1
//   8        1       1 - TX
//  Joy 2
//   8        2       0 - RX


// Keyboard Matrix Now Matches real C64 with one more column.
// Matrix can be found at 
// http://sta.c64.org/cbm64kbdlay.html

// For Standard Keys See
// http://www.asciitable.com/
// 
// Arduino Keyboard Modifier Keys
// Key                  Decimal value
// KEY_LEFT_CTRL            128
// KEY_LEFT_SHIFT           129
// KEY_LEFT_ALT             130
// KEY_LEFT_GUI             131
// KEY_RIGHT_CTRL           132
// KEY_RIGHT_SHIFT          133
// KEY_RIGHT_ALT            134
// KEY_RIGHT_GUI            135
// KEY_RETURN               176
// KEY_ESC                  177
// KEY_BACKSPACE            178
// KEY_TAB                  179
// KEY_CAPS_LOCK            193
// KEY_F1                   194
// KEY_F2                   195
// KEY_F3                   196
// KEY_F4                   197
// KEY_F5                   198
// KEY_F6                   199
// KEY_F7                   200
// KEY_F8                   201
// KEY_F9                   202
// KEY_F10                  203
// KEY_F11                  204
// KEY_F12                  205
// PrintScrn                206
// Scrlk                    207
// Pause                    208
// KEY_INSERT               209
// KEY_HOME                 210
// KEY_PAGE_UP              211
// KEY_END                  213
// KEY_DELETE               212
// KEY_PAGE_DOWN            214
// KEY_RIGHT_ARROW          215
// KEY_LEFT_ARROW           216
// KEY_DOWN_ARROW           217
// KEY_UP_ARROW             218
// Keypad /                 220
// Keypad *                 221
// Keypad -                 222
// Keypad +                 223
// Keypad ENTER             224
// Keypad 1 & End           225
// Keypad 2 & Down Arrow    226
// Keypad 3 & PageDn        227
// Keypad 4 & Left Arrow    228
// Keypad 5                 229
// Keypad 6 & Right Arrow   230
// Keypad 7 & Home          231
// Keypad 8 & Up Arrow      232
// Keypad 9 & PageUp        233
// Keypad 0 & Insert        234
// Keypad . & Delete        235

#include <HID.h> 
#include <Keyboard.h> 

int inChar=0;
int keyPos=0;
int keyDown[110];
long lastDebounceTime[110];
int debounceDelay=50;
int shift=0;
int Row=0;
int i;
char * keyMap;
int windowsShift;
int DefaultKBMode=0;                                  // Select 0 For Windows Mode On startup or 1 for C64 Mode
int USKeyboard=1;                                     // Select 1 For MiSTer C64 Keyboard
int HybridKeyboard=1;                                 // Select 0 for normal or 1 for the left HybridKey allowing all f keys and cursor keys in windows mode. (Also has a shifted restore key)
int HybridKey=17;                                     // Position of the Hybrid Select key in the keymap, Left Shift = 17, Right Shift = 64
int HybridKey2=72;                                    // Position of the Hybrid2 Select key in the keymap, CTRL = 72
//                                                   A5
//                   13, 19, 18, 17, 16, 15, 14, 20, 21, 22, 23 
int RowPinMap[11] = { 9,  3,  4,  5,  6,  7,  8,  2, A5, A4, 13};                       // Convert Row number to associated output pin
//int RowPinMap[11] = { 9,  3,  4,  5,  6,  7,  8,  2, A5};                       // Convert Row number to associated output pin


//                   12,   11,   10,  5,  8   7   6,   9,  1, NC   
int ColPinMap[10] = {10, MOSI, MISO, A3, A0, A1, A2, SCK, 11, 12};        // Convert Col number to associated input pin

char keyMapUS128[110]={
// US Keymaps                                         // Joystick Keys are the indicated by J(Direction) then number, i.e JU1 is up on joystick one.
                                                      // Firebuttons are Noted JF(Joystick number)- Fire Button number, i.e JF2-1 Joystick 2, fire button 1, JF2-2 Joystick 2, fire button 2.
                                                      // Current values for the Joysticks are located at the end of each line. KP stands for numeric keypad inputs.
                 
// >> MiSTer << C128 Keyboard with busted + = swap???
// This keyset is passed to the computer along with the shift key.

                                                      //      [12]  [11]   [10] [5]  [8]   [7] [6] [9]        [3]         [24]     [25]     NC = [2] [4]
178, 176, 215, 200, 194, 196, 198, 217,   0,   0,     //[13]  Del   Return LR   F7   F1    F3  F5  UD      
 51, 119,  97,  52, 122, 115, 101, 129,   0,   0,     //[19]  3     W      A    4    Z     S   E   LSHFT   
 53, 114, 100,  54,  99, 102, 116, 120,   0,   0,     //[18]  5     R      D    6    C     F   T   X       
 55, 121, 103,  56,  98, 104, 117, 118,   0,   0,     //[17]  7     Y      G    8    B     H   U   V       
 57, 105, 106,  48, 109, 107, 111, 110,   0,   0,     //[16]  9     I      J    Zero M     K   O   N       
 61, 112, 108,  45,  46,  59,  91,  44,   0,   0,     //[15]  +     P      L    -    .     :   @   ,       
202,  93,  39, 210, 133, 203,  92,  47, 204,   0,     //[14]  Pound *      ;    Home RSHFT =   Pi  /           Restore 
 49, 126, 128,  50,  32, 130, 113, 177,   0,   0,     //[20]  1     BS     CTRL 2    SPC   C=  Q   RunStop 
204, 232, 229, 179, 226, 228, 231, 225,   0,   0,     //[21]  Help  kp8    kp5  Tab  kp2   kp4 kp7 kp1                                                        
177,  61, 222,  10, 224, 230, 233, 227,   0,   0,     //[22]  Esc   kp+    kp-  LnFd Enter kp6 kp9 kp3                   
130, 234, 235, 218, 217, 216, 215, 207,   0,   0,     //[23]  Alt   kp0    kp.  aUP  aDN   aL  aR  NoScroll      
                                                      //[ 1]                                                   [RESTORE] [40/80] [CAPS]
};

char keyMapUSstd[110]={
// US Keymaps                                         // Joystick Keys are the indicated by J(Direction) then number, i.e JU1 is up on joystick one.
                                                      // Firebuttons are Noted JF(Joystick number)- Fire Button number, i.e JF2-1 Joystick 2, fire button 1, JF2-2 Joystick 2, fire button 2.
                                                      // Current values for the Joysticks are located at the end of each line. KP stands for numeric keypad inputs.
                 
// >> MiSTer << C128 Keyboard normal
// This keyset is passed to the computer along with the shift key.

                                                      //      [12]  [11]   [10] [5]  [8]   [7] [6] [9]        [3]         [24]     [25]     NC = [2] [4]
178, 176, 215, 200, 194, 196, 198, 217,   0,   0,     //[13]  Del   Return LR   F7   F1    F3  F5  UD      
 51, 119,  97,  52, 122, 115, 101, 129,   0,   0,     //[19]  3     W      A    4    Z     S   E   LSHFT   
 53, 114, 100,  54,  99, 102, 116, 120,   0,   0,     //[18]  5     R      D    6    C     F   T   X       
 55, 121, 103,  56,  98, 104, 117, 118,   0,   0,     //[17]  7     Y      G    8    B     H   U   V       
 57, 105, 106,  48, 109, 107, 111, 110,   0,   0,     //[16]  9     I      J    Zero M     K   O   N       
203, 112, 108,  45,  46,  59,  91,  44,   0,   0,     //[15]  +     P      L    -    .     :   @   ,       
202,  93,  39, 210, 133,  61,  92,  47, 204,   0,     //[14]  Pound *      ;    Home RSHFT =   Pi  /           Restore 
 49, 126, 128,  50,  32, 130, 113, 177,   0,   0,     //[20]  1     BS     CTRL 2    SPC   C=  Q   RunStop 
204, 232, 229, 179, 226, 228, 231, 225,   0,   0,     //[21]  Help  kp8    kp5  Tab  kp2   kp4 kp7 kp1                                                        
177, 203, 222,  10, 224, 230, 233, 227,   0,   0,     //[22]  Esc   kp+    kp-  LnFd Enter kp6 kp9 kp3                   
130, 234, 235, 218, 217, 216, 215, 207,   0,   0,     //[23]  Alt   kp0    kp.  aUP  aDN   aL  aR  NoScroll      
                                                      //[ 1]                                                   [RESTORE] [40/80] [CAPS]
};

char Hybridkeys[7]{
                                                      // Hybrid Keys. These are the shifted values.  
                                                      // These allow keys to be passed in windows mode that are not shifted values of the original key. 
                                                      // This allows the windows mode to use the cursor keys and F1-F8. The cursor keys and F keys are default, The restore key is F10.
216, 201, 195, 197, 199, 218, 205,                    // LR F8 F2 F4 F6 UD Restore
};

char Hybridkeys2[5]{
                                                      // Hybrid Keys2. These are the shifted values.  Defaults to the CTRL Key
                                                      // These allow keys to be passed in windows mode that are not shifted values of the original key. 
                                                      // Page Down, F11, `, Page UP, F9 Mapped to F7,F1,F3,F8 and Restore
214, 204, 96, 211, 202,
};

void setup() 
{
  Keyboard.begin();// initialize control over the keyboard:

  for (i=0; i<110; i++) keyDown[i]=0; // Set all keys as up
  
  // Old code mapped row pins to OUTPUT, set Row on pin 2 LOW and 
  // all of the rest HIGH, but then turned it right around and 
  // made them all INPUT during the first call to loop().  Just 
  // make them all INPUT and keep them HIGH with internal pullups, 
  // since they were set to HIGH after scanning a Row anyway.
  for (int Row=0; Row<11; Row++)
   pinMode(RowPinMap[Row], INPUT_PULLUP); 

  for (int Col=0; Col<10; Col++)
    pinMode(ColPinMap[Col],INPUT_PULLUP); // use internal pullups to hold pins high

  if (DefaultKBMode==1)
  {  
    if (!digitalRead(10)) windowsShift=1; else windowsShift=0; // detect if '1' is held on power up to swap mode
  }
  if (DefaultKBMode==0)
  {
    if (!digitalRead(10)) windowsShift=0; else windowsShift=1; // detect if '1' is held on power up to swap mod
  }
  if (USKeyboard == 1)
    {keyMap = keyMapUS128;}
  else
    {keyMap = keyMapUS128;}
}


void loop() // main keyboard scanning loop
{
  int InputValue; // Replaces digitalread, so that global variable can be removed
  int RowPin;     // Replaces rowPinSet, so that global variable can be removed

  for (int Row=0; Row<11; Row++) // scan through all rows
  {
    RowPin = RowPinMap[Row];  // Map logical row to output pin
    pinMode(RowPin,OUTPUT);   // Set output pin to OUTPUT
    digitalWrite(RowPin,LOW); // Set output pin to LOW
    
    for (int Col=0; Col<10; Col++) // scan through columns
    {
      keyPos=Col+(Row*10); // calculate character map position
      
      if (!windowsShift) inChar=keyMap[keyPos+shift]; // work out which key it is from the map and shift if needed
      else inChar=keyMap[keyPos];  // use "windows" keymap where shift is passed through

      InputValue = digitalRead(ColPinMap[Col]); // LOW = Key pressed, HIGH = Key Not Pressed

      if ((millis()-lastDebounceTime[keyPos])>debounceDelay) // debounce for each key individually
      {
        if ((InputValue == LOW) && keyDown[keyPos]==0) // if a key is pressed and wasn't already down
        {
          keyDown[keyPos]=inChar;        // put the right character in the keydown array
          if (HybridKeyboard==1)
          {
            if (keyDown[HybridKey]&&keyDown[2])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[0];}
            if (keyDown[HybridKey]&&keyDown[3])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[1];}
            if (keyDown[HybridKey]&&keyDown[4])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[2];}
            if (keyDown[HybridKey]&&keyDown[5])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[3];}
            if (keyDown[HybridKey]&&keyDown[6])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[4];}
            if (keyDown[HybridKey]&&keyDown[7])  {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[5];}
            if (keyDown[HybridKey]&&keyDown[68]) {Keyboard.release (keyDown[HybridKey]);keyDown[keyPos]=Hybridkeys[6];}

            // Don't do this it breaks the keyboard for Ctrl - Fx combinations.
            //if (keyDown[HybridKey2]&&keyDown[3])  {Keyboard.release (keyDown[HybridKey2]);keyDown[keyPos]=Hybridkeys2[0];}
            //if (keyDown[HybridKey2]&&keyDown[4])  {Keyboard.release (keyDown[HybridKey2]);keyDown[keyPos]=Hybridkeys2[1];}
            //if (keyDown[HybridKey2]&&keyDown[5])  {Keyboard.release (keyDown[HybridKey2]);keyDown[keyPos]=Hybridkeys2[2];}
            //if (keyDown[HybridKey2]&&keyDown[6])  {Keyboard.release (keyDown[HybridKey2]);keyDown[keyPos]=Hybridkeys2[3];}
            //if (keyDown[HybridKey2]&&keyDown[68]) {Keyboard.release (keyDown[HybridKey2]);keyDown[keyPos]=Hybridkeys2[4];}
          }
          
          if ((keyPos!=17&&keyPos!=64)||windowsShift==1)// is it not-shift or in windows mode?
          {  // if so pass the key through
            lastDebounceTime[keyPos] = millis(); // reset the debounce delay
            Keyboard.press(keyDown[keyPos]);    // pass the keypress to windows
          }
          else { lastDebounceTime[keyPos]=millis(); shift=80; } // reset keybounce delay and mark as shift press
        }
        if ((InputValue == HIGH) && keyDown[keyPos]!=0) // key is up and a character is stored in the keydown position
        {
          if ((keyPos!=17&&keyPos!=64)||windowsShift==1) // not-shift or windows mode
          {
            lastDebounceTime[keyPos] = millis();  // reset keybounce delay
            Keyboard.release(keyDown[keyPos]);    // pass key release to windows
          }
          else { lastDebounceTime[keyPos]=millis(); shift=0; } // reset keybounce delay and mark as un-shifted
          keyDown[keyPos]=0; // set keydown array position as up

          // Press the HELP Key to swap + and =
          if (keyPos==80&&USKeyboard==1)
          {
              keyMap = keyMapUSstd;
              USKeyboard = 0;
          } else if (keyPos==80&&USKeyboard==0)
          {
              keyMap = keyMapUS128;
              USKeyboard = 1;
          }

        }
      }
    }
    digitalWrite(RowPin,HIGH);    // Set output pin to HIGH
    delay(2);                     // Delay to make sure it has time to go HIGH before switching to INPUT
    pinMode(RowPin,INPUT_PULLUP); // Set output pin back to INPUT with pullup to make sure it stays HIGH
  }
}
