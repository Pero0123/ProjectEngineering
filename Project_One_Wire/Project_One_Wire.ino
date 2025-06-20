#include <OneWire.h>
#include <EEPROM.h>
#define ONEWIRE_CRC
#define ONEWIRE_CRC8_TABLE  //This will use the 2x16 lookup table to calculate the CRC. This is much faster but comes at a memory cost

OneWire ds(10);

void setup() {
  Serial.begin(9600);
}

void loop() {
  byte i;
  byte j = 0;
  byte data[9];
  byte addr[8];
  bool sensorConnected;
  byte type_s = 1;
  int const ARRAY_SIZE = 20;
  uint8_t addresses[ARRAY_SIZE][8];
  static uint8_t SensorCount = 0;
  float celsius;
  static uint8_t addressesConfigured = 0;



  pinMode(13, INPUT);

  if (!addressesConfigured && !digitalRead(13))  //reads data from eeprom on first start
  {
    SensorCount = EEPROM.read(200);
    uint8_t k = 0;
    for (j = 0; j <= (SensorCount); j++) {
      for (i = 0; i < 8; i++) {
        addresses[j][i] = EEPROM.read(k);
        k++;
      }
    }
    Serial.println("Addresses Restored from EEPROM");
    addressesConfigured = 1; //on first loop, the addresses stored in eeprom will be copied into working memory unless the address search is ran
  }


  if (digitalRead(13)) {
    Serial.println("Address Search Mode Selected");
    ds.reset_search();  // Reset the search state

    while (ds.search(addr)) {  //starts a search and check if a new address has been found
      for (i = 0; i < 8; i++) {
        addresses[j][i] = addr[i];  //store the address stored by search in a 2 dimensional array
      }
      Serial.println();
      uint8_t addressCRC = ds.crc8(addresses[j], 8);
      if (addressCRC = addresses[j][7]) {
        Serial.print("\t CRC valid \t");
      } else {
        Serial.print("\t CRC not valid. Restarting search \t");
        break;
      }

      Serial.print("address CRC: ");
      Serial.print(addressCRC, HEX);
      Serial.print("\t");

      //print out the address
      for (i = 0; i < 8; i++) {
        if (addresses[j][i] < 0x10) {
          Serial.print("0");  // Add leading zero for single digit hex values
        }
        Serial.print(addresses[j][i], HEX);
        Serial.print(" ");
      }
      SensorCount = j;
      j++;
      if (j >= ARRAY_SIZE) {
        Serial.println("Maximum number of addresses reached");  //limit number of devices to array size
        break;                                                // Limit to ARRAY_SIZE addresses
      }
    }

    //Write data to eeprom for persistant storage
    uint8_t k = 0;
    for (j = 0; j <= SensorCount; j++) {
      for (i = 0; i < 8; i++) {
        EEPROM.update(k, addresses[j][i]);
        k++;
      }
    }

    EEPROM.update(200, SensorCount);  //store the sensor count in location 200 of eeprom. note: change to location 0 later for maintainability
    Serial.println("\nAll addresses stored");
    Serial.println();
    while (digitalRead(13)) {};  //block until jumper is removed
  }


  //prepare sensors for a reading the temperature using temp convert command
  ds.reset();         // reset. depowers the bus
  ds.write(0xCC, 0);  //Issue the skip rom command to address all devices on the bus
  ds.write(0x44, 1);  //starts a temperature conversion for all devices
  delay(800);         //allow atleast 750ms for temp conversion at the 9bit resolution
  ds.depower();       //depowers the bus

  //read the scatchpad and print out the temperature for each sensor in the address array
  for (j = 0; j <= SensorCount; j++) {
    ds.reset();
    ds.select(addresses[j]);  //select device address
    ds.write(0xBE, 0);        // Read scratchpad command


    for (i = 0; i < 9; i++) {  // we need 9 bytes
      data[i] = ds.read();
      // Serial.print(data[i], HEX);
      // Serial.print(" ");
    }

    uint8_t crc = ds.crc8(data, 8);  //calculate crc
    if (crc == data[8]) {
      Serial.print("\tCRC is valid. Device is Connected\t");
    } else {
      Serial.print("\tCRC failed. Check Device Connection\t");
    }
    // Serial.print("\tcrc: ");
    // Serial.print(crc, HEX);
    // Serial.print("\t");


    //code to allow different resolutions and sensors. currently defaults to type_s = 1
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3;  // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;       // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3;  // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1;  // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    Serial.print("Sensor: ");
    Serial.print(j + 1);
    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius, ");
    Serial.println();
  }
  Serial.println();
}
