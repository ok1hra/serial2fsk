#include <Arduino.h>

/*

  Serial to (A)FSK keying for Arduino
  --------------------------------------------
  Based on BAUDOT TELEMETRY BEACON by JI3BNB
  http://k183.bake-neko.net/ji3bnb/page13.html

  http://remoteqth.com/serial2fsk.php
  2015-12 by OK1HRA

  ___               _        ___ _____ _  _                
 | _ \___ _ __  ___| |_ ___ / _ \_   _| || |  __ ___ _ __  
 |   / -_) '  \/ _ \  _/ -_) (_) || | | __ |_/ _/ _ \ '  \ 
 |_|_\___|_|_|_\___/\__\___|\__\_\|_| |_||_(_)__\___/_|_|_|
                                                           

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
Features:
- FSK (preset baud rate) and AFSK output
- PTT with preset lead and tail time
- serial receive ascii, and '\n' as LF and '\r' as CR
- LCD show Shift to figures/letters

*/

#include <stdio.h>
#include <LiquidCrystal.h>
////////////////////////////////////////////////////////////////////////////
#define AFSK_OUT 4                       // AFSK AUDIO pin OUTPUT
#define FSK_OUT  12                      // TTL LEVEL pin OUTPUT
#define PTT      13                      // PTT pin OUTPUT
#define MARK     1445                    // AFSK mark 1445 / 2295 Hz
#define SPACE    1275                    // AFSK space 1275 / 2125 Hz
#define FMARK    HIGH                    // FSK mark level [LOW/HIGH]
#define FSPACE   LOW                     // FSK space level [LOW/HIGH]
#define BaudRate 45.45                   // RTTY baud rate
#define StopBit  1.5                     // stop bit long
#define PTTlead  400                     // PTT lead delay ms
#define PTTtail  200                     // PTT tail delay ms
#define SERBAUD  9600                    // Serial port in/out baudrate
#define serialECHO                       // enable TX echo on serial port
LiquidCrystal lcd(A2, 10, 6, 7, 8, 9);   // LCD pins
////////////////////////////////////////////////////////////////////////////

int     OneBit = 1/BaudRate*1000;
boolean d1;
boolean d2;
boolean d3;
boolean d4;
boolean d5;
boolean space;
boolean fig1;
int     fig2;
char    ch;
int     r2;
int     positionCounter;
byte LFi[8] = {0b10111,0b10111,0b10001,0b11111,0b10001,0b10111,0b10011,0b10111};
byte CRi[8] = {0b11001,0b10111,0b10111,0b11001,0b11111,0b10001,0b10111,0b10111};
byte UPi[8] = {0b11111,0b11111,0b11011,0b10001,0b00100,0b01110,0b11111,0b11111};
byte DWNi[8] = {0b11111,0b11111,0b01110,0b00100,0b10001,0b11011,0b11111,0b11111};

void setup()
{
        pinMode(PTT,  OUTPUT);
        pinMode(FSK_OUT,  OUTPUT);
        pinMode(AFSK_OUT, OUTPUT);
        digitalWrite(FSK_OUT, LOW);
        delay(700);
        lcd.begin(16, 2);
        lcd.createChar(0, CRi);
        lcd.createChar(1, LFi);
        lcd.createChar(2, UPi);
        lcd.createChar(3, DWNi);
        Serial.begin(SERBAUD);
}

