
void pump1Test() {
    analogWrite(pump1, 255);
    delay(1000);
    for (int i = 50; i >= 0; i--) {
        Serial.println("Pump 1 at Rate: " + String(i) + "");
        analogWrite(pump1, i);
        delay(3000);
    }
    analogWrite(pump1, 0);
    delay(5000);
}

void pump2Test() {
    analogWrite(pump2, 255);
    delay(1000);
    for (int i = 50; i >= 0; i--) {
        Serial.println("Pump 2 at Rate: " + String(i) + "");
        analogWrite(pump2, i);
        delay(3000);
    }
    analogWrite(pump2, 0);
    delay(5000);
}

void timerTest() {
    Serial.println("Ask Timer 1");
    Serial.println(String(myTimer(1)));
    delay(1000);
    Serial.println("Ask again Timer 1");
    Serial.println(String(myTimer(1)));
    delay(1000);

    Serial.println("Ask Timer 2");
    Serial.println(String(myTimer(2)));
    delay(1000);
    Serial.println("Ask again Timer 2");
    Serial.println(String(myTimer(2)));
    delay(1000);

    Serial.println("Set Timer 1");
    Serial.println(String(myTimer(1, 1000)));
    delay(500);
    Serial.println("AskTimer 1");
    Serial.println(String(myTimer(1)));
    Serial.println("Set Timer 2");
    Serial.println(String(myTimer(2, 500)));

    Serial.println("Ask Timer 2");
    Serial.println(String(myTimer(2)));
    delay(1000);

    Serial.println("Ask Timer 2");
    Serial.println(String(myTimer(2)));
    delay(1000);

    Serial.println("Ask again Timer 1");
    Serial.println(String(myTimer(1)));
    delay(1000);
}

void test() {
    return;
    timerTest();
    pump2Test();
    pump1Test();
}
