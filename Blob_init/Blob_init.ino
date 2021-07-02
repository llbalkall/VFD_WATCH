
  
#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "functions.h" 

#define DS3231_I2C_ADDRESS 0x68

#define setPin(b) ( (b)<8 ? PORTD |=(1<<(b)) : PORTB |=(1<<(b-8)) )
#define clrPin(b) ( (b)<8 ? PORTD &=~(1<<(b)) : PORTB &=~(1<<(b-8)) )
#define tstPin(b) ( (b)<8 ? (PORTD &(1<<(b)))!=0 : (PORTB &(1<<(b-8)))!=0 )

/* mapping of 6920's OUT pins to the bits in shift register */

#define OUT0  0x0002
#define OUT1  0x0004
#define OUT2  0x0008
#define OUT3  0x0010
#define OUT4  0x0020
#define OUT5  0x0040
#define OUT6  0x0080
#define OUT7  0x0100
#define OUT8  0x0200
#define OUT9  0x0400
#define OUT10 0x0800
#define OUT11 0x0001
                                    //   -- A -- 
#define vfd_segment_A     OUT11         //  |       |
#define vfd_segment_B     OUT8          //  F       B
#define vfd_segment_C     OUT2          //  |       |
#define vfd_segment_D     OUT3          //   -- G --
#define vfd_segment_E     OUT1          //  |       |  
#define vfd_segment_F     OUT6          //  E       C
#define vfd_segment_G     OUT9          //  |       |
                                    //   -- D --    

#define vfd_multiplexer_1    0x0020   // 10 h
#define vfd_multiplexer_2    0x0002   // 1  h
#define vfd_multiplexer_3    0x0040   // :
#define vfd_multiplexer_4    0x0100   // 10 min
#define vfd_multiplexer_5    0x0800  // 1  min

#define LED_1_PIN A1
#define LED_2_PIN A0
#define VFD_DATA_PIN 0
#define VFD_CLK_PIN 1
#define VFD_LOAD_PIN 2
//#define VFD_POWER_SWITCH_PIN 3
#define VFD_BLANK_PIN 4
#define VFD_HEAT1_PIN 5
#define VFD_HEAT2_PIN 6
#define LED_4_PIN 7
#define LED_3_PIN 8
#define BUTTON_1_PIN 9
#define BUTTON_2_PIN 10


#define POWERSENSE_PIN A2
#define BUZZER_PIN 3
#define VFD_POWER_SWITCH_PIN SDA1
#define POWER_MEASURE_PIN A7
#define TEMPERATURE_PIN A6

// Clarified listing of control states the watch can be in
const uint16_t DISPLAY_TIME = 0;
const uint16_t DISPLAY_DATE = 1;
const uint16_t DISPLAY_SECONDS = 2;
const uint16_t DISPLAY_DAY_OF_WEEK = 3;
const uint16_t DISPLAY_TEMPERATURE = 4;
const uint16_t ENTER_SETTINGS = 5;
const uint16_t SETTING_NAME_HOUR = 6;
const uint16_t SETTING_HOUR = 7;
const uint16_t SETTING_NAME_MINUTE = 8;
const uint16_t SETTING_MINUTE = 9;
const uint16_t SETTING_NAME_DAY_OF_WEEK = 10;
const uint16_t SETTING_DAY_OF_WEEK = 11;
const uint16_t SETTING_NAME_DAY_OF_MONTH = 12;
const uint16_t SETTING_DAY_OF_MONTH = 13;
const uint16_t SETTING_NAME_MONTH = 14;
const uint16_t SETTING_MONTH = 15;
const uint16_t SETTING_NAME_YEAR = 16;
const uint16_t SETTING_YEAR = 17;
const uint16_t SETTING_NAME_TEMPERATURE = 18;
const uint16_t SETTING_TEMPERATURE = 19;
const uint16_t STOPWATCH = 20;




const int BUTTON_HOLD_DURATION_MINIMUM = 1000; // milliseconds
const short COLON_BLINK_PERIOD = 250;          // also ms
const char NUM_CONTROL_STATES = 20;
const uint16_t TEMP_READ_INTERVAL = 1000;
const uint16_t BATTERY_READ_INTERVAL = 10;
const uint16_t BATTERY_READ_MAX_COUNT = 15;
const uint16_t SLEEP_TIMEOUT_INTERVAL = 25000;
const uint16_t LIGHT_LOW_MAX_VALUE = 100;
const uint16_t LIGHT_MED_MAX_VALUE = 200;
const uint16_t LIGHT_READ_INTERVAL = 1000;
const uint16_t HIGH_BATTERY_ADC_VALUE    = 302;//3.9    //275 3.7;//285;//760;
const uint16_t MEDIUM_BATTERY_ADC_VALUE  = 275;//3.7    //248 3.5;//269;
const uint16_t LOW_BATTERY_ADC_VALUE     = 251;//3.55   //221 3.3;//188;//254;
const uint16_t LOW_BATTERY_MESSAGE_DISPLAY_DURATION = 2000;
const uint16_t LED_FLASH_INTERVAL = 150;

