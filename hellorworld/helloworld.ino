
void setup(){
    pinMode(13, OUTPUT);
}

void loop(){
    Serial.println("hello world");
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}