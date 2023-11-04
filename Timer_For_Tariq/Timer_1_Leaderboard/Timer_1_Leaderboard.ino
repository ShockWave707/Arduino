#include "SparkFun_External_EEPROM.h"
#include <Wire.h>
#include <LiquidCrystal.h>

#define PUSH HIGH
#define NOPUSH LOW
#define EEPROM_ADDRESS 0x50 
#define ARRAY_LENGTH 6 //length of the 2 parallel arrays for holding labels and time

//EEPROM stuff
ExternalEEPROM myMem;

//global variables
  //button analog pins
const int buttonStart = 7;
const int buttonSelect = 8;
const int buttonUp = A0;
const int buttonDown = A1;
const int buttonAdd = A2;
const int buttonDelete = 10;

  //array setup
int timeArray[3] = {0}; //HR:MIN:SEC
unsigned int leaderboardArray[ARRAY_LENGTH] = {0}; //top 5 times, index 0 is smallest, index 5 is largest
char letterArray[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}; //Array to cycle through to name a time
char leaderboardChar[ARRAY_LENGTH] = {' ', ' ', ' ', ' ', ' ', ' '}; //parallel array to leaderboardArray to hold the label for each time, ' ' will be default

  //variables
int index = 0;
int contrast =  60;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int charIndex = 0;
int holdVar = 0;

//function to turn the seconds in leaderboardArray into hr:min:sec, holding data with timeArray
void secondConverter (unsigned int seconds, int timeArrayFunction[])
{
  timeArrayFunction[0] = ((seconds / 60) / 60);
  timeArrayFunction[1] = ((seconds - ((timeArrayFunction[0] * 60) * 60)) / 60);
  timeArrayFunction[2] = (seconds - (((timeArrayFunction[0] * 60) * 60) + (timeArrayFunction[1] * 60)));  
}

// a quick fix for the sorter when empty times are set to 0
void quickFix(unsigned int arrayLB[])
{
  for (int quickFixVar = 0; quickFixVar < 6; quickFixVar++)
  {
    if (arrayLB[quickFixVar] == 0)
    {
      arrayLB[quickFixVar] = 65535; //65535 is the max set by unsigned int limit
    } 
  }
}

//to undo the quick fix for display
void quickFixUndo(unsigned int arrayLBoard[])
{
  for (int quickFixUndo = 0; quickFixUndo < 6; quickFixUndo++)
  {
    if (arrayLBoard[quickFixUndo] == 65535)
    {
      arrayLBoard[quickFixUndo] = 0; //65535 is the max set by unsigned int limit
    }
  }
}

void sortArray(unsigned int arrayToSort[], char parallelCharArray[])
{
  for (unsigned int forOutControl = 1; forOutControl < ARRAY_LENGTH; ++forOutControl)
  {
    for (unsigned int forInControl = 0; forInControl < ARRAY_LENGTH - 1; ++forInControl)
    {
      if (arrayToSort[forInControl] > arrayToSort[forInControl + 1])
      {
        unsigned int hold = arrayToSort[forInControl];
        arrayToSort[forInControl] = arrayToSort[forInControl + 1];
        arrayToSort[forInControl + 1] = hold;

        char charHold = parallelCharArray[forInControl];
        parallelCharArray[forInControl] = parallelCharArray[forInControl + 1];
        parallelCharArray[forInControl + 1] = charHold;
      }
    }
  }
}

void writeLoop(unsigned int arrayToWrite[], char charArrayToWrite[])
{
  int controlIndex = 0;
  for (int writeLoopControl = 0; writeLoopControl < 60; writeLoopControl = writeLoopControl + 10)
  {
    myMem.put(writeLoopControl, arrayToWrite[controlIndex]);
    controlIndex++;
    delay(5);
  }

  controlIndex = 0;
  for (int charWriteLoopControl = 60; charWriteLoopControl < 120; charWriteLoopControl = charWriteLoopControl + 10)
  {
    myMem.put(charWriteLoopControl, charArrayToWrite[controlIndex]);
    controlIndex++;
    delay(5); 
  }
}