uint16_t char_to_segments(char inputChar);
void set_vfd_cell(uint8_t cell_num, char character_to_set, bool include_colon);
void update_button_state();
void setDS3231time(byte second, byte minute, byte hour, 
byte dayOfWeek, byte dayOfMonth, byte month, byte year);
void readDS3231time(byte *second, byte *minute, byte *hour,
byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
void show_displayed_character_array();
void run_control_state();
void read_current_time();
void power_board_down(bool permit_wakeup);
void display_stopwatch();
void read_light_level();
void read_battery_level();
void flash_leds();

// second, minute, hour, dayOfWeek, dayOfMonth, month, year
byte current_time[7] = {0, 0, 0, 0, 0, 0, 0}; 
uint16_t button_state = 0;
volatile uint16_t control_state = 0;
volatile bool ignore_next_button_release = false;;
int button_hold_counts[2] = {0, 0};

volatile bool board_sleeping = false;
volatile bool repower_vfd = false;
volatile byte battery_level = 0;
float battery_adc_sum = 0;
float battery_adc_measurement_count = 0;
bool stopwatch_running = false;
byte adcsra, mcucr1, mcucr2;
uint16_t setting_value = 0;
char temperature_unit = ' ';
const int temperature_unit_eeprom_address = 16;
unsigned long last_temp_read = 0;
unsigned long last_light_read = 0;
char light_level = 0;
volatile unsigned long last_input_millis = 0;
volatile unsigned long wake_board_millis = 0;
unsigned long current_millis = 0;
volatile unsigned long last_battery_read_millis = 0;
int stopwatch_times[4] = {0, 0, 0, 0};
unsigned long colon_millis = 0;
//int setting_value_1 = 0;
//int setting_value_2 = 0;
bool colon_steady = false;
byte vfd_current_cell_id = 0;
int vfd_heat_counter = 0;
// TODO un-volatile
char vfd_displayed_characters[5] = {0, 0, 0, 0, 0};
bool vfd_blinking_noncolon_grids[4] = {false, false, false, false};
// 0-indexed list, 5th cell is [4]
const uint16_t vfd_cells[5] = {vfd_multiplexer_1, vfd_multiplexer_2, vfd_multiplexer_3, vfd_multiplexer_4, vfd_multiplexer_5};
const uint16_t MONTH_LENGTHS[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 

unsigned long duration_pins = 0;
unsigned long duration_not_pins = 0;
//VFDManager vfdManager;

void setup() {
  // Temperature
  pinMode(TEMPERATURE_PIN, INPUT);
  temperature_unit = EEPROM.read(temperature_unit_eeprom_address);
  // Button 1, Button 2
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);   // PCINT 1
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  // Voltage check
  pinMode(A2, INPUT);
  
  // LED 1, 2, 3, 4
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  // VFD futes tap
  pinMode(VFD_POWER_SWITCH_PIN, OUTPUT);
  digitalWrite(VFD_POWER_SWITCH_PIN, LOW);
  //pinMode(VFD_POWER_SWITCH_PIN, OUTPUT);
  // VFD futes pinek
  pinMode(VFD_HEAT1_PIN, OUTPUT);
  pinMode(VFD_HEAT2_PIN, OUTPUT);
  // VFD adat pinek
  pinMode(VFD_DATA_PIN, OUTPUT);
  pinMode(VFD_CLK_PIN, OUTPUT);
  pinMode(VFD_LOAD_PIN, OUTPUT);
  pinMode(VFD_BLANK_PIN, OUTPUT);

  pinMode(POWERSENSE_PIN, INPUT);
  pinMode(POWER_MEASURE_PIN, OUTPUT);
  digitalWrite(POWER_MEASURE_PIN, LOW);

  // Processor timer interrupt setup
  cli(); // Stop existing interrupts
  // Set timer1 interrupt at 50Hz
  TCCR1A = 0;// set entire TCCR0A register to 0
  TCCR1B = 0;// same for TCCR0B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 50Hz increments
  OCR1A = 16;//312;// = (16*10^6) / (50*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // Allow interrupts

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_board_down(true);
}

void loop() {
  current_millis = millis();
  //read_battery_level();
  if (battery_level == 4) read_battery_level();
  else if (battery_level == 3) power_board_down(false);
  else {
    if (!board_sleeping && repower_vfd) {
      digitalWrite(VFD_POWER_SWITCH_PIN, HIGH);
      if (battery_level != 2) digitalWrite(VFD_LOAD_PIN, HIGH);
      repower_vfd = false;
      wake_board_millis = current_millis;
      last_input_millis = current_millis;
    }
    // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
    // 3 - #1 held, 4 - #2 held, 5 - both held
    if (battery_level != 2) {
      update_button_state();
      read_current_time();
      select_control_state();
      read_light_level();
      //debug_4_digit(duration_pins, vfd_displayed_characters);
      show_displayed_character_array();
    }
    if (battery_level == 1 || battery_level == 2) flash_leds();
    
    // End of interactive loop; all necessary input from button has been registered, reset it
    if (button_state != 0) last_input_millis = current_millis;
    button_state = 0;
    colon_steady = false;
    if (current_millis - last_input_millis > SLEEP_TIMEOUT_INTERVAL && !board_sleeping) power_board_down(true);
  }
}

void update_button_state() {
  bool button_1_released = false;
  bool button_2_released = false;
  if (digitalRead(BUTTON_2_PIN) == LOW) button_hold_counts[0] += 1;
  else if (button_hold_counts[0] > 0) {
    button_1_released = true;
    button_hold_counts[0] = 0;
  }
  if (digitalRead(BUTTON_1_PIN) == LOW) button_hold_counts[1] += 1;
  else if (button_hold_counts[1] > 0){
    button_2_released = true;
    button_hold_counts[1] = 0;
  }

  if (button_hold_counts[0] > BUTTON_HOLD_DURATION_MINIMUM &&
      button_hold_counts[1] > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 5;
    ignore_next_button_release = true;    
  } else if (button_hold_counts[1] > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 4;
    ignore_next_button_release = true;
  } else if (button_hold_counts[0] > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 3;
    ignore_next_button_release = true;
  } else if (button_2_released && !ignore_next_button_release) {
    button_state = 2;
  } else if (button_2_released && ignore_next_button_release) {
    button_state = 0;
    ignore_next_button_release = false;
  } else if (button_1_released && !ignore_next_button_release) {
    button_state = 1;
  } else if (button_1_released && ignore_next_button_release) {
    button_state = 0;
    ignore_next_button_release = false;
  }
}
void read_current_time() {
  readDS3231time(&current_time[0], &current_time[1], &current_time[2], &current_time[3],
  &current_time[4], &current_time[5], &current_time[6]);
}
void select_control_state() {
  // Launch relevant state functions
  // Switch states dependent on button state
  switch(control_state) {
    case DISPLAY_TIME:
      display_hour_minute();
      if (button_state == 1) control_state = DISPLAY_DATE;
      else if (button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
      break;
    case DISPLAY_DATE:
      display_date();
      if (button_state == 1) control_state = DISPLAY_SECONDS;
      else if (button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
      break;
    case DISPLAY_SECONDS:
      display_seconds();
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
    break;
    case DISPLAY_DAY_OF_WEEK:
      display_day_of_week(current_time[3]);
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = DISPLAY_TEMPERATURE;
      break;
    case DISPLAY_TEMPERATURE:
      display_temperature();
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = DISPLAY_TIME;
      break;
    case STOPWATCH:
      display_stopwatch();
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) {
        if (stopwatch_running) {
          stopwatch_running = false;
          stopwatch_times[3] = current_time[4] - stopwatch_times[3];
          stopwatch_times[2] = current_time[2] - stopwatch_times[2];
          stopwatch_times[1] = current_time[1] - stopwatch_times[1];
          stopwatch_times[0] = current_time[0] - stopwatch_times[0];
        }
        else if (stopwatch_times[0] == 0 && stopwatch_times[1] == 0 && stopwatch_times[2] == 0) {
          stopwatch_running = true;
          stopwatch_times[0] = current_time[0];
          stopwatch_times[1] = current_time[1];
          stopwatch_times[2] = current_time[2];
          stopwatch_times[3] = current_time[4];
        } else {
          stopwatch_times[0] = 0;
          stopwatch_times[1] = 0;
          stopwatch_times[2] = 0;
          stopwatch_times[3] = 0; // day of month
        }
      }
      break;
    case ENTER_SETTINGS:
      vfd_displayed_characters[0] = 'S';
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 't';
      vfd_displayed_characters[4] = ' ';
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = SETTING_NAME_HOUR;
      break;
    case SETTING_NAME_HOUR:
      vfd_displayed_characters[0] = 'H';
      vfd_displayed_characters[1] = 'o';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'u';
      vfd_displayed_characters[4] = 'r';
      if (button_state == 1) { 
        control_state = SETTING_HOUR;
        setting_value = current_time[2];
      } else if (button_state == 2) control_state = SETTING_NAME_MINUTE;
      break;
    case SETTING_HOUR:
      vfd_displayed_characters[0] = setting_value / 10;
      vfd_displayed_characters[1] = setting_value % 10;
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = ' ';
      vfd_displayed_characters[4] = ' ';
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 24) setting_value = 0;
      }
      else if (button_state == 1) {
        setDS3231time(0, current_time[1], setting_value, current_time[3], current_time[4], current_time[5], current_time[6]);
        control_state = SETTING_NAME_MINUTE;
      }
      break;
    case SETTING_NAME_MINUTE:
      vfd_displayed_characters[0] = 'N';
      vfd_displayed_characters[1] = 'N';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'i';
      vfd_displayed_characters[4] = 'n';
      if (button_state == 1) { 
        control_state = SETTING_MINUTE;
        setting_value = current_time[1];
      }
      else if (button_state == 2) {
        control_state = SETTING_NAME_DAY_OF_WEEK;
        vfd_displayed_characters[3] = 'o';
      }
      break;
    case SETTING_MINUTE:
      vfd_displayed_characters[0] = ' ';
      vfd_displayed_characters[1] = ' ';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = setting_value / 10;
      vfd_displayed_characters[4] = setting_value % 10;
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 60) setting_value = 0;
      }
      else if (button_state == 1) {
        setDS3231time(0, setting_value, current_time[2], current_time[3], current_time[4], current_time[5], current_time[6]);
        control_state = SETTING_NAME_DAY_OF_WEEK;
      }
      break;
    case SETTING_NAME_DAY_OF_WEEK:
      vfd_displayed_characters[0] = 'd';
      vfd_displayed_characters[1] = 'o';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'U';
      vfd_displayed_characters[4] = 'U';
      if (button_state == 1) { 
        control_state = SETTING_DAY_OF_WEEK;
        setting_value = current_time[3];
      }
      else if (button_state == 2) control_state = SETTING_NAME_DAY_OF_MONTH;
      break;
    case SETTING_DAY_OF_WEEK:
    display_day_of_week(setting_value);
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 8) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], setting_value, current_time[4], current_time[5], current_time[6]);
        control_state = SETTING_NAME_DAY_OF_MONTH;
      }
      break;
    case SETTING_NAME_DAY_OF_MONTH:
      vfd_displayed_characters[0] = 'd';
      vfd_displayed_characters[1] = 'o';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'N';
      vfd_displayed_characters[4] = 'N';
      if (button_state == 1) { 
        control_state = SETTING_DAY_OF_MONTH;
        setting_value = current_time[4];
      }
      else if (button_state == 2) control_state = SETTING_NAME_MONTH;
      break;
    case SETTING_DAY_OF_MONTH:
      vfd_displayed_characters[0] = ' ';
      vfd_displayed_characters[1] = ' ';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = setting_value / 10;
      vfd_displayed_characters[4] = setting_value % 10;
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == (MONTH_LENGTHS[current_time[5]] + 1)) setting_value = 1;
        // Leap day condition
        if (current_time[6] % 4 == 0 && current_time[5] == 2 && setting_value == 29) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], setting_value, current_time[5], current_time[6]);
        control_state = SETTING_NAME_MONTH;
      }
      break;
    case SETTING_NAME_MONTH:
      vfd_displayed_characters[0] = 'N';
      vfd_displayed_characters[1] = 'N';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'o';
      vfd_displayed_characters[4] = 'n';
      if (button_state == 1) { 
        control_state = SETTING_MONTH;
        setting_value = current_time[5];
      }
      else if (button_state == 2) control_state = SETTING_NAME_YEAR;
      break;
    case SETTING_MONTH:
      vfd_displayed_characters[0] = setting_value / 10;
      vfd_displayed_characters[1] = setting_value % 10;
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = ' ';
      vfd_displayed_characters[4] = ' ';
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 13) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], current_time[4], setting_value, current_time[6]);
        control_state = SETTING_NAME_YEAR;
      }
      break;
    case SETTING_NAME_YEAR:
      vfd_displayed_characters[0] = 'Y';
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'A';
      vfd_displayed_characters[4] = 'r';
      if (button_state == 1) { 
        control_state = SETTING_YEAR;
        setting_value = current_time[6];
      }
      else if (button_state == 2) control_state = SETTING_NAME_TEMPERATURE;
      break;
    case SETTING_YEAR:
      vfd_displayed_characters[0] = '2';
      vfd_displayed_characters[1] = '0';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = setting_value / 10;
      vfd_displayed_characters[4] = setting_value % 10;
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 100) setting_value = 20;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], current_time[4], current_time[5], setting_value);
        control_state = SETTING_NAME_TEMPERATURE;
      }
      break;
    case SETTING_NAME_TEMPERATURE:
      vfd_displayed_characters[0] = 't';
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'n';
      vfd_displayed_characters[4] = 'n';
      if (button_state == 1) control_state = SETTING_TEMPERATURE;
      else if (button_state == 2) control_state = SETTING_NAME_HOUR;
      break;
    case SETTING_TEMPERATURE:
      vfd_displayed_characters[0] = ' ';
      vfd_displayed_characters[1] = ' ';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = '*'; 
      vfd_displayed_characters[4] = temperature_unit;
      if (button_state == 1) {
        control_state = SETTING_NAME_HOUR;
        EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
      }
      else if (button_state == 2) {
        if (temperature_unit == 'C') temperature_unit = 'F';
        else temperature_unit = 'C'; 
      }
      break;
  }
  if (button_state == 4) control_state = ENTER_SETTINGS;
  else if (button_state == 3) control_state = STOPWATCH;
  if (battery_level == 1 && current_millis - wake_board_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
    /*vfd_displayed_characters[0] = 'L';
    vfd_displayed_characters[1] = 'b';
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = 'A'; 
    vfd_displayed_characters[4] = 't';*/
    vfd_displayed_characters[0] = 'B';
    vfd_displayed_characters[1] = 'A';
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = 'L'; 
    vfd_displayed_characters[4] = 'o';
  }
}
void display_stopwatch() {
  if (stopwatch_running) {
    int elapsed_seconds = ((current_time[2] - stopwatch_times[2]) * 3600) + ((current_time[1] - stopwatch_times[1]) * 60) + (current_time[0] - stopwatch_times[0]);
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99) elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    vfd_displayed_characters[0] = elapsed_minutes / 10;
    vfd_displayed_characters[1] = elapsed_minutes % 10;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = remaining_seconds / 10; 
    vfd_displayed_characters[4] = remaining_seconds % 10;
  } else if (stopwatch_times[0] != 0 || stopwatch_times[1] != 0 || stopwatch_times[2] != 0){
    colon_steady = true;
    int elapsed_seconds = (stopwatch_times[2] * 3600) + (stopwatch_times[1] * 60) + stopwatch_times[0];
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99) elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    vfd_displayed_characters[0] = elapsed_minutes / 10;
    vfd_displayed_characters[1] = elapsed_minutes % 10;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = remaining_seconds / 10; 
    vfd_displayed_characters[4] = remaining_seconds % 10;
  } else {
    colon_steady = true;
    vfd_displayed_characters[0] = 0;
    vfd_displayed_characters[1] = 0;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = 0; 
    vfd_displayed_characters[4] = 0;
  }
}
void display_hour_minute() {
  char hour_digit_1   = current_time[2] / 10;
  char hour_digit_2   = current_time[2] % 10;
  char minute_digit_1 = current_time[1] / 10;
  char minute_digit_2 = current_time[1] % 10;
  vfd_displayed_characters[0] = hour_digit_1;
  vfd_displayed_characters[1] = hour_digit_2;
  vfd_displayed_characters[2] = 1;
  vfd_displayed_characters[3] = minute_digit_1;
  vfd_displayed_characters[4] = minute_digit_2;
}
void display_date() {
  colon_steady = true;
  char month_digit_1   = current_time[5] / 10;
  char month_digit_2   = current_time[5] % 10;
  char day_of_month_digit_1 = current_time[4] / 10;
  char day_of_month_digit_2 = current_time[4] % 10;
  vfd_displayed_characters[0] = month_digit_1;
  vfd_displayed_characters[1] = month_digit_2;
  vfd_displayed_characters[2] = 1;
  vfd_displayed_characters[3] = day_of_month_digit_1;
  vfd_displayed_characters[4] = day_of_month_digit_2;
}
void display_seconds() {
  char seconds_digit_1 = current_time[0] / 10;
  char seconds_digit_2 = current_time[0] % 10;
  vfd_displayed_characters[0] = ' ';
  vfd_displayed_characters[1] = ' ';
  vfd_displayed_characters[2] = ' ';
  vfd_displayed_characters[3] = seconds_digit_1;
  vfd_displayed_characters[4] = seconds_digit_2;
}
void display_day_of_week(byte day) {
  switch(day) {
    case 1:
      vfd_displayed_characters[0] = 'N';
      vfd_displayed_characters[1] = 'N';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'o';
      vfd_displayed_characters[4] = 'n';
    break;
    case 2:
      vfd_displayed_characters[0] = 't';
      vfd_displayed_characters[1] = 'u';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'E';
      vfd_displayed_characters[4] = ' ';
    break;
    case 3:
      vfd_displayed_characters[0] = 'U';
      vfd_displayed_characters[1] = 'U';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'E';
      vfd_displayed_characters[4] = 'd';
    break;
    case 4:
      vfd_displayed_characters[0] = 't';
      vfd_displayed_characters[1] = 'h';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'u';
      vfd_displayed_characters[4] = ' ';
    break;
    case 5:
      vfd_displayed_characters[0] = 'F';
      vfd_displayed_characters[1] = 'r';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'i';
      vfd_displayed_characters[4] = ' ';
    break;
    case 6:
      vfd_displayed_characters[0] = 's';
      vfd_displayed_characters[1] = 'A';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 't';
      vfd_displayed_characters[4] = ' ';
    break;
    case 7:
      vfd_displayed_characters[0] = 'S';
      vfd_displayed_characters[1] = 'u';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'n';
      vfd_displayed_characters[4] = ' ';
    break;
  }
}
void display_temperature() {
  float current_temp = read_adc_to_celsius();
  if (temperature_unit == 'F') {
    current_temp = celsius_to_fahrenheit(current_temp);
    uint8_t fh_hundreds = current_temp / 100;
    uint8_t fh_tens     = int(current_temp / 10) % 10;
    uint8_t fh_ones     = int(current_temp)  % 10;
    vfd_displayed_characters[0] = fh_hundreds;
    vfd_displayed_characters[1] = fh_tens;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = fh_ones;
  } else {
    colon_steady = true;
    uint8_t celsius_tens   = current_temp / 10;
    uint8_t celsius_ones   = int(current_temp) % 10;
    vfd_displayed_characters[0] = celsius_tens;
    vfd_displayed_characters[1] = celsius_ones;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = '*';
  }
  vfd_displayed_characters[4] = temperature_unit;
}

