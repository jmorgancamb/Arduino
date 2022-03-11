#include <avr/interrupt.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const int aPin = 2;
const int bPin = 4;

volatile int aPinLastState = 0, aPinState = 0, bPinState = 0, counter = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    pinMode(aPin, INPUT);
    pinMode(bPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(aPin), RotaryEncISR, RISING);
    aPinLastState = digitalRead(aPin); 

    lcd.init();
    lcd.backlight();    // To Power ON the back light
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rotary Enc Demo");
}

void loop() 
{
    static char posnString[16];

    sprintf(posnString, "Position: %d  ", counter);
    lcd.setCursor(0, 1);
    lcd.print(posnString);

    delay(50);
}

void RotaryEncISR()
{
    digitalWrite(LED_BUILTIN, HIGH);
        
    aPinState = digitalRead(aPin);

    // if the previous and the current state of the clock pin are different, that means a pulse has occured
    if (aPinState != aPinLastState)
    {     
        // if the DT pin state is different to the clock pin state, that means the encoder is rotating clockwise
        if (digitalRead(bPin) != aPinState) 
        { 
            counter++;
        } 
        else 
        {
            counter--;
        }
   
        aPinLastState = aPinState; // updates the previous state of the clock pin with the current state
    }   

    digitalWrite(LED_BUILTIN, LOW);
}
