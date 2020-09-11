#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

QueueHandle_t xLCDMsgQueue = NULL;

void StatusLEDTask(void *pvParameters);
void LCDTask(void *pvParameters);

// the setup function runs once when you press reset or power the board
void setup() 
{
    xLCDMsgQueue = xQueueCreate(5, sizeof(const char*));

    // Now set up two tasks to run independently.
    xTaskCreate(StatusLEDTask,  
                "Blink",
                128,    // Stack size
                NULL,
                2,      // priority
                NULL);

    xTaskCreate(LCDTask,
                "LCD",
                128,    // Stack size
                NULL,
                1,      // priority
                NULL);

    // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
    // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void StatusLEDTask(void *pvParameters)
{
    (void) pvParameters;

    const char * const msg[] = { "Mary had", 
                                 "a little lamb", 
                                 "her father", 
                                 "shot it dead", 
                                 "now Mary takes", 
                                 "her lamb to", 
                                 "school", 
                                 "between two", 
                                 "slices of bread" };
    int index = 0;
  
    // initialize digital pin 13 as an output.
    pinMode(PB1, OUTPUT);

    for (;;) 
    {
        digitalWrite(PB1, HIGH);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        digitalWrite(PB1, LOW);
        vTaskDelay(500 / portTICK_PERIOD_MS);

        xQueueSend(xLCDMsgQueue, &msg[index], 0);
        index = (index >= 8) ? 0 : index + 1;
    }
}

void LCDTask(void *pvParameters)
{
    (void) pvParameters;

    // I2C pins declaration
    LiquidCrystal_I2C lcd(0x27, 16, 2); 

    lcd.init();
    lcd.backlight();    // To Power ON the back light

    for (;;)
    {
        char *msg, data[20];
    
        xQueueReceive(xLCDMsgQueue, &msg, portMAX_DELAY);
        lcd.clear();
        lcd.setCursor(0,0);
        sprintf(data, "*msg = 0x%04x", msg);
        lcd.print(data);
        lcd.setCursor(0, 1);
        lcd.print(msg);
    }
}