// This function runs when the board's sleep is interrupted by button 1 press
// Declare all variables modified by this as "volatile"
ISR(PCINT0_vect) {
  // Flag to indicate first button press (waking up the board)
  // This button press should not be processed for normal state changes
  if (board_sleeping) {
    ignore_next_button_release = true;
    repower_vfd = true;
    board_sleeping = false;
    battery_level = 4;
    sleep_disable();
    ADCSRA = adcsra;
    control_state = DISPLAY_TIME;
    last_input_millis = current_millis;
    last_battery_read_millis = 0;
    TIMSK1 |= (1 << OCIE1A);
    
    digitalWrite(POWER_MEASURE_PIN, HIGH);
  }
}
void power_board_down(bool permit_wakeup) {
  // Save current setting data
  switch (control_state) {
    case SETTING_HOUR:
      setDS3231time(0, current_time[1], setting_value, current_time[3], current_time[4], current_time[5], current_time[6]);
      break;
    case SETTING_MINUTE:
      setDS3231time(0, setting_value, current_time[2], current_time[3], current_time[4], current_time[5], current_time[6]);
      break;
    case SETTING_DAY_OF_WEEK:
      setDS3231time(current_time[0], current_time[1], current_time[2], setting_value, current_time[4], current_time[5], current_time[6]);
      break;
    case SETTING_DAY_OF_MONTH:
      setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], setting_value, current_time[5], current_time[6]);
      break;
    case SETTING_MONTH:
      setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], current_time[4], setting_value, current_time[6]);
      break;
    case SETTING_YEAR:
      setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], current_time[4], current_time[5], setting_value);
      break;
    case SETTING_TEMPERATURE:
      EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
      break;
  }
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  digitalWrite(VFD_CLK_PIN, LOW);
  digitalWrite(VFD_LOAD_PIN, LOW);
  digitalWrite(VFD_DATA_PIN, LOW);
  digitalWrite(VFD_BLANK_PIN, LOW);
  digitalWrite(VFD_POWER_SWITCH_PIN, LOW);
  digitalWrite(VFD_HEAT1_PIN, LOW);
  digitalWrite(VFD_HEAT2_PIN, LOW);
  digitalWrite(POWER_MEASURE_PIN, LOW);
  board_sleeping = true;
  battery_adc_sum = 0;
  battery_adc_measurement_count = 0;
  // Set sleep wakeup interrupts
  if (permit_wakeup) {
    PCICR  |= 0b00000001;   // turn on port b for PCINTs
    PCMSK0 |= 0b00000100;   // turn on PCINT 2 mask
  } else {
    PCICR  = 0b00000000;    // turn off PCINT interrupt
    PCMSK0 = 0b00000000;
  }
  
  adcsra = ADCSRA;               //save the ADC Control and Status Register A
  ADCSRA = 0;                    //disable ADC
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);
  MCUCR = mcucr1;
  MCUCR = mcucr2;
  // Disable vfd heating timer interrupt
  TIMSK1 = 0;
  sleep_enable();
  sleep_cpu();                   //go to sleep
}


