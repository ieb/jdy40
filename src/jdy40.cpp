#include "jdy40.h"


const char OK[] PROGMEM = "OK";
const char AT_SERIAL[] PROGMEM = "AT+CLSSA0";
const char AT_TRANSMIT_POWER[] PROGMEM = "AT+POWE9";
const char AT_BAUD_1200[] PROGMEM = "AT+BAUD1";
const char AT_BAUD_2400[] PROGMEM = "AT+BAUD2";
const char AT_BAUD_4800[] PROGMEM = "AT+BAUD3";
const char AT_BAUD_9600[] PROGMEM = "AT+BAUD4";
const char AT_BAUD_14400[] PROGMEM = "AT+BAUD5";
const char AT_BAUD_19200[] PROGMEM = "AT+BAUD6";
const char AT_RFID[] PROGMEM = "AT+RFID%u04";
const char AT_DVID[] PROGMEM = "AT+DVID%u04";
const char AT_RFC[] PROGMEM = "AT+RFC%u03";

Jdy40::Jdy40(int dataEnablePin) {
    dataEnPin = dataEnablePin;
    debugStream = NULL;
    inputLine = NULL;
    maxLineLength = 0;
    baud = 0;
}

void Jdy40::begin(Stream *_jdy40Stream, uint16_t _baud) {
    baud = _baud;
    jdy40Stream = _jdy40Stream;
    pinMode(dataEnPin, OUTPUT);
}

void Jdy40::setDebug(Stream *_debugStream) {
    debugStream = _debugStream;
}
void Jdy40::setInputBuffer(char * inputBuffer, uint16_t maxLen) {
  inputLine = inputBuffer;
  maxLineLength = maxLen;
  bufferPos = 0;
}




void Jdy40::startConfig() {
  if ( !inConfig ) {
      digitalWrite(dataEnPin, 0);
      delay(500);
      inConfig = true;  
  }
}

void Jdy40::endConfig() {
  if ( inConfig ) {
      digitalWrite(dataEnPin, 0);   
      delay(500);
      inConfig = false;  
  }
}


void Jdy40::init() {
  setBaud(baud);
  send(AT_SERIAL); // Serial transmission.
  send(AT_TRANSMIT_POWER); // Power 12db
}




void Jdy40::setBaud(uint16_t baud) {
   startConfig();
   switch(baud) {
       case 1200: send(AT_BAUD_1200); return;
       case 2400: send(AT_BAUD_2400); return;
       case 4800: send(AT_BAUD_4800); return;
       case 9600: send(AT_BAUD_9600); return;
       case 14400: send(AT_BAUD_14400); return;
       case 19200: send(AT_BAUD_19200); return;
       default: send(AT_BAUD_9600); return;
   }
}


void Jdy40::setRFID(uint16_t  rfid) {
  char buffer[13];
  sprintf(buffer,AT_RFID, rfid);
  startConfig();
  send(buffer); // Wireless ID set to 1020
}
void Jdy40::setDeviceID(uint16_t dvid) {
  char buffer[13];
  sprintf(buffer,AT_DVID, dvid);
  startConfig();
  send(buffer); // Wireless ID set to 1020
}
void Jdy40::setChannel(uint16_t chan) { 
  char buffer[13];
  sprintf(buffer,AT_RFC, chan); // 128 channels
  startConfig();
  send(buffer); // Wireless ID set to 1020
}

uint16_t Jdy40::crc_ccitt (const uint8_t * str, unsigned int length) {
  uint16_t crc = 0;
  for (unsigned int i = 0; i < length; i++) {
    uint8_t data = str[i];
    data ^= crc & 0xff;
    data ^= data << 4;
    crc = ((((uint16_t)data << 8) | (crc >> 8)) ^ (uint8_t)(data >> 4) 
                    ^ ((uint16_t)data << 3));
  }
  return crc;  
}


int16_t Jdy40::checkCRC(const char *inputLine) {
  uint16_t len = strlen(inputLine);
  uint16_t lastComma = len;
  if ( len == 0 ) {
    return -1;
  }

  while( lastComma > 0 && inputLine[lastComma] != ',') lastComma--;
  if (lastComma == len-1 ) {
    return -1;
  }
  // check the crc, incuding the comma
  uint16_t crc_check = crc_ccitt((const uint8_t *)inputLine, lastComma); 
  uint16_t crc_in = strtoul(&inputLine[lastComma+1],NULL,16);
  if (crc_check == crc_in) {
    return lastComma;
  } else {
    return -1;
  }
}


uint16_t Jdy40::writeLine(const char *output) {
  endConfig();
  uint16_t len = strlen(output);
  uint16_t checksum = crc_ccitt((const uint8_t *)output, len);
  jdy40Stream->print(output);
  jdy40Stream->print(',');
  jdy40Stream->println(checksum,HEX);
  return checksum;
}


char * Jdy40::readLine() {
   endConfig();
  // Read lines from serial into buffer for processing.
  // 
  while(jdy40Stream->available() > 0) {
    char b = jdy40Stream->read();

    if ( maxLineLength && b == '\n') {
      inputLine[bufferPos] = '\0';
      int16_t crcPos = checkCRC(inputLine);
      if ( crcPos >= 0 ) {
        inputLine[crcPos] = '\0';
        bufferPos = 0;
        return inputLine;
      } else {
        if (debugStream != NULL ) {
          debugStream->print(F("CRC Error, rejected"));
          debugStream->println(inputLine);
        }
        // drop the line CRC invalid.
        bufferPos = 0;
        return NULL;
      }
    } else if ( bufferPos < maxLineLength-1 ) {
      inputLine[bufferPos] = b;
      bufferPos++;
    } else {
      // drop extra characters.
    }
  }
  return NULL;
}


int Jdy40::send(const char *cmd) {
  for(int i = 0; i < 4; i++) {
    delay(500);
    while(jdy40Stream->available()) {
      jdy40Stream->read();
    }

    jdy40Stream->println(cmd);
    delay(100);
    String res = jdy40Stream->readStringUntil('\n');
    res.trim();
    if ( res.equals(OK) ) {
      if ( debugStream != NULL ) {
        debugStream->print(cmd);
        debugStream->println(F(" OK"));
      }
      return 1;    
    } 
    if ( debugStream != NULL ) {
      debugStream->println(F("Init Failed command:"));
      debugStream->println(cmd);
      debugStream->println(res);
      dumpHex(res.c_str());
      dumpHex(OK);
    }
  }
  return 0;
}

void Jdy40::dumpHex(const char * str) {
  for (uint16_t i = 0; i<strlen(str); i++)
  {
    debugStream->print(str[i], HEX);//excludes NULL byte
  }
  debugStream->print(F(":"));
  debugStream->println(strlen(str));
}
