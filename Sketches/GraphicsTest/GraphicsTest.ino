// There should be output on LCD screen. See the different ways of clearing the screen.

// Connect data pins LCD_D 0-7 to arduino UNO:
// LCD_D 0 -- D8
// LCD_D 1 -- D9
// LCD_D 2 -- D2
// LCD_D 3 -- D3
// LCD_D 4 -- D4
// LCD_D 5 -- D5
// LCD_D 6 -- D6
// LCD_D 7 -- D7
// Connect command pins:
// LCD_RST -- A4   1 -> 0 min 15 micros 0 -> 1 
// LCD_CS  -- A3   chip select, aktiv LOW
// LCD_RS  -- A2   data/command select, 0 command, 1 data
// LCD_WR  -- A1   0 -> 1, HIGH when not used
// LCD_RD  -- A0   0 -> 1, HIGH when not used

// arduino UNO ports:
// B (digital pin 8 to 13) 
// C (analog input pins) 
// D (digital pins 0 to 7)   0 1 are RX TX, don't use 

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void LCD_write(uint8_t d) {
  // ILI9341 reads data pins when WR rises from LOW to HIGH (A1 pin on arduino)
  PORTC = PORTC & B11111101; // WR 0
  
  // data pins of ILI9341 connected to two arduino ports
  PORTD = (PORTD & B00000011) | ((d) & B11111100); 
  PORTB = (PORTB & B11111100) | ((d) & B00000011); 
  
  PORTC = PORTC | B00000010; // WR 1
}

void LCD_command_write(uint8_t d) {
  PORTC = PORTC & B11111011; // LCD_RS = 0, arduino pin A2
  // write data pins
  LCD_write(d);
}

void LCD_data_write(uint8_t d) {
  PORTC = PORTC | B00000100; // LCD_RS = 1, arduino pin A2
  // write data pins
  LCD_write(d);
}

uint8_t LCD_read(void) {
  // CS LOW, WR HIGH, RD HIGH->LOW>HIGH, RS(D/C) HIGH 
  PORTC = PORTC | B00000100; // RS 1
  
  // LCD_RD - arduino pin A0
  // After RD falls from HIGH to LOW ILI9341 outputs data until RD returns to HIGH
  PORTC = PORTC & B11111110; // RD 0
  BD_as_input(); // Set arduino pins as input
  uint8_t pin72 = PIND & B11111100; // Read data pins 7-2
  uint8_t pin10 = PINB & B00000011; // Read data pins 1-0
  PORTC = PORTC | B00000001; // RD 1
  BD_as_output(); // Re-Set arduino pins as output
  return pin72 | pin10;
}

void BD_as_input(void) {
  // Pins 7-2 as input, no change for pins 1,0 (RX TX)
  DDRD = DDRD & B00000011; 
  // Pins 8-9 as input
  DDRB = DDRB & B11111100; 
}

void BD_as_output(void) {
  // Pins 7-2 as output, no change for pins 1,0 (RX TX)
  DDRD = DDRD | B11111100; 
  // Pins 8-9 as output
  DDRB = DDRB | B00000011; 
}

void LCD_init(void) {
  // LCD_RESET 1 - 0 - 1, arduino pin A4
  PORTC = PORTC | B00010000; // 1
  delay(10);
  PORTC = PORTC & B11101111; // 0
  delay(20);
  PORTC = PORTC | B00010000; // 1
  delay(20);
  
  // CS HIGH, WR HIGH, RD HIGH, CS LOW
  PORTC = PORTC | B00001000; // CS 1
  PORTC = PORTC | B00000010; // WR 1
  PORTC = PORTC | B00000001; // RD 1
  PORTC = PORTC & B11110111; // CS 0
  
  LCD_command_write(0xF7); // Pump ratio control
  LCD_data_write(0x20); // 
  
  LCD_command_write(0x3A); // COLMOD: Pixel Format Set
  LCD_data_write(0x55); 
  
  LCD_command_write(0x36); // Memory Access Control 
  // MY  - Row Address Order (bit7)
  // MX  - Column Address Order
  // MV  - Row / Column Exchange
  // ML  - Vertical Refresh Order
  // BGR - RGB-BGR Order
  // MH  - Horizontal Refresh ORDER(bit2)
  LCD_data_write(B00001000); 
  
  LCD_command_write(0x11); // Sleep OUT
  LCD_command_write(0x29); // Display ON
  
  delay(50);
}

void LCD_rect(int16_t col,int16_t row, int16_t width, int16_t height, int16_t color) {
  
  LCD_command_write(0x2a); // Column Address Set
  LCD_data_write(row>>8);
  LCD_data_write(row);
  LCD_data_write((row+height-1)>>8);
  LCD_data_write(row+height-1);
  LCD_command_write(0x2b); // Page Address Set
  LCD_data_write(col>>8); 
  LCD_data_write(col);
  LCD_data_write((col+width-1)>>8);
  LCD_data_write(col+width-1);
  LCD_command_write(0x2c); // Memory Write
 
  byte chigh=color >> 8;
  byte clow=color;
  int i,j;
  for(i=0;i<width;i++)
    for(j=0;j<height;j++)
    {
      LCD_data_write(chigh);
      LCD_data_write(clow);
    }
}

void LCD_clear(byte color) {
  /* 
  Clear screen faster sacrifing color depth. Instead of writing
  to data bits high and low byte of the color for each pixel, which takes more 
  than 300ms to fill the screen, set once data bits to 0's for black or 
  to 1's for white and start changing control bit WR from LOW to HIGH to 
  write the whole area. It takes cca 70 ms. In this way the colors of screen are
  limited to those with the same high and low byte. For example setting color
  to 0x0C fills the screen with color 0x0C0C.
  Writing two pixels in one cycle lowering cycle count from 76800 (240x320) to 
  38400 clears screen in less then 30ms.
  */
  
  LCD_command_write(0x2a); 
  LCD_data_write(0);
  LCD_data_write(0);
  LCD_data_write(0);
  LCD_data_write(0xEC);
  LCD_command_write(0x2b); 
  LCD_data_write(0); 
  LCD_data_write(0);
  LCD_data_write(1);
  LCD_data_write(0x3F);
  LCD_command_write(0x2c);
  
  PORTC = PORTC | B00000100; // LCD_RS = 1 - DATA
 
  PORTD = (PORTD & B00000011) | ((color) & B11111100); 
  PORTB = (PORTB & B11111100) | ((color) & B00000011); 
  
  uint16_t x;
  x=38400; // 240*320/2
  byte wr0=PORTC & B11111101; // set WR 0
  byte wr1=PORTC | B00000010; // set WR 1
  for(uint16_t y=0;y<x;y++)
  {
    PORTC = wr0;
    PORTC = wr1;
    PORTC = wr0;
    PORTC = wr1;
    
    PORTC = wr0;
    PORTC = wr1;
    PORTC = wr0;
    PORTC = wr1;
  }
}

void setup()
{
  Serial.begin(9600);
  // Set pins 1-8 as output
  BD_as_output();
  
  // Set pins A0-A4 as output
  DDRC = DDRC | B00011111; 
  
  LCD_init();
}

void loop()
{
  LCD_rect(0,0,320,240, WHITE); // clear whole screen, slow
  delay(500);
  LCD_rect(0,0,320,240, 0x0C0C); 
  delay(1000);
  
  LCD_clear(0xFF); // clear whole screen, faster
  delay(500);
  LCD_clear(0x0C); ;
  delay(1000);
  
  LCD_rect(200,50,50,50, WHITE);
  LCD_rect(50,50,50,50, BLACK);
  delay(1000);
}