void readLoop(unsigned int arrayToRead[], char charArrayToRead[])
{
  int indexControl = 0;
  for (int loopControl = 0; loopControl < 60; loopControl = loopControl + 10)
  {
    myMem.get(loopControl, arrayToRead[indexControl]);
    indexControl++;
  }

  indexControl = 0;
  for (int charLoopControl = 60; charLoopControl < 120; charLoopControl = charLoopControl + 10)
  {
    myMem.get(charLoopControl, charArrayToRead[indexControl]);
    indexControl++; 
  }
}
void setup() 
{
  //Button Pin Setup
  pinMode(buttonStart, INPUT);
  pinMode(buttonSelect, INPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonAdd, INPUT);
  pinMode(buttonDelete, INPUT);

  //LCD Screen setup
  analogWrite(6, contrast);
  lcd.begin(16, 2);
  
  //EEPROM setup
  Wire.begin();

  //test for memory
  if (myMem.begin() == false)
  {
    lcd.setCursor(0, 0);
    lcd.print("   NO  MEMORY   ");
    lcd.setCursor(0, 1);
    lcd.print("    DETECTED    ");
    while (1)
      ;
  }

  //initialize display
  readLoop(leaderboardArray, leaderboardChar);
  secondConverter(leaderboardArray[0], timeArray);
}

