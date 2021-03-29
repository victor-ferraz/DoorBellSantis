#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <BlynkSimpleEsp32.h> // Blynk-ESP32

byte storedCard[4];   // Stores an ID read from EEPROM
byte masterCard[4];   // Stores master card's ID read from EEPROM


//////////////////////////////////////// Check if has master card registered //////////////////////////////
boolean hasMaster() {
  uint8_t magicNumber = EEPROM.read(1);
  if (magicNumber == 0x55){return true;}
  else{return false;}
}

//////////////////////////////////////// Read master card ID from EEPROM //////////////////////////////
void readMaster() {
  for ( uint8_t i = 2; i < 6; i++ ) {     // Loop 4 times to get the 4 Bytes
    masterCard[i-2] = EEPROM.read(i);   // Assign values read from EEPROM to array
  }
}

//////////////////////////////////////// Read master card ID from EEPROM //////////////////////////////
void writeMaster(byte card[] ) {
  for ( uint8_t i = 2; i < 6; i++ ) {     // Loop 4 times to get the 4 Bytes
    EEPROM.write(i, card[i-2]);   // Assign values read from EEPROM to array
  }
  EEPROM.write(1,0x55); // write the magic number
}

//////////////////////////////////////// Read master card ID from EEPROM //////////////////////////////
void deleteMaster() {
  for ( uint8_t i = 2; i < 6; i++ ) {     // Loop 4 times to get the 4 Bytes
    EEPROM.write(i, 0x00);   // Assign values read from EEPROM to array
  }
  EEPROM.write(1,0x00); // delete the magic number
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    // successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
    Blynk.virtualWrite(V0, "Cartao salvo!\n");
  }
  else {
    // failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
    Blynk.virtualWrite(V0, "Falhou o registro do cartao\n");
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    // failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    // successDelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}