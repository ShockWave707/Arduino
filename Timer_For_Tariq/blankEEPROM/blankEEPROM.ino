#include <Wire.h>
#include "SparkFun_External_EEPROM.h" 
ExternalEEPROM myMem;



void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  
  myMem.put(0, 0);
  myMem.put(10, 0);
  myMem.put(20, 0);
  myMem.put(30, 0);
  myMem.put(40, 0);
  myMem.put(50, 0);
  myMem.put(60, 'x');
  myMem.put(70, 'x');
  myMem.put(80, 'x');
  myMem.put(90, 'x');
  myMem.put(100, 'x');
  myMem.put(110, 'x');
  
  myMem.put(120, 0);
  myMem.put(130, 0);
  myMem.put(140, 0);
  myMem.put(150, 0);
  myMem.put(160, 0);
  myMem.put(170, 0);
  myMem.put(180, 'x');
  myMem.put(190, 'x');
  myMem.put(200, 'x');
  myMem.put(210, 'x');
  myMem.put(220, 'x');
  myMem.put(230, 'x');
}

void loop() {
  // put your main code here, to run repeatedly:
}