void loop() 
{ 
  //initialize variables for each loop
  unsigned int seconds = 0;
  
  secondConverter(leaderboardArray[index], timeArray);
  
  //Display leaderboardArray[index] on LCD screen
  lcd.setCursor(0, 0);
  lcd.print(index + 1);
  lcd.setCursor(1, 0);
  lcd.print(")hr : min : sec");

  lcd.setCursor(0, 1);
  lcd.print("                ");  
  lcd.setCursor(0, 1);
  lcd.print(leaderboardChar[index]);
  lcd.setCursor(1, 1);
  lcd.print(")  ");
  lcd.setCursor(4, 1);
  lcd.print(timeArray[0]);
  lcd.setCursor(6, 1);
  lcd.print(" : ");
  lcd.setCursor(9, 1);
  lcd.print(timeArray[1]);
  lcd.setCursor(11, 1);
  lcd.print(" : ");
  lcd.setCursor(14, 1);
  lcd.print(timeArray[2]);



  //if statement to look for inputs
  if (digitalRead(buttonStart) == PUSH)
  {
    int tempCounter = 1;

    delay(1000); //delay to prevent stopping of timer just as it starts
    
    //while loop start
    while ((digitalRead(buttonStart) == NOPUSH))
    {
      //19 loops through temp counter at 50 ms each, on 20th loop, reset temp counter, add 1 to seconds
      if (tempCounter < 20)
      {
        tempCounter++;
      }

      else
      {
        tempCounter = 1;
        seconds++;
      }

      //Display seconds passed through secondConverter function
      secondConverter(seconds, timeArray);
      lcd.setCursor(0, 0);
      lcd.print("                ");    
      lcd.setCursor(0, 0);
      lcd.print("hr : min : sec");

      lcd.setCursor(0, 1);
      lcd.print("                "); 
      lcd.setCursor(4, 1);
      lcd.print(timeArray[0]);
      lcd.setCursor(6, 1);
      lcd.print(" : ");
      lcd.setCursor(9, 1);
      lcd.print(timeArray[1]);
      lcd.setCursor(11, 1);
      lcd.print(" : ");
      lcd.setCursor(14, 1);
      lcd.print(timeArray[2]);

      //delay to make seconds accurate
      delay (50);
    }

    quickFix(leaderboardArray);
    
    //if time recorded does not belong in top 5 (shortest) times
    if (seconds >= leaderboardArray[4])
    {
      //display message stating it was not in the top 5
      lcd.setCursor(0, 0);
      lcd.print("                "); 
      lcd.setCursor(0, 0);
      lcd.print("Not Faster Than");

      lcd.setCursor(0, 1);
      lcd.print("                "); 
      lcd.setCursor(0, 1);
      lcd.print("Top 5   ");
      lcd.setCursor(8, 1);
      lcd.print(timeArray[0]);
      lcd.setCursor(10, 1);
      lcd.print(":");
      lcd.setCursor(11, 1);
      lcd.print(timeArray[1]);
      lcd.setCursor(13, 1);
      lcd.print(":");
      lcd.setCursor(14, 1);
      lcd.print(timeArray[2]);

      delay(4000);
      
      quickFixUndo(leaderboardArray);
    }
  
    else
    { 
      secondConverter(seconds, timeArray);
      int whileVar = 0;
      while (whileVar != 1)
      { 
        lcd.setCursor(0, 0);
        lcd.print("                "); 
        lcd.setCursor(0, 0);
        lcd.print("ADD/DEL Time");

        lcd.setCursor(0, 1);
        lcd.print("                "); 
        lcd.setCursor(0, 1);
        lcd.print("        ");
        lcd.setCursor(8, 1);
        lcd.print(timeArray[0]);
        lcd.setCursor(10, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.print(timeArray[1]);
        lcd.setCursor(13, 1);
        lcd.print(":");
        lcd.setCursor(14, 1);
        lcd.print(timeArray[2]);        
        
        if (digitalRead(buttonAdd) != NOPUSH)
        { 
          //sort logic to add time and delete slowest time
          //quick fix to avoid issues with default times that are 0
          quickFix(leaderboardArray);

          //set the last index of array equal to the time, only 5 times will display
          //5th index in array is meant for sorting
          leaderboardArray[5] = seconds;

          charIndex = 0;
          //wait for input to change label
          while(digitalRead(buttonSelect) == NOPUSH)
          {
            //display time and label
            lcd.setCursor(0, 0);
            lcd.print("                "); 
            lcd.setCursor(0, 0);
            lcd.print("Label: ");
            lcd.setCursor(7, 0);
            lcd.print(letterArray[charIndex]);
            lcd.setCursor(8, 0);
            lcd.print("        ");

            lcd.setCursor(0, 1);
            lcd.print("                "); 
            lcd.setCursor(0, 1);
            lcd.print("    ");
            lcd.setCursor(4, 1);
            lcd.print(timeArray[0]);
            lcd.setCursor(6, 1);
            lcd.print(" : ");
            lcd.setCursor(9, 1);
            lcd.print(timeArray[1]);
            lcd.setCursor(11, 1);
            lcd.print(" : ");
            lcd.setCursor(14, 1);
            lcd.print(timeArray[2]);
      
            //go up through letters
            if (digitalRead(buttonUp) != NOPUSH)
            {
              if (charIndex != 25)
              {
                charIndex++;
              }
              else
              {
                charIndex = 0;
              }
              delay(250);
            }

            else if (digitalRead(buttonDown) != NOPUSH)
            {
              if (charIndex != 0)
              {
                charIndex = charIndex - 1;
              }
              else
              {
                charIndex = 25;
              }
              delay(250);
            }
            leaderboardChar[5] = letterArray[charIndex];
          } 
          
          
          sortArray(leaderboardArray, leaderboardChar);
          
          //undo to make the display look better
          quickFixUndo(leaderboardArray);
          
          whileVar = 1;
          writeLoop(leaderboardArray, leaderboardChar);
          delay(500);
        }
        
        //if delete button is pressed
        else if (digitalRead(buttonDelete) != NOPUSH)
        {
          //display "Time Discarded" on LCD screen
          lcd.setCursor(0, 0);
          lcd.print("                "); 
          lcd.setCursor(0, 0);
          lcd.print("Time Discarded");

          lcd.setCursor(0, 1);
          lcd.print("                "); 
          lcd.setCursor(4, 1);
          lcd.print(timeArray[0]);
          lcd.setCursor(6, 1);
          lcd.print(" : ");
          lcd.setCursor(9, 1);
          lcd.print(timeArray[1]);
          lcd.setCursor(11, 1);
          lcd.print(" : ");
          lcd.setCursor(14, 1);
          lcd.print(timeArray[2]);


          
          delay(4000);
          whileVar = 1;
        }
      } 
    }
  }


// if select button is pushed
  else if (digitalRead(buttonSelect) == PUSH)
  {
    delay(500);
    
    //to set the displayed label to the label currently being used 
    //must be outside of loop to allow up and down button to function
    charIndex = 0;
    while ((letterArray[charIndex] != leaderboardChar[index]) && (leaderboardChar[index] != 'x'))
    {
      charIndex++;
    }
    
    //while loop to wait for input
    int whileControlVar = 0;
    while (whileControlVar != 1)
    { 
      lcd.setCursor(0, 0);
      lcd.print("                "); 
      lcd.setCursor(0, 0);
      lcd.print("Label: ");
      lcd.setCursor(7, 0);
      lcd.print(letterArray[charIndex]);

      lcd.setCursor(0, 1);
      lcd.print("                "); 
      lcd.setCursor(0, 1);
      lcd.setCursor(0, 1);
      lcd.print("    ");
      lcd.setCursor(4, 1);
      lcd.print(timeArray[0]);
      lcd.setCursor(6, 1);
      lcd.print(" : ");
      lcd.setCursor(9, 1);
      lcd.print(timeArray[1]);
      lcd.setCursor(11, 1);
      lcd.print(" : ");
      lcd.setCursor(14, 1);
      lcd.print(timeArray[2]);
      
      if (digitalRead(buttonDelete) == PUSH)
      {
        //line to set currently selected time to 0
        leaderboardArray[index] = 0;
        leaderboardChar[index] = 'x';
        leaderboardArray[5] = 0;
        leaderboardChar[5] = 'x';

        quickFix(leaderboardArray);
        sortArray(leaderboardArray, leaderboardChar);
        quickFixUndo(leaderboardArray);

        
        //line to write to EEPROM
        writeLoop(leaderboardArray, leaderboardChar);
        
        lcd.setCursor(0, 0);
        lcd.print("                "); 
        lcd.setCursor(0, 0);
        lcd.print("Time Deleted");

        lcd.setCursor(0, 1);
        lcd.print("                "); 
        lcd.setCursor(4, 1);
        lcd.print(timeArray[0]);
        lcd.setCursor(6, 1);
        lcd.print(" : ");
        lcd.setCursor(9, 1);
        lcd.print(timeArray[1]);
        lcd.setCursor(11, 1);
        lcd.print(" : ");
        lcd.setCursor(14, 1);
        lcd.print(timeArray[2]);

        delay(4000);
        whileControlVar = 1;
      }

      //go back if select button is pushed
      else if (digitalRead(buttonSelect) == PUSH)
      {
        leaderboardChar[index] = letterArray[charIndex];
        writeLoop(leaderboardArray, leaderboardChar);
        
        whileControlVar = 1;
      }
     
      //go up through letters
      else if (digitalRead(buttonUp) != NOPUSH)
      {
        if (charIndex != 25)
        {
          charIndex++;
        }
        else
        {
          charIndex = 0;
        }
        delay(250);
      }

      else if (digitalRead(buttonDown) != NOPUSH)
      {
        if (charIndex != 0)
        {
          charIndex = charIndex - 1;
        }
        else
        {
          charIndex = 25;
        }
        delay(250);
      } 
       //display time and label
      
    }
    delay (500);
  }

//go up through times
  else if (digitalRead(buttonUp) != NOPUSH)
  {
    if (index != 0)
    {
      index = index - 1;
    }
    else
    {
      index = 4;
    }
    //to update the display time
    secondConverter(leaderboardArray[index],timeArray);
    
    delay(500);
  }

//go down through times
  else if (digitalRead(buttonDown) != NOPUSH)
  {
    if (index != 4)
    {
      index++;
    }
    else
    {
      index = 0;
    }
    //to update the display time
    secondConverter(leaderboardArray[index],timeArray);
    
    delay(500);
  }
}