void show_displayed_character_array() {
  if (vfd_current_cell_id == 2) {
    vfd_current_cell_id += 1;
}
  bool include_colon = ((current_millis - colon_millis) < COLON_BLINK_PERIOD || colon_steady) && vfd_displayed_characters[2] != ' ';
  // Group the colon light with turning on grid 3, grid 1 if 3 is empty
  if (vfd_displayed_characters[3] == ' ') {
    if (vfd_current_cell_id != 1) include_colon = false;
  } else if (vfd_current_cell_id != 3) include_colon = false;
  
  set_vfd_cell(vfd_current_cell_id, vfd_displayed_characters[vfd_current_cell_id], include_colon);
  vfd_current_cell_id += 1;
  if (current_millis - colon_millis > 2 * COLON_BLINK_PERIOD) colon_millis = current_millis;
  if (vfd_current_cell_id == 5) vfd_current_cell_id = 0;
}

ISR(TIMER1_COMPA_vect){ //timer1 interrupt 50Hz toggles pin 5, 6
  if ((vfd_heat_counter >= 4 && vfd_heat_counter < 10) || vfd_heat_counter >= 14){
    digitalWrite(VFD_HEAT1_PIN, LOW);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  } else if (vfd_heat_counter < 4) {
    digitalWrite(VFD_HEAT1_PIN, HIGH);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  } else if (vfd_heat_counter >= 10 && vfd_heat_counter < 14){
    digitalWrite(VFD_HEAT1_PIN, HIGH);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  }
  vfd_heat_counter += 1;
  if (vfd_heat_counter == 20) {
      vfd_heat_counter = 0;
  }
}
/*
 * int percent = 10 * ((millis() / 2000) % 10);
  int secondd = 10;
  int last = 20;
  int first = secondd * percent / 100;
  int third = last * percent /100 ;
  if ((heat_counter >= first && heat_counter < secondd) || heat_counter >= third){
    digitalWrite(VFD_HEAT1_PIN, LOW);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  } else if (heat_counter < first) {
    digitalWrite(VFD_HEAT1_PIN, HIGH);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  } else if (heat_counter >= secondd && heat_counter < third){
    digitalWrite(VFD_HEAT1_PIN, HIGH);
    digitalWrite(VFD_HEAT2_PIN, LOW);
  }
  heat_counter += 1;
  if (heat_counter == last) {
      heat_counter = 0;
  }
*/

