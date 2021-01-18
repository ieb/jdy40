#include "../testsupport.h"
#include <jdy40.h>




using namespace fakeit;


Stream* output = ArduinoFakeMock(Stream);
Stream* debug = ArduinoFakeMock(Stream);
MockStreamLoader loader;
Jdy40 jdy(10);
#define INPUT_BUFFER_LEN 1024
char inputBuffer[INPUT_BUFFER_LEN];


void setUp(void) {
    ArduinoFakeReset();
    setupStream();
    When(Method(ArduinoFake(), pinMode)).Return();
    jdy.begin(output,9600);
    // jdy.setDebug(debug);
    jdy.setInputBuffer(&inputBuffer[0],INPUT_BUFFER_LEN);

    Verify(Method(ArduinoFake(), pinMode).Using(10, OUTPUT)).Once();
}



void test_check_fail_crc() {



    TEST_ASSERT_EQUAL_INT16(-1, jdy.checkCRC(""));
    TEST_ASSERT_EQUAL_INT16(-1, jdy.checkCRC("12312,12312,"));
    TEST_ASSERT_EQUAL_INT16(-1, jdy.checkCRC("12312,12312,asaa"));

}

void test_write() {
    jdy.writeLine("testing,1,2,3");

    Verify(OverloadedMethod(ArduinoFake(Stream), print, size_t(const char *)).Using("testing,1,2,3")).Once();
    Verify(OverloadedMethod(ArduinoFake(Stream), print, size_t(char)).Using(',')).Once();
    Verify(OverloadedMethod(ArduinoFake(Stream), println, size_t(int, int)).Using(13080,HEX)).Once();
    
}

void test_read() {
    loader.load("testing,1,2,3,3318\n");
//    When(Method(ArduinoFake(Serial), available)).AlwaysReturn(1);
//    // output line and chcksum in hex 13080 == 0x3318
//    When(Method(ArduinoFake(Serial), read)).Return('t','e','s','t','i','n','g',',','1',',','2',',','3',',','3','3','1','8','\n');
    char * line = jdy.readLine();
    TEST_ASSERT_NOT_NULL(line);
    TEST_ASSERT_EQUAL_STRING("testing,1,2,3", line);

}


void test_read_badchecksum() {
    // bad checksum
    loader.load("testing,1,2,3,3316\n");
    char * line = jdy.readLine();
    TEST_ASSERT_NULL(line);
}



void test_read_emptyline() {
    loader.load("\n");
    char * line = jdy.readLine();
    TEST_ASSERT_NULL(line);
}


void test_read_comma() {
    loader.load(",");
    char * line = jdy.readLine();
    TEST_ASSERT_NULL(line);
}



int main(int argc, char **argv) { 
    try {
        UNITY_BEGIN();
        RUN_TEST(test_check_fail_crc);
        RUN_TEST(test_write);
        RUN_TEST(test_read);
        RUN_TEST(test_read_badchecksum);
        RUN_TEST(test_read_emptyline);
        RUN_TEST(test_read_comma);
        return UNITY_END();
    } catch( UnexpectedMethodCallException e) {
            std::cout << "Exception:" << e << std::endl;

    }
}