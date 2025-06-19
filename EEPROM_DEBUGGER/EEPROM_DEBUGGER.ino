#include <EEPROM.h>

//2 scripts to assist with debugging and testing the persistance storage of one-wire addresses. note that EEPROM has a write limit of 100000 so code should not be place in a loop
void setup() {
  Serial.begin(9600);

  uint8_t i;
  //write 0xFF to the first 160 bytes in EEPROM
    for(i=0; i<=160 ; i++ ){
  Serial.print(" ");
  EEPROM.put(i, 255);
  EEPROM.put(200, 255);//sensor count location
  }

  //Print out HEX value of first 160 EEPROM value
  for(i=0; i<=160 ; i++ ){
  Serial.print(" ");
  Serial.print(EEPROM.read(i),HEX);
  }
  Serial.print("\t\t");
  Serial.println(EEPROM.read(200),HEX);//sensor count location
}

void loop() {

}
