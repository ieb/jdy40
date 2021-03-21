
#ifndef _WINDSENSOR_H
#define _WINDSENSOR_H

#include <Arduino.h>

class Jdy40 {

    public:
        Jdy40(int dataEnablePin=4);
        void begin(Stream *_jdy40Stream=&Serial, uint16_t _baud=9600);
        void setDebug(Stream *_debugStream);
        void setInputBuffer(char * inputBuffer, uint16_t maxLen);

        void startConfig();
        void endConfig();
        char *readLine();
        uint16_t writeLine(const char *output);
        void init();
        bool setBaud(uint16_t baud);

        bool setRFID(uint16_t  rfid);
        bool setDeviceID(uint16_t dvid);
        bool setChannel(uint16_t chan);
        int16_t checkCRC(const char *inputLine);
        uint16_t getCRCErrors();


    private:
        void dumpHex(const char * str);
        bool send(const char *cmd);
        uint16_t crc_ccitt (const uint8_t * str, unsigned int length);
        void setupJDY40();
        Stream * debugStream;
        Stream * jdy40Stream;
        uint8_t dataEnPin;
        uint16_t baud;
        bool inConfig;
        uint16_t bufferPos;
        uint16_t maxLineLength;
        uint16_t crcErrors = 0;
        char *inputLine;
};

#endif

