// LCD Connections :-
//
// GND (grey)   -> GND
// VCC (red)    -> 3.3V
// SDA (blue)   -> A4
// SCL (violet) -> A5
//
// Rotary Encoder Connections :-
//
// CLK (brown)  -> 4 
// DT  (yellow) -> 2
// SW  (grey)   -> 3
// +   (red)    -> 5V
// GND (black)  -> GND

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include <task.h>
#include <queue.h>

typedef enum { eENCODER_MOVED, eSWITCH_PRESSED } rotaryEncoderAction_t;

typedef struct rotaryEncoderMsg_tag
{
    rotaryEncoderAction_t action;
    int aPinState;
    int bPinState;
} rotaryEncoderMsg_t;

const int statusLEDPin = PB1;
const int aPin = PD2;
const int bPin = PD4;
const int switchPin = PD3;

QueueHandle_t rotaryEncoderMsgQueue = NULL;
QueueHandle_t counterValueQueue = NULL;
TimerHandle_t statusLedTimer = NULL;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() 
{
    rotaryEncoderMsgQueue = xQueueCreate(5, sizeof(rotaryEncoderMsg_t)); 
    counterValueQueue = xQueueCreate(5, sizeof(int)); 

    pinMode(statusLEDPin, OUTPUT);
    statusLedTimer = xTimerCreate("STATUS_LED_TMR", pdMS_TO_TICKS(500), pdTRUE, (void*) 0, statusLedTimerCallback);
    xTimerStart(statusLedTimer, 0);
    
    // Now set up two tasks to run independently.
    xTaskCreate(LCDTask,
                "LCD",
                128,    // Stack size
                NULL,
                2,      // priority
                NULL);
    
    xTaskCreate(RotaryEncoderTask,
                "Rotary Encoder",
                128,    // Stack size
                NULL,
                1,      // priority
                NULL);
}

void loop() 
{
}

void statusLedTimerCallback(TimerHandle_t timerHandle)
{
    static int ledStatus = HIGH;
        
    digitalWrite(statusLEDPin, ledStatus);
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
}

void LCDTask(void *pvParameters)
{
    (void) pvParameters;

    int counterValue = 0;

    lcd.init();
    lcd.backlight();    // To Power ON the back light
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rotary enc test");

    for (;;)
    {
        if (xQueueReceive(counterValueQueue, &counterValue, portMAX_DELAY))
        {
            char lcdString[16];

            sprintf(lcdString, "Position = %d  ", counterValue);
            lcd.setCursor(0, 1);
            lcd.print(lcdString);
        }
    }
}

void RotaryEncoderMoveISR()
{
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();

    if (interruptTime - lastInterruptTime > 5)
    {
        rotaryEncoderMsg_t rotaryEncoderMsg = { .action = eENCODER_MOVED, .aPinState = digitalRead(aPin), .bPinState = digitalRead(bPin) };
        xQueueSendFromISR(rotaryEncoderMsgQueue, &rotaryEncoderMsg, NULL);
        lastInterruptTime = interruptTime;
    }
}

void RotaryEncoderSwitchISR()
{
    rotaryEncoderMsg_t rotaryEncoderMsg = { .action = eSWITCH_PRESSED, .aPinState = LOW, .bPinState = LOW };
    xQueueSendFromISR(rotaryEncoderMsgQueue, &rotaryEncoderMsg, NULL);
}

void RotaryEncoderTask(void *pvParameters)
{
    (void) pvParameters;

    int aPinLastState = LOW, counter = 0;

    pinMode(aPin, INPUT);
    pinMode(bPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(aPin), RotaryEncoderMoveISR, LOW);

    pinMode(switchPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(switchPin), RotaryEncoderSwitchISR, LOW);
    
    for (;;)
    {
        rotaryEncoderMsg_t rotaryEncoderMsg;
        
        if (xQueueReceive(rotaryEncoderMsgQueue, &rotaryEncoderMsg,  portMAX_DELAY) == pdPASS) 
        {    
            switch (rotaryEncoderMsg.action)
            {
                case eENCODER_MOVED:
                    if (rotaryEncoderMsg.bPinState == LOW)
                    {
                        counter--;
                    }
                    else
                    {
                        counter++;
                    }
                    counter = min(128, max(-128, counter));
                    break;

                case eSWITCH_PRESSED:
                    counter = 0;
                    break;    

                default:
                    break;
            }
            
            xQueueSend(counterValueQueue, &counter, portMAX_DELAY);
        }
    }
}
