#include <LedControl.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

const int rs = 2, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


int DIN = 12;
int CS =  11;
int CLK = 10;

LedControl lc=LedControl(DIN,CLK,CS,0);

byte nothing[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //use to clear matrix
byte line[8] = {0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x03};  //line we will be using in the game
byte secondrow[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00};
byte smile[8]=   {0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C}; //smiley face for the winner
byte menu[8] = {0x90,0x92,0x90,0x90,0xf2,0x92,0x92,0x92}; //output "HI" to the led matrix

byte currentline = line[7]; //use to compare 
byte current_seconddots = secondrow[7];

int row = 0;
int pos = 0;
int angle = 0;
int score = 0;
int address = 0;
int secondaddress = 1;
int value = 0;
int secondvalue = 0;
int secondscore = 0;


const int stackButton = 3;
const int resetButton = 5;
const int changelvlButton = 4;



int lvlcount = 2;


bool winner = false;
bool secondWinner = false;
bool onedot = false;
bool endpart = false;
bool lvl2_in = true;
bool lvl3_in = false;




typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
    
} task;

int delay_gcd;
const unsigned short tasksNum = 2;
task tasks[tasksNum];


void clearMatrix() {
  for(int i = 0; i <= 8; i++){
    write(i, 0x00);
  }

}

void write(byte temper, byte sum){
  digitalWrite(CS, LOW);

  masking(temper);
  masking(sum);

  digitalWrite(CS, LOW);
  digitalWrite(CS, HIGH);
}

void masking(byte temp){
  byte val = 8;
  byte mask;

  while(val > 0) {
    mask = 0x01 << (i - 1);

    digitalWrite(CLK, LOW);
    digitalWrite(13, HIGH);
    if(temp & mask) {
      digitalWrite(DIN, HIGH);
    }
    else{
      digitalWrite(DIN, LOW);
    }
    digitalWrite(CLK, HIGH);
    digitalWrite(13, LOW);
    val--;
  }
}

enum Stack_States {Stack_Init, Stack_Level1, Stack_Level2, Stack_Level3, Stack_Level4, Stack_reset};
int Stack_Tick(int state) {
   switch(state){
      case Stack_Init:
        if(digitalRead(stackButton) == HIGH) {
          state = Stack_Level1;
        }
        else if(digitalRead(changelvlButton) == HIGH) {
          state = Stack_Level3;
        }
        else if((digitalRead(stackButton) == HIGH) && (digitalRead(changelvlButton) == HIGH)) {
            state = Stack_reset; 
         }
        else{
          state = Stack_Init;
        }
      break;

      case Stack_Level1:
         if(digitalRead(resetButton) == LOW) {
            clearMatrix();
            state = Stack_Init;
         }
         else if(winner == true) {
            state = Stack_Level2;
         }
         else if(digitalRead(changelvlButton) == HIGH) {
            state = Stack_Level3;
         }
         else if((digitalRead(stackButton) == HIGH) && (digitalRead(changelvlButton) == HIGH)) {
            state = Stack_reset; 
         }

         else{
            state = Stack_Level1;
         }
      break;

      case Stack_Level2:
         if(digitalRead(resetButton) == LOW) {
            lcd.clear();
            state = Stack_Init;
         }
         else if(digitalRead(stackButton) == HIGH) {
            lcd.clear();
            endGame();
            score = 0;
            winner = false;
            state = Stack_Level1;
         }
         else if(digitalRead(changelvlButton) == HIGH) {
             lcd.clear();
             lcd.print("LEVEL 2");          
             state = Stack_Level3;
         }
         else{
            state = Stack_Level2;
         }
       break;

       case Stack_Level3:
            if(digitalRead(stackButton) == HIGH) {
               lcd.clear();
               state = Stack_Level4;
            }
            else if(digitalRead(changelvlButton) == HIGH) {
              state = Stack_Level1;
            }
       break;

       case Stack_Level4:
            if(digitalRead(resetButton) == LOW) {
              state = Stack_Init;
            }
            else if(digitalRead(changelvlButton) == HIGH) {
              state = Stack_Level1;
            }
        break;

        case Stack_reset:
            break;
   }

   switch(state) {
      case Stack_Init:
        lcd.clear();
        value = EEPROM.read(address);
        printByte(menu);
        lcd.print("HELLO PLAYER!");
        row = 0;
        break;

      case Stack_Level1:
        if(angle == 0 && onedot == false) {
          pos++;
          if(pos == 6) {
            angle = 1;
           }
        }
        else if(angle == 1) {
          pos--;
          if(pos == 0) {
            angle = 0;
          }
        }
        else if(angle == 0 && onedot == true) {
          pos++;
          if(pos == 7) {
            angle = 1;
          }
        }
        line[row] = (currentline << pos); //change the position of the dots

        for(int i = 0; i <= 7; i++) {
          write(i+1, line[i]);
        }
        if(digitalRead(stackButton) == HIGH) {
          gameStart();
        }
        
        break;

      case Stack_Level2:
        score = 8;
        value = score;
        EEPROM.update(address, value);
        lcd.clear();
        lcd.print("YOU WON:)");
        printByte(smile);
        

        break;

      case Stack_Level3:

          
       break;

       case Stack_Level4:
          break;

       case Stack_reset:
          value = 0;
          secondvalue = 0;
          EEPROM.update(address, value);
          EEPROM.update(secondaddress, secondvalue);
          break;
   }
   return state;
   
}