unsigned long check_point = 0;

void set_vfd_cell(uint8_t cell_num, char character_to_set, bool include_colon) {
  if (cell_num > 4) return;
  unsigned long current_millis__ = micros();
  
  uint16_t segment_pattern = char_to_segments(character_to_set);
  segment_pattern |= vfd_cells[cell_num];
  if (include_colon) segment_pattern |= vfd_cells[2];
  digitalWrite(VFD_BLANK_PIN, HIGH);
  bool out_bit = 0;
  duration_not_pins = current_millis__  - check_point;
  check_point = current_millis__ ;
  for (char i = 0; i < 12; i++) {
    out_bit = 1 & (segment_pattern >> i);
    // Load data to pin
    if (out_bit) digitalWrite(VFD_DATA_PIN, HIGH);
    else digitalWrite(VFD_DATA_PIN, LOW);
    /*if (out_bit) clrPin(VFD_DATA_PIN);
    else setPin(VFD_DATA_PIN);*/
    
    // Trigger write: CLK pin High->Low
    //digitalWrite(VFD_LOAD_PIN, LOW);
    //digitalWrite(VFD_LOAD_PIN, HIGH);
   
    clrPin(VFD_LOAD_PIN);
    setPin(VFD_LOAD_PIN);
    //digitalWrite(VFD_CLK_PIN, HIGH);
    //digitalWrite(VFD_CLK_PIN, LOW);
    setPin(VFD_CLK_PIN); // else
    clrPin(VFD_CLK_PIN);
    // Trigger shift: LOAD pin Low->High
    
  }
  /*for (char k = 0; k<80; k++){
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_1_PIN, HIGH);
  }*/
  // Show output to display
  /*if (light_level == 0) digitalWrite(4, LOW);
  else analogWrite(4, light_level);*/
  analogWrite(VFD_BLANK_PIN, light_level);
  //digitalWrite(VFD_BLANK_PIN, LOW);
  duration_pins = micros() - check_point;
}

