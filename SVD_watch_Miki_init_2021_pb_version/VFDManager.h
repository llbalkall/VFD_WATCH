#ifndef VFDMANAGER_H_INCLUDED
#define VFDMANAGER_H_INCLUDED

#include <Arduino.h>

/* mapping of 6920's OUT pins to the bits in shift register */
#define OUT0  0x0002      //B00000010
#define OUT1  0x0004      //B00000100
#define OUT2  0x0008      //B00001000
#define OUT3  0x0010      //B00010000
#define OUT4  0x0020      //B00100000
#define OUT5  0x0040      //...
#define OUT6  0x0080
#define OUT7  0x0100
#define OUT8  0x0200
#define OUT9  0x0400
#define OUT10 0x0800      //...
#define OUT11 0x0001      //B00000001
                                    //   -- A -- 
#define segment_A     OUT11         //  |       |
#define segment_B     OUT8          //  F       B
#define segment_C     OUT2          //  |       |
#define segment_D     OUT3          //   -- G --
#define segment_E     OUT1          //  |       |  
#define segment_F     OUT6          //  E       C
#define segment_G     OUT9          //  |       |
                                    //   -- D --    
#define multiplexer_1    OUT4   // 10 h
#define multiplexer_2    OUT0   // 1  h
#define multiplexer_3    OUT5   // :
#define multiplexer_4    OUT7   // 10 min
#define multiplexer_5    OUT10  // 1  min

#define DATA_PIN 0
#define CLK_PIN 1
#define LOAD_PIN 2
#define POWER_SWITCH_PIN SDA1
#define BLANK_PIN 4
#define HEAT1_PIN 5
#define HEAT2_PIN 6

class VFDManager{
    private:
        const uint16_t cells[5] = {multiplexer_1, multiplexer_2, multiplexer_3, multiplexer_4, multiplexer_5};
        const short COLON_BLINK_PERIOD = 250;
        unsigned char current_cell_id;
        unsigned long colon_millis;
        

    public:
        int heat_counter;
        bool repower;
        bool colon_steady;
        char displayed_characters[5];
        VFDManager();
        void set_cell(uint8_t cell_num, char character_to_set, bool include_colon);
        unsigned int char_to_segments(char inputChar);
        void update_char_array(char* characters);
        void update_char_array(char c1, char c2, char c3, char c4, char c5);
        void show_displayed_character_array(unsigned long current_millis);
        void heating();
        void turn_on();
        void turn_off();
};

#endif //VFDMANAGER_H_INCLUDED
