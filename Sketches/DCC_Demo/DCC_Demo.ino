#include <avr/io.h>
#include <avr/interrupt.h>

typedef enum {
    REVERSE,
    FORWARD
};

typedef enum {
    PREAMBLE,
    ADDR_BYTE,
    DATA_BYTE,
    ERROR_BYTE
};

// using the internal 4.8MHz oscillator = 1 CPU tick every 0.21us (approx)
// using a pre-scaler of 8 updates the timer0 every 1.68us
// 200us (i.e. for a DCC zero bit) is reached every 119 timer0 ticks
// 116us (i.e. for a DCC one bit) is reached every 69 timer0 ticks
typedef enum {
  DCC_ZERO = 119 / 2,
  DCC_ONE = 69 / 2
};

typedef unsigned char  byte_t;
typedef unsigned short word_t;

typedef struct 
{
    union {
        struct {
            byte_t addr:7;
            byte_t unused:1;
        } _addr;
        byte_t addr;
    };
    union {
        struct {
            byte_t speed:5;
            byte_t direction:1;
            byte_t unused:2;      /* note - this is always set to the value b01. */
        } _data;
        byte_t data;
    };
    byte_t error;
} dcc_pkt_t;

static dcc_pkt_t _cntl_pkt, _rail_pkt;

#if 0
ISR(TIM0_COMPA_vect)
{
    static byte_t dcc_bit = 0, pkt_state = PREAMBLE; 
    static word_t bit_mask = 0x0400; 

    // toggle the DCC output pin.
    PORTB = (PORTB ^ (1 << 4)); 

    if (!(++dcc_bit % 2)) 
    {
        // ISR called twice for each DCC bit (i.e. one high and one low part of 
        // the DCC bit cycle), so only evaluate the packet's next bit on every 
        // second call. 

        bit_mask >>= 1;

        switch (pkt_state) 
        {
            case PREAMBLE:
                if (bit_mask == 0) 
                {
                    pkt_state = ADDR_BYTE; 
                    bit_mask = 0x0100; 
                    
                    // next bit is the address byte start bit - i.e. zero.
                    OCR0A = DCC_ZERO;
                }
                break;

            case ADDR_BYTE:
                if (bit_mask == 0) 
                {
                    pkt_state = DATA_BYTE;
                    bit_mask = 0x0100;
                    
                    // next bit is the data byte start bit - i.e. zero.
                    OCR0A = DCC_ZERO;
                }
                else 
                {
                    // next bit is one of the 8 address bits.
                    OCR0A = (_rail_pkt.addr & bit_mask) ? DCC_ONE : DCC_ZERO; 
                }
                break;

            case DATA_BYTE:
                if (bit_mask == 0) 
                {
                    pkt_state = ERROR_BYTE;
                    bit_mask = 0x0100;
                    
                    // next bit is the error byte start bit - i.e. zero.
                    OCR0A = DCC_ZERO;
                }
                else 
                {
                    // next bit is one of the 8 data bits.
                    OCR0A = (_rail_pkt.data & bit_mask) ? DCC_ONE : DCC_ZERO; 
                }
                break;

            case ERROR_BYTE:
                if (bit_mask == 0) 
                {
                    pkt_state = PREAMBLE;
                    bit_mask = 0x0400;
                    
                    // next bit is the end of packet bit followed by the next packets 
                    // 10 preamble bits - i.e. all ones. 
                    OCR0A = DCC_ONE;

                    // get the next DCC packet data.
                    _rail_pkt.addr  = _cntl_pkt.addr;
                    _rail_pkt.data  = _cntl_pkt.data;
                    _rail_pkt.error = _rail_pkt.addr ^ _rail_pkt.data;
                }
                else 
                {
                    // next bit is one of the 8 error bits.
                    OCR0A = (_rail_pkt.error & bit_mask) ? DCC_ONE : DCC_ZERO; 
                }
                break;

            default:
                break;
        }
    }
}
#endif

ISR(TIM1_COMPA_vect)
{
    PORTD |= (1 << DDD5);    
}

ISR(TIM1_COMPB_vect)
{
    TCNT1 = 0;
    PORTD &= ~(1 << DDD5);
}

void setup() 
{
    // PD5 is now an output
    DDRD = (1 << DDD5);

    // set up timer with no prescaler, CTC mode
    TCCR1B |= /*(1 << WGM12) |*/ (1 << CS12);
 
    // enable output compare interrupts
    TIMSK1 = (1 << OCIE1A)/* | (1 << OCIE1B) */;

    // initialize compare value
    OCR1A = 800;
    OCR1B = 400;

    // initialize counter
    TCNT1 = 0;

    interrupts();
}

void loop()
{ 
    //PORTD |= (1 << DDD5);
    //PORTD &= ~(1 << DDD5);
}