uint16_t char_to_segments(char inputChar) {
  uint16_t outputSegCode = 0x0000;
  switch(inputChar) {
    case 0:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F;
    break;
    case 1:
      outputSegCode = vfd_segment_B | vfd_segment_C;
    break;
    case 2:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_G | vfd_segment_E | vfd_segment_D;
    break;
    case 3:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_G | vfd_segment_D;
    break;
    case 4:
      outputSegCode = vfd_segment_B | vfd_segment_C | vfd_segment_F | vfd_segment_G;
    break;
    case 5:
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case 6:
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 7:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C;
    break;
    case 8:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 9:
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case '0':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F;
    break;
    case '1':
      outputSegCode = vfd_segment_B | vfd_segment_C;
    break;
    case '2':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_G | vfd_segment_E | vfd_segment_D;
    break;
    case '3':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_G | vfd_segment_D;
    break;
    case '4':
      outputSegCode = vfd_segment_B | vfd_segment_C | vfd_segment_F | vfd_segment_G;
    break;
    case '5':
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case '6':
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case '7':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C;
    break;
    case '8':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case '9':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case ' ':
      outputSegCode = 0;
    break;
    case 'c':
      outputSegCode = vfd_segment_G | vfd_segment_E | vfd_segment_D;
    break;
    case 'C':
      outputSegCode = vfd_segment_A | vfd_segment_E | vfd_segment_F | vfd_segment_D;
    break;
    case 'u':
      outputSegCode = vfd_segment_E | vfd_segment_D | vfd_segment_C;
    break;
    case 'w':
      outputSegCode = vfd_segment_D | vfd_segment_C;
    break;
    case 'U':
      outputSegCode = vfd_segment_E | vfd_segment_D | vfd_segment_C | vfd_segment_B | vfd_segment_F;
    break;
    case 'W':
      outputSegCode = vfd_segment_D | vfd_segment_C | vfd_segment_B;
    break;
    case 'h':
      outputSegCode = vfd_segment_F | vfd_segment_E | vfd_segment_G | vfd_segment_C;
    break;
    case 'H':
      outputSegCode = vfd_segment_F | vfd_segment_E | vfd_segment_G | vfd_segment_C | vfd_segment_B;
      break;
    case 'o':
      outputSegCode = vfd_segment_E | vfd_segment_G | vfd_segment_C | vfd_segment_D;
    break;
    case 't':
      outputSegCode = vfd_segment_F | vfd_segment_E | vfd_segment_D | vfd_segment_G;
    break;
    case 'r':
      outputSegCode = vfd_segment_E | vfd_segment_G;
    break;
    case 'n':
      outputSegCode = vfd_segment_E | vfd_segment_G | vfd_segment_C;
    break;
    case 'N':
      outputSegCode = vfd_segment_E | vfd_segment_A | vfd_segment_C | vfd_segment_B | vfd_segment_F;
    break;
    case 'M':
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_B;
    break;
    case 'm':
      outputSegCode = vfd_segment_G | vfd_segment_C;
    break;
    case 'f':
      outputSegCode = vfd_segment_A | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'F':
      outputSegCode = vfd_segment_A | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'i':
      outputSegCode = vfd_segment_E;
    break;
    case 'E':
      outputSegCode = vfd_segment_A | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'e':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'S':
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case 's':
      outputSegCode = vfd_segment_A | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case 'y':
      outputSegCode = vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_F | vfd_segment_G;
    break;
    case 'Y':
      outputSegCode = vfd_segment_B | vfd_segment_C | vfd_segment_F | vfd_segment_G;
    break;
    case 'A':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'I':
      outputSegCode = vfd_segment_B | vfd_segment_C;
    break;
    case 'd':
      outputSegCode = vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_G;
    break;
    case 'B':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'b':
      outputSegCode = vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
    break;
    case 'L':
      outputSegCode = vfd_segment_D | vfd_segment_E | vfd_segment_F;
    break;
    case '*':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_F | vfd_segment_G;
    break;
    //vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F | vfd_segment_G;
  }
  return outputSegCode;
}
void readDS3231time(byte *second, byte *minute, byte *hour,
byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
byte decToBcd(byte val) {
  return( (val/10*16) + (val%10) );
}
byte bcdToDec(byte val) {
  return( (val/16*10) + (val%16) );
}
void read_light_level() {
  if (current_millis - last_light_read > LIGHT_READ_INTERVAL || last_light_read == 0) {
    last_light_read = current_millis;
    uint16_t adc_light_level = analogRead(A7);
    if (adc_light_level < LIGHT_LOW_MAX_VALUE) {
    //vfd_displayed_characters[0] = '0';
    light_level = 200; // duty cycle is 0-255, our signal is LOW
    }
    else if (adc_light_level < LIGHT_MED_MAX_VALUE) {
      //vfd_displayed_characters[0] = '1'; 
      light_level = 100;
    }
    else {
      //vfd_displayed_characters[0] = '2'; 
      light_level = 0;
    }
  }
}
float read_adc_to_celsius() {
  if (current_millis - last_temp_read > TEMP_READ_INTERVAL || last_temp_read == 0) {
    last_temp_read = current_millis;
    float temp = (analogRead(TEMPERATURE_PIN) * 3.3) / 1024.0;
    temp -= 0.5;         // Set to 0°C
    temp = temp / 0.01;  // Scale 10 mv / °C
    return temp;
  }
}
void read_battery_level() {
  // 5V = 1024, 0V = 0
  // Battery level: above 3.7V: 0, 3.7-3.5: 1, 3.5-3.3: 2, 3.3- 3
  if (current_millis - last_battery_read_millis > BATTERY_READ_INTERVAL || last_battery_read_millis == 0) {
    last_battery_read_millis = current_millis;
    uint16_t bat_level_adc = analogRead(POWERSENSE_PIN);
    if (battery_adc_measurement_count < BATTERY_READ_MAX_COUNT){
      battery_level == 4;
      battery_adc_sum += bat_level_adc;
      battery_adc_measurement_count += 1;
      if (battery_adc_measurement_count == BATTERY_READ_MAX_COUNT) battery_adc_sum = battery_adc_sum / battery_adc_measurement_count;
    } else if (battery_adc_sum > HIGH_BATTERY_ADC_VALUE)   battery_level = 0;
    else if (battery_adc_sum > MEDIUM_BATTERY_ADC_VALUE)   battery_level = 1;
    else if (battery_adc_sum > LOW_BATTERY_ADC_VALUE)      battery_level = 2;
    else battery_level = 3;
  }
}
void flash_leds() {
  int led_millis = current_millis - wake_board_millis;
  if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION && led_millis % (2*LED_FLASH_INTERVAL) < LED_FLASH_INTERVAL) {
    digitalWrite(LED_1_PIN, HIGH);
    digitalWrite(LED_2_PIN, HIGH);
    digitalWrite(LED_3_PIN, HIGH);
    digitalWrite(LED_4_PIN, HIGH);
  } else if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_2_PIN, LOW);
    digitalWrite(LED_3_PIN, LOW);
    digitalWrite(LED_4_PIN, LOW);
  }
}
float celsius_to_fahrenheit(float celsius) {
  float fh = (celsius * 9.0) / 5.0;
  fh += 32;
  return fh;
}