void sendFsk()
{
        #if defined(serialECHO )
              Serial.print(d1);Serial.print(d2);Serial.print(d3);Serial.print(d4);Serial.print(d5);Serial.print(' '); // 5bit code serial echo
        //      Serial.print(OneBit);Serial.print('|');Serial.print(OneBit*StopBit);Serial.print(' ');                  // ms
        #endif
        //--start bit
        digitalWrite(FSK_OUT, FSPACE); tone(AFSK_OUT, SPACE); delay(OneBit);
        //--bit1
        if(d1 == 1){digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); }
        else       {digitalWrite(FSK_OUT, FSPACE);  tone(AFSK_OUT, SPACE);} delay(OneBit);
        //--bit2
        if(d2 == 1){digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); }
        else       {digitalWrite(FSK_OUT, FSPACE);  tone(AFSK_OUT, SPACE);} delay(OneBit);
        //--bit3
        if(d3 == 1){digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); }
        else       {digitalWrite(FSK_OUT, FSPACE);  tone(AFSK_OUT, SPACE);} delay(OneBit);
        //--bit4
        if(d4 == 1){digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); }
        else       {digitalWrite(FSK_OUT, FSPACE);  tone(AFSK_OUT, SPACE);} delay(OneBit);
        //--bit5
        if(d5 == 1){digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); }
        else       {digitalWrite(FSK_OUT, FSPACE);  tone(AFSK_OUT, SPACE);} delay(OneBit);
        //--stop bit
        digitalWrite(FSK_OUT, FMARK); tone(AFSK_OUT, MARK); delay(OneBit*StopBit);
}

void chTable()
{
        fig2 = -1;
        if(ch == ' ')
        {
                d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 0;
                space = 1;
        }
        else if(ch == 'A'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'B'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'C'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'D'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'E'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'F'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'G'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'H'){d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'I'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'J'){d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'K'){d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'L'){d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'M'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'N'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'O'){d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'P'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'Q'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'R'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'S'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'T'){d1 = 0; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'U'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'V'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'W'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'X'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'Y'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'Z'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == '0'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '1'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '2'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '3'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '4'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '5'){d1 = 0; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '6'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '7'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '8'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '9'){d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == '-'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '?'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == ':'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '('){d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == ')'){d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '.'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == ','){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '/'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == '+'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;} //ITA2
        else if(ch == '\n'){d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 0;} //LF
        else if(ch == '\r'){d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 0;} //CR
        else
        {
                ch = ' ';
                d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 0;
                space = 1;
        }
}

void loop(){
    if (Serial.available()) {
        fig1 = 1;                         // every shift to start message 
        positionCounter=0;
        lcd.clear();
        lcd.setCursor(0, 1);
        tone(AFSK_OUT, MARK);             // AFSK ON
        digitalWrite(PTT, HIGH);          // PTT ON
        delay(PTTlead);                   // PTT lead delay
        // ch = ' '; Serial.print(ch); chTable(); sendFsk();   // Space before sending
        while (Serial.available()) {
            positionCounter++;
            if (positionCounter > 15){
                lcd.scrollDisplayLeft();
            }
            ch = toUpperCase(Serial.read());
            chTable();
    
            if(fig1 == 0 && fig2 == 1){
                    d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 1; //FIGURES
                    lcd.write(byte(2));                     // UP LCD char
                    sendFsk();
            }else if(fig1 == 1 && fig2 == 0){
                    d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 1; //LETTERS
                    lcd.write(byte(3));                     // DWN LCD char
                    sendFsk();
            }else if(space == 1 && fig2 == 1){
                    d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 1; //FIGURES 
                    lcd.write(byte(2));                     // UP LCD char
                    sendFsk();
            }
            
            if(fig2 == 0 || fig2 == 1){
                    space = 0;
                    fig1 = fig2;
            }
            
            if (ch == '\r'){              // CR LCD char
                      lcd.write(byte(0));
            }else if(ch == '\n'){         // LF LCD char
                      lcd.write(byte(1));
            }else{
                      lcd.print(ch);
            }
            chTable();
            sendFsk();
            delay(5);
    
        }
        // ch = ' '; Serial.print(ch); chTable(); sendFsk();   // Space after sending
        delay(PTTtail);
        digitalWrite(PTT, LOW);Serial.println();
        digitalWrite(FSK_OUT, LOW);
        noTone(AFSK_OUT);
      }
}