enum Lvl2_States{Lvl2_Init, Lvl2_game, Lvl2_win};
int Lvl2_Tick(int state) {
  switch(state) {
    case Lvl2_Init:
      if((digitalRead(changelvlButton)) == HIGH &&(digitalRead(stackButton) == LOW) ) {
        score = 0;
        state = Lvl2_game;
      }
     break;

     case Lvl2_game:
      if(digitalRead(resetButton) == LOW) {
        state = Lvl2_Init;
      }
      else if(secondWinner == true) {
         state = Lvl2_win;
      }
      else if(digitalRead(changelvlButton) == HIGH) {
         state = Lvl2_Init;
      }
       break;

     case Lvl2_win:
     break;
  }

  switch(state) {
    case Lvl2_Init:
      break;

    case Lvl2_game:
        if(angle == 0 && onedot == false) {
          pos++;
          if(pos == 6) {
            angle = 1;
           }
        }
        else if(angle == 1) {
          pos--;
          if(pos == 0) {
            angle = 0;
          }
        }
        else if(angle == 0 && onedot == true) {
          pos++;
          if(pos == 7) {
            angle = 1;
          }
        }
        line[row] = (currentline << pos); //change the position of the dots

        for(int i = 0; i <= 7; i++) {
          write(i+1, line[i]);
        }
       if(digitalRead(stackButton) == HIGH) {
          
         secondLevel();
       }
      break;

    case Lvl2_win:
        secondscore = 8;
        secondvalue = score;
        EEPROM.update(secondaddress, secondvalue);
        lcd.clear();
        lcd.print("YOU WON:)");
        printByte(smile);
       
       
  }
  return state;
}

void gameStart() {
    delay(150);
    if(row >= 0) {
       if(row < 7) {
          if((line[row] & line[row+1]) > 0){
            winner = true;
            endGame();
            return;
          }
          row++;
          line[row] = line[row - 1] & line[row];
          currentline = ((line[row] & line[row - 1]) >> pos);
          checkForOne();
        
          if(row == 3 && onedot == false){
            currentline = 0x01;
            pos = 0;
            angle = 0;
            onedot == true;
          }

        row--;
      }
      else{
        endGame();
        delay(1000);
      }
    }
   else{
     row--''
  }
  lcd.clear();
  score++;
  lcd.print("LEVEL 1");
  lcd.setCursor(0,1);
  lcd.print("Score ");
  lcd.print(score);
}

void endGame() {
  if(score > value) {
    value = score;
    EEPROM.update(address,value);
  }
  EEPROM.write(address, value);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("High score: ");
  lcd.print(value);
  clearMatrix();
  line[7] = 0x03;
  pos = 0;
  angle = 0;
  row = 0;
  currentline = line[7];
  onedot = false;
  score = -1;
}

void secondLevel() {
    delay(150);
    if(row >= 0) {
       if(row < 7) {
          if((line[row] & line[row+1]) > 0){
            winner = true;
            endlvltwoGame();
            return;
          }
          row++;
          line[row] = line[row - 1] & line[row];
          currentline = ((line[row] & line[row - 1]) >> pos);
          checkForOne();
        
          if(row == 3 && onedot == false){
            currentline = 0x01;
            pos = 0;
            angle = 0;
            onedot == true;
          }

        row--;
      }
      else{
        endlvltwoGame();
        delay(1000);
      }
    }
   else{
     row--''
  }
  lcd.clear();
  secondscore++;
  lcd.print("LEVEL 2");
  lcd.setCursor(0,1);
  lcd.print("Score ");
  lcd.print(secondscore);
}


void endlvltwoGame(){
  if(secondscore > secondvalue) {
    secondvalue = secondscore;
    EEPROM.update(secondaddress,secondvalue);
  }
  EEPROM.write(secondaddress, secondvalue);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("High score: ");
  lcd.print(secondvalue);    
  clearMatrix();
  line[7] = 0x03;
  pos = 0;
  angle = 0;
  row = 0;
  currentline = line[7];
  onedot = false;
  secondscore = -1;  
}


void checkForOne() {
  int count = 0;
  int shift;
  for(int i = 0; i <= 7; i++) {
    shift = 0x01 << i;
    if(shift & currentline > 0) {
      count++;
      }
   }
  
  if(count > 1) {
    onedot = false;
  }
  else{
     onedot = true;
     currentline = 0x01;
  }
}



void printByte(byte character [])
{
  int i = 0;
  for(i=0;i<=7;i++)
  {
    lc.setRow(0,i,character[i]);
  }
}



void setup() {
  // put your setup code here, to run once:
  pinMode(stackButton, INPUT);
  pinMode(resetButton, INPUT_PULLUP);
  pinMode(changelvlButton, INPUT);
  pinMode(DIN, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(CLK, OUTPUT);


  lc.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
  lc.setIntensity(0,15);      // Set the brightness to maximum value
  lc.clearDisplay(0);         // and clear the display

  lcd.begin(16,2);
  
  unsigned char i = 0;
  tasks[i].state = Stack_Init;
  tasks[i].period = 150;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Stack_Tick;
  i++;
  tasks[i].state = Lvl2_Init;
  tasks[i].period = 50;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &Lvl2_Tick;



  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) {
     if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = millis(); // Last time this task was ran
      
     }
   }
  

}