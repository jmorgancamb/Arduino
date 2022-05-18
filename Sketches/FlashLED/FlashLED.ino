
void setup() 
{
    // initialize digital pin 13 as an output.
    pinMode(PB1, OUTPUT);
}

void loop() 
{
    digitalWrite(PB1, HIGH);
    delay(250);
    digitalWrite(PB1, LOW);
    delay(250);
}
