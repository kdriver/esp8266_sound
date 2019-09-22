#include <Arduino.h>
#include "History.h"
#include <unity.h>


void  test_h(void)
{
    History *h = new History(20);

    h->add(5);
    h->add(10);
    h->add(15);

    TEST_ASSERT_EQUAL(10.0,h->average());

 

}
void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_h);
    UNITY_END();
}



void loop() {
    delay(10);
}