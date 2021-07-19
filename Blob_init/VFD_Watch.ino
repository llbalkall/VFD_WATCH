#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 

#include <avr/io.h> 
//#include <avr/interrupt.h>

#include <math.h>

#define DS3231_I2C_ADDRESS 0x68
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
#define vfd_segment_A     OUT11
#define vfd_segment_B     OUT8
#define vfd_segment_C     OUT2
#define vfd_segment_D     OUT3
#define vfd_segment_E     OUT1
#define vfd_segment_F     OUT6
#define vfd_segment_G     OUT9
#define vfd_multiplexer_1    OUT4   // 10 h
#define vfd_multiplexer_2    OUT0   // 1  h
#define vfd_multiplexer_3    OUT5   // :
#define vfd_multiplexer_4    OUT7   // 10 min
#define vfd_multiplexer_5    OUT10  // 1  min

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
#define ALARM_INTERRUPT A3 
//#define BUZZER_PIN 21

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
const uint16_t PARTY_TIME = 21;
const uint16_t SETTING_NAME_ALARM = 22;
const uint16_t SETTING_ALARM_MODE = 23;
const uint16_t SETTING_ALARM_HOUR = 24;
const uint16_t SETTING_ALARM_MINUTE = 25;
const uint16_t SETTING_NAME_TIME = 26;
const uint16_t SETTING_NAME_DATE = 27;
const uint16_t SETTING_DATE_ORDER = 28;
const uint16_t SETTING_SCREEN_TIMEOUT_NAME = 29; 
const uint16_t SETTING_SCREEN_TIMEOUT = 30;
const uint16_t ALARM = 31;
const uint16_t LOW_BATTERY = 32;
const uint16_t SNOOZE_MESSAGE = 33;

const int BUTTON_HOLD_DURATION_MINIMUM = 1000; // milliseconds
const short COLON_BLINK_PERIOD = 250;          // also ms
const char NUM_CONTROL_STATES = 20;
const uint16_t TEMP_READ_INTERVAL = 1000;
const uint16_t BATTERY_READ_INTERVAL = 10;
const uint16_t BATTERY_READ_MAX_COUNT = 30;
uint16_t sleep_timout_interval = 5000;
const uint16_t SLEEP_TIMEOUT_INTERVAL = 5000;
const uint16_t PARTY_INIT_INTERVAL = 1000;  // How long we write "Party" befor setting the duration of it
const uint16_t PARTY_SET_INTERVAL = SLEEP_TIMEOUT_INTERVAL - 500; //How long do we wait before we consider it set in the party_setting should be 
const uint16_t LIGHT_LOW_MAX_VALUE = 100;
const uint16_t LIGHT_MED_MAX_VALUE = 200;
const uint16_t LIGHT_READ_INTERVAL = 1000;
/*  I measured this stuff, if link doesn't work, it's probably like 2080.
 *  https://docs.google.com/spreadsheets/d/1mv_m5flVV4PhzLbO76UwapgEKzr8os-OZrgATmgtAh4/edit?usp=sharing
 *  adc_value ~= 67.99 * Battery_voltage + 27.12 (between 3.55 V and 4.3 V) */
const uint16_t HIGH_BATTERY_ADC_VALUE    = 100;//285;//3.80
const uint16_t MEDIUM_BATTERY_ADC_VALUE  = 90;//280;//3.72
const uint16_t LOW_BATTERY_ADC_VALUE     = 80;//275;//3.65
const uint16_t LOW_BATTERY_MESSAGE_DISPLAY_DURATION = 2000;
const uint16_t ALARM_DURATION = 5000;
const uint16_t LED_FLASH_INTERVAL = 150;
const uint16_t BUZZER_BUZZ_INTERVAL = 10;  // micro sec

uint16_t char_to_segments(char inputChar);
void set_vfd_cell(uint8_t cell_num, uint16_t segment_pattern, bool include_colon);
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
//void read_light_level();
void read_battery_level();
void flash_leds();
void buzz_buzzer(); //unused function
void setup_interrupts();
void select_control_state();
void debug();
void alarm();

void debug_4_digit(float n);
void display_hertz_of_function();

// second, minute, hour, dayOfWeek, dayOfMonth, month, year
byte current_time[7] = {0, 0, 0, 0, 0, 0, 0}; 
uint16_t button_state = 0;
volatile uint16_t control_state = 0;
volatile uint16_t ignore_next_button_releases = 0;
int button_hold_counts[2] = {0, 0};

volatile bool board_sleeping = false;
volatile bool repower_vfd = false;
volatile byte battery_level = 4;
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
volatile unsigned long alarm_start_millis = 0;
unsigned long current_millis = 0;
unsigned long current_micros = 0;  //only for buzzer
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
const int PARTY_TIMES_SIZE = 7;
const uint16_t PARTY_TIMES[PARTY_TIMES_SIZE] = {1, 5, 10, 20, 30, 45, 60};
int party_time = 1;
bool party_time_is_on = false;
unsigned long party_init_millis = 0;  
unsigned long party_battery_measure_millis = 0;
volatile bool display_debug = false; //if true then a debug_float will be displayed
volatile float debug_float = 1234.0;
volatile float hertz_counter = 0.0;//for measuring the hertz of the interrupt ISR function
volatile bool buzzer_is_on = false; 
volatile bool buzzing = false;
bool month_before_day = true;

//TODO place this where belong
volatile bool alarm_flag = false;
void setup() {
  //alarm_flag=true;
  //buzzer_is_on = true;
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  /*while (true){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1); //1000 = 2s, 500 = 1s, 0.5 = 1/1000s
    digitalWrite(BUZZER_PIN, LOW);
    delay(1);
  }*/
  // Temperature
  pinMode(TEMPERATURE_PIN, INPUT);
  temperature_unit = EEPROM.read(temperature_unit_eeprom_address);
  // Button 1, Button 2
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);   // PCINT 1
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  // Voltage check
  pinMode(POWERSENSE_PIN, INPUT);
  pinMode(POWER_MEASURE_PIN, OUTPUT);
  digitalWrite(POWER_MEASURE_PIN, LOW);
  // LED 1, 2, 3, 4
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  // VFD futes tap
  pinMode(VFD_POWER_SWITCH_PIN, OUTPUT);
  digitalWrite(VFD_POWER_SWITCH_PIN, LOW);
  // VFD futes pinek
  pinMode(VFD_HEAT1_PIN, OUTPUT);
  pinMode(VFD_HEAT2_PIN, OUTPUT);
  // VFD adat pinek
  pinMode(VFD_DATA_PIN, OUTPUT);
  pinMode(VFD_CLK_PIN, OUTPUT);
  pinMode(VFD_LOAD_PIN, OUTPUT);
  pinMode(VFD_BLANK_PIN, OUTPUT);

  pinMode(ALARM_INTERRUPT, INPUT_PULLUP);
  
  setup_interrupts();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_board_down(true);
}

volatile unsigned long milliss = 0;

void loop() {
  
  if (!board_sleeping){
    /*if (alarm_flag) digitalWrite(LED_2_PIN, HIGH);
    else digitalWrite(LED_2_PIN, LOW);*/
    /*digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_4_PIN, LOW);
    digitalWrite(LED_2_PIN, LOW);
    digitalWrite(LED_3_PIN, LOW);*/
    //current_millis = millis();
    current_millis = milliss/2;
    
    read_battery_level();
    
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
        //digitalWrite(LED_4_PIN, HIGH);
        
        update_button_state();
        read_current_time();
        select_control_state();
        //read_light_level();
        //debug();
        alarm();
        led_effect();
        //show_displayed_character_array();
      }
      //led_effect();
      if (battery_level == 1 || battery_level == 2) {
        if (party_time_is_on) {
          //power_board_down(true);
          party_time_is_on = false;
        } /*else {
          //flash_leds();
          //effect_state = SEC_BLINK_EFFECT;  
        }*/
        /*if (battery_level == 1) digitalWrite(LED_1_PIN, HIGH);
        if (battery_level == 2) digitalWrite(LED_2_PIN, HIGH);*/
      }
      
      // End of interactive loop; all necessary input from button has been registered, reset it
      if (button_state != 0) last_input_millis = current_millis;
      button_state = 0;
      if (control_state != STOPWATCH) colon_steady = false;
      float sleep_timeout;
      if (party_time_is_on){
        sleep_timeout = party_time * 500 * 60.0; // It's 500 and not 1000 because 500 is a second here
        if (party_time == 69) sleep_timeout = current_millis - last_input_millis; //never gonna reach it
      } else {
        sleep_timeout = /*SLEEP_TIMEOUT_INTERVAL*/ sleep_timout_interval * 1.0;
      }

      
      
      if (current_millis - last_input_millis > /*SLEEP_TIMEOUT_INTERVAL*/ sleep_timeout && !board_sleeping && !alarm_flag){
        if (party_time_is_on) party_time_is_on = false;
        if (!board_sleeping) power_board_down(true);
        //TODO also check if we are in the party_time_setting if so then go to the DISPLAY_TIME mode
        /*if (control_state == PARTY_TIME) control_state = DISPLAY_TIME;
        else {
          if (party_time_is_on) party_time_is_on = false;
          power_board_down(true); 
        }*/
      }
    }
  } else {
    digitalWrite(LED_1_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_1_PIN, LOW);
    delay(1000);
  }
}

//TODO: find the place for this
int effect_state = 0;

const int NO_EFFECT = 0;
const int SEC_FLASH_EFFECT = 1;
const int SEC_BLINK_EFFECT = 2;

void led_effect(){
  long t = current_millis - wake_board_millis;
  effect_state = NO_EFFECT;
  if (party_time_is_on) effect_state = SEC_FLASH_EFFECT;
  if (alarm_flag) effect_state = SEC_BLINK_EFFECT;
  if ((battery_level == 1 || battery_level == 2) && t < LOW_BATTERY_MESSAGE_DISPLAY_DURATION){
    effect_state = SEC_BLINK_EFFECT;
  }
  switch(effect_state) {
    case NO_EFFECT:
        //leds_go(-25, 100, 500);
        leds_turn_off();   //just in case
        //leds_go(0, 100, 500);
        leds_go(-25, 100, 500);
      break;
    case SEC_FLASH_EFFECT:
        leds_go(-25, 100, 500);
      break;
    case SEC_BLINK_EFFECT:
        leds_go(0, 100, 500);
      break;
  } 
}

void leds_turn_off(){
  digitalWrite(LED_1_PIN, LOW);    
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
}

void leds_go(int delta, int wave_length, int window){
  wave_length*=2;
  delta = 0;
  a_led(LED_1_PIN, 0, wave_length, window);
  a_led(LED_2_PIN, delta, wave_length, window);
  a_led(LED_3_PIN, 2*delta, wave_length, window);
  a_led(LED_4_PIN, 3*delta, wave_length, window);
  /* some good values: -25, 100, 500  -- One 4 length snake per second
   *  
    */
}

void a_led(int pin, int delta, int wave_length, int window){
  long t = current_millis - wake_board_millis;
  if ((t + delta) % (window) < wave_length){
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

volatile int alarm_counter = 0;
bool alarm_sound = true;
//TODO find a place for this code
void alarm(){
  //if (!alarm_flag) alarm_start_millis = current_millis;
  long t = current_millis - //wake_board_millis;
                            alarm_start_millis;
  //buzz_for_alarm();
  if (alarm_flag){
    if (alarm_sound) buzz_for_alarm();
    //flash_leds_for_alarm(0);
    //TODO buzz_for_alarm();
    if (t> ALARM_DURATION) {
      //digitalWrite(LED_2_PIN, HIGH);
      alarm_flag = false;
      clearAlarmStatusBits();
      buzzer_is_on=false;
      if (alarm_counter>2){
        //alarm_counter=0;
        //reset_real_alarm();
        
      }
    }
  } 
}

//TODO
void buzz_for_alarm(){
  //if (!alarm_flag) alarm_start_millis = current_millis;
  long t = current_millis - //wake_board_millis;
                            alarm_start_millis;
  if (t<ALARM_DURATION){
    if (t%500<250){
      buzzer_is_on = true;
    } else {
      buzzer_is_on = false;  
    } 
  } else {
    if (t<ALARM_DURATION*2){
      if (t%500<250 && t%63<31){
        buzzer_is_on = true;
      } else {
        buzzer_is_on = false;  
      }
    } else {
      buzzer_is_on = false;  
    }
  }
}

void set_alarm_for_snooze(){
  //current_time[1] minute;   [2] hour
  digitalWrite(LED_1_PIN, HIGH);
  int snooze_length = 1;
  int snooze_hour = 0;
  int snooze_minute = 0;
  snooze_minute += current_time[1] + snooze_length;
  snooze_hour = current_time[2];
  if (snooze_minute>59) {
    snooze_minute -= 60;
    snooze_hour = current_time[2] + 1;
    if (snooze_hour>23){
      snooze_hour -= 24;
    }
  } 
  writeRTCRegister(0x07, B00000001) ; //ALARM1 seconds reg
  writeRTCRegister(0x09, decToBcd(snooze_hour)) ; //Setting Alarm1 hour register
  writeRTCRegister(0x08, decToBcd(snooze_minute)) ; //Setting Alarm1 minute register
  writeRTCRegister(0x0A, B10000000); //Setting Alarm1 day register in a way it triggers when the hour and minute match and second but it's set to 0
  writeRTCRegister(0x0e, B00000101);//enable alarm1, disable alarm2
}

void flash_leds_for_alarm(int debug) {
  if (debug == 0){
    long led_milliss = current_millis - wake_board_millis;
  
    //if (party_time_is_on) led_millis = current_millis - party_battery_measure_millis;
    if (/*led_millis < ALARM_DURATION &&*/ led_milliss % (2*LED_FLASH_INTERVAL) < LED_FLASH_INTERVAL) {
      digitalWrite(LED_1_PIN, HIGH);
      digitalWrite(LED_2_PIN, HIGH);
      digitalWrite(LED_3_PIN, HIGH);
      digitalWrite(LED_4_PIN, HIGH);
    } else /*if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION)*/ { //B:I'm not sure what's the purpose of this so: commented
      digitalWrite(LED_1_PIN, LOW);
      digitalWrite(LED_2_PIN, LOW);
      digitalWrite(LED_3_PIN, LOW);
      digitalWrite(LED_4_PIN, LOW);
    }
    if (led_milliss > ALARM_DURATION) {
    alarm_flag = false;
    clearAlarmStatusBits();
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_2_PIN, LOW);
    digitalWrite(LED_3_PIN, LOW);
    digitalWrite(LED_4_PIN, LOW);
  }
  }
}



//TODO find a place for this code
void debug(){
  display_debug = true;
  if (display_debug){
     //debug_4_digit(battery_adc_sum);
     show_displayed_character_array();
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
  // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
  // 3 - #1 held, 4 - #2 held, 5 - both held
  if (button_hold_counts[0] > BUTTON_HOLD_DURATION_MINIMUM &&
      button_hold_counts[1] > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 5;
    ignore_next_button_releases = 2;    
  } else if (button_hold_counts[1] > BUTTON_HOLD_DURATION_MINIMUM &&
             button_hold_counts[0] < 1) {
    if (ignore_next_button_releases == 2){
      button_state = 5;
    } else {
      button_state = 4;
      ignore_next_button_releases = 1;
    }
  } else if (button_hold_counts[0] > BUTTON_HOLD_DURATION_MINIMUM &&
             button_hold_counts[1] < 1) {
    if (ignore_next_button_releases == 2){
      button_state = 5;
    } else {
      button_state = 3;
      ignore_next_button_releases = 1;
    }
  } else if (button_2_released && ignore_next_button_releases == 0) {
    button_state = 2;
  } else if (button_2_released && ignore_next_button_releases != 0) {
    button_state = 0;
    ignore_next_button_releases = 0;
  } else if (button_1_released && ignore_next_button_releases == 0) {
    button_state = 1;
  } else if (button_1_released && ignore_next_button_releases != 0) {
    button_state = 0; 
    ignore_next_button_releases = 0;
  }
}

void read_current_time() {
  readDS3231time(&current_time[0], &current_time[1], &current_time[2], &current_time[3],
  &current_time[4], &current_time[5], &current_time[6]);
}

void select_control_state() {
  // Launch relevant state functions
  // Switch states dependent on button state
  if (button_state == 4) {
    control_state = ENTER_SETTINGS;
  }
  else if (button_state == 3) control_state = STOPWATCH;
  else if (button_state == 5) {
    control_state = PARTY_TIME;
    setting_value = 0;
    party_init_millis = current_millis;
  }
  if (battery_level == 1 && current_millis - wake_board_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
    control_state=LOW_BATTERY;
  } else {
    if (control_state==LOW_BATTERY) control_state = DISPLAY_TIME;
  }
  switch(control_state) {
    case ALARM:
      display_hour_minute();
      if (button_state == 1) {
        alarm_start_millis = current_millis - ALARM_DURATION;
        clearAlarmStatusBits();
        buzzer_is_on=false;
        alarm_counter = 0;
        control_state = DISPLAY_TIME;
        //digitalWrite(LED_1_PIN, HIGH);
        alarm_flag = false;
      }
      else if (button_state == 2) {
        clearAlarmStatusBits();
        buzzer_is_on=false;
        if (alarm_counter < 4) set_alarm_for_snooze();  
        else alarm_counter = 0;
        control_state = SNOOZE_MESSAGE; 
        alarm_flag = false;
      }
      break;
    case SNOOZE_MESSAGE:
      vfd_displayed_characters[0] = 'r';//Re
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = ' ';
      vfd_displayed_characters[4] = 3 - (alarm_counter+3)%4; //1,2,3,0, -> 0,1,2 ->  3,2,1
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = DISPLAY_TIME;
      break;
    case DISPLAY_TIME:
      display_hour_minute();
      if (button_state == 1) control_state = DISPLAY_DATE;
      else if (button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
      if (alarm_flag) control_state = ALARM;
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
        else /*if (stopwatch_times[0] == 0 && stopwatch_times[1] == 0 && stopwatch_times[2] == 0)*/ {
          stopwatch_running = true;
          stopwatch_times[3] = current_time[4] - stopwatch_times[3];
          stopwatch_times[2] = current_time[2] - stopwatch_times[2];
          stopwatch_times[1] = current_time[1] - stopwatch_times[1];
          stopwatch_times[0] = current_time[0] - stopwatch_times[0];
        }/* else {
          stopwatch_times[0] = 0;
          stopwatch_times[1] = 0;
          stopwatch_times[2] = 0;
          stopwatch_times[3] = 0; // day of month
        }*/
      } else if (button_state == 3){
        stopwatch_running = false;
        stopwatch_times[0] = 0;
        stopwatch_times[1] = 0;
        stopwatch_times[2] = 0;
        stopwatch_times[3] = 0; // day of month
      }
      break;
    case ENTER_SETTINGS:
      vfd_displayed_characters[0] = 'S';
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 't';
      vfd_displayed_characters[4] = ' ';
      if (button_state == 1) control_state = DISPLAY_TIME;
      else if (button_state == 2) control_state = SETTING_NAME_ALARM;
      break;

    case SETTING_NAME_ALARM:
      vfd_displayed_characters[0] = 'A';
      vfd_displayed_characters[1] = 'L';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'A';
      vfd_displayed_characters[4] = 'r';
      if (button_state == 1) { 
        control_state = SETTING_ALARM_MODE;
        setting_value = 0 /*TODO: CURRENT_ALARM MODE*/;
      } else if (button_state == 2) control_state = SETTING_NAME_HOUR;
      break;
    case SETTING_ALARM_MODE:
      /*It's a bit different than the other settings: First we set the mode: On/Off/Silent 
        If Off was selected I think we should exit Alarm setting
        Otherwise going to the SETTING_ALARM_HOUR then SETTING_ALARM_MINUTE*/
      if (setting_value == 0){
        vfd_displayed_characters[0] = 'O';
        vfd_displayed_characters[1] = 'n';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = ' ';
        vfd_displayed_characters[4] = ' ';
      } else if (setting_value == 1){
        vfd_displayed_characters[0] = 'S';
        vfd_displayed_characters[1] = 'i';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = 'L';
        vfd_displayed_characters[4] = 'E';  
      } else if (setting_value == 2){
        vfd_displayed_characters[0] = 'O';
        vfd_displayed_characters[1] = 'F';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = 'F';
        vfd_displayed_characters[4] = ' ';
      }
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 3) setting_value = 0;
      }
      else if (button_state == 1) {
        if (setting_value == 2 /*We selected off*/){
          control_state = /*SETTING_NAME_HOUR*/DISPLAY_TIME;
          writeRTCRegister(0x0e, B00000100); //setting the A2IE bit to zero: disableing the alarm (other pins are good as 0) 
        } else {
          if (setting_value == 0) alarm_sound = true;
          if (setting_value == 1) alarm_sound = false;
          writeRTCRegister(0x0e, B00000110); //setting the A2IE bit to one: enableing the alarm 
          clearAlarmStatusBits();
          //TODO remember somehow if silent or default alarm is set
          control_state = SETTING_ALARM_HOUR ;
          setting_value = bcdToDec(readRTCRegister(0x0C));
        }
      }
      break;
    case SETTING_ALARM_HOUR:
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
        control_state = SETTING_ALARM_MINUTE;
        writeRTCRegister(0x0C, decToBcd(setting_value)) ; //Setting Alarm2 hour register
        setting_value = bcdToDec(readRTCRegister(0x0B));
      }
      break;
    case SETTING_ALARM_MINUTE:
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
        control_state = /*SETTING_NAME_HOUR*/DISPLAY_TIME;
        writeRTCRegister(0x0B, decToBcd(setting_value)) ; //Setting Alarm2 minute register
        writeRTCRegister(0x0D, B10000000); //Setting Alarm2 day register in a way it triggers when the hour and minute match
      }
      break;
    case SETTING_NAME_HOUR://It's actually SETTING_NAME_TIME
      vfd_displayed_characters[0] = 't';
      vfd_displayed_characters[1] = 'i';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'n';
      vfd_displayed_characters[4] = 'n';
      if (button_state == 1) { 
        control_state = SETTING_HOUR;
        setting_value = current_time[2];
      } else if (button_state == 2) control_state = /*SETTING_NAME_MINUTE*/SETTING_NAME_DATE;
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
        control_state = /*SETTING_NAME_MINUTE*/SETTING_MINUTE;
        setting_value = current_time[1];
      }
      break;
    case SETTING_NAME_MINUTE://not need/unreachable code
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
        control_state = SETTING_NAME_DAY_OF_WEEK;//TODO connect
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
        control_state = /*SETTING_NAME_DAY_OF_WEEK*/DISPLAY_TIME;
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
      else if (button_state == 2) control_state = /*SETTING_NAME_DAY_OF_MONTH*/SETTING_NAME_YEAR;
      break;
    case SETTING_DAY_OF_WEEK:
    display_day_of_week(setting_value);
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 8) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], setting_value, current_time[4], current_time[5], current_time[6]);
        control_state = /*SETTING_NAME_DAY_OF_MONTH*/DISPLAY_TIME;
      }
      break;
    case SETTING_DATE_ORDER:
      if (setting_value == 1){
        vfd_displayed_characters[0] = 'n';
        vfd_displayed_characters[1] = 'n';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = 'd';
        vfd_displayed_characters[4] = 'd';
      } else if (setting_value == 2){
        vfd_displayed_characters[0] = 'd';
        vfd_displayed_characters[1] = 'd';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = 'n';
        vfd_displayed_characters[4] = 'n';
      }
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 3) setting_value = 1;
      }
      else if (button_state == 1) {
        if (setting_value == 1) month_before_day = true;
        else month_before_day = false;
        control_state = /*SETTING_NAME_MONTH*/SETTING_MONTH;
        setting_value = current_time[5];
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
      else if (button_state == 2) control_state = /*SETTING_NAME_MONTH*/SETTING_NAME_DATE;
      break;
    case SETTING_DAY_OF_MONTH:
      if (month_before_day){
        vfd_displayed_characters[0] = ' ';
        vfd_displayed_characters[1] = ' ';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = setting_value / 10;
        vfd_displayed_characters[4] = setting_value % 10;
      } else {
        vfd_displayed_characters[0] = setting_value / 10;
        vfd_displayed_characters[1] = setting_value % 10;
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = ' ';
        vfd_displayed_characters[4] = ' ';
      }
      
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == (MONTH_LENGTHS[current_time[5]] + 1)) setting_value = 1;
        // Leap day condition
        if (current_time[6] % 4 == 0 && current_time[5] == 2 && setting_value == 29) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], setting_value, current_time[5], current_time[6]);
        control_state = /*SETTING_NAME_MONTH*/DISPLAY_TIME;
      }
      break;
    case /*SETTING_NAME_MONTH*/SETTING_NAME_DATE:
      vfd_displayed_characters[0] = 'd';
      vfd_displayed_characters[1] = 'A';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 't';
      vfd_displayed_characters[4] = 'E';
      if (button_state == 1) { 
        control_state = SETTING_DATE_ORDER;
        if (month_before_day) setting_value = 1;
        else setting_value = 2;
      }
      else if (button_state == 2) control_state = /*SETTING_NAME_YEAR*/SETTING_NAME_DAY_OF_WEEK;
      break;
    case SETTING_MONTH:
      if (month_before_day){
        vfd_displayed_characters[0] = setting_value / 10;
        vfd_displayed_characters[1] = setting_value % 10;
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = ' ';
        vfd_displayed_characters[4] = ' ';
      } else {
        vfd_displayed_characters[0] = ' ';
        vfd_displayed_characters[1] = ' ';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = setting_value / 10;
        vfd_displayed_characters[4] = setting_value % 10;
      }
      if (button_state == 2) { 
        setting_value += 1;
        if (setting_value == 13) setting_value = 1;
      }
      else if (button_state == 1) {
        setDS3231time(current_time[0], current_time[1], current_time[2], current_time[3], current_time[4], setting_value, current_time[6]);
        control_state = /*SETTING_NAME_YEAR*//*DISPLAY_TIME*/SETTING_DAY_OF_MONTH;
        setting_value = 1;//current_time[4];
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
        control_state = /*SETTING_NAME_TEMPERATURE*/DISPLAY_TIME;
      }
      break;
    case SETTING_NAME_TEMPERATURE:
      vfd_displayed_characters[0] = 't';
      vfd_displayed_characters[1] = 'E';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'n';
      vfd_displayed_characters[4] = 'n';
      if (button_state == 1) control_state = SETTING_TEMPERATURE;
      else if (button_state == 2) control_state = /*SETTING_NAME_HOUR*/ SETTING_SCREEN_TIMEOUT_NAME;
      break;
    case SETTING_TEMPERATURE:
      vfd_displayed_characters[0] = ' ';
      vfd_displayed_characters[1] = ' ';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = '*'; 
      vfd_displayed_characters[4] = temperature_unit;
      if (button_state == 1) {
        control_state = /*SETTING_NAME_HOUR*/DISPLAY_TIME;
        EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
      }
      else if (button_state == 2) {
        if (temperature_unit == 'C') temperature_unit = 'F';
        else temperature_unit = 'C'; 
      }
      break;
    case SETTING_SCREEN_TIMEOUT_NAME:
      vfd_displayed_characters[0] = 'L';
      vfd_displayed_characters[1] = 'I';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = '9';
      vfd_displayed_characters[4] = 'H';
      if (button_state == 1) control_state = SETTING_SCREEN_TIMEOUT;
      else if (button_state == 2) control_state = /*SETTING_NAME_HOUR*/ SETTING_NAME_ALARM;
      break;
    case SETTING_SCREEN_TIMEOUT:
      vfd_displayed_characters[0] = ' ';
      vfd_displayed_characters[1] = ' ';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = sleep_timout_interval /500 / 10; 
      vfd_displayed_characters[4] = sleep_timout_interval /500 % 10;
      if (button_state == 1) {
        control_state = /*SETTING_NAME_HOUR*/DISPLAY_TIME;
      }
      else if (button_state == 2) {
        setting_value += 1;
        if (setting_value > 10) setting_value = 0;
        sleep_timout_interval = 5000 + 1000 * setting_value; 
      }
      break;
    case PARTY_TIME:
      if (current_millis - party_init_millis < PARTY_INIT_INTERVAL) {
        vfd_displayed_characters[0] = 'P';
        vfd_displayed_characters[1] = 'r';
        vfd_displayed_characters[2] = ' ';
        vfd_displayed_characters[3] = 't';
        vfd_displayed_characters[4] = 'Y';
        //vfd_displayed_characters[0] = setting_value;
      } else {
        
        char pt_digit_1   = PARTY_TIMES[setting_value] / 10;
        char pt_digit_2   = PARTY_TIMES[setting_value] % 10;
        
        if (setting_value == PARTY_TIMES_SIZE){
          vfd_displayed_characters[0] = 'O';
          vfd_displayed_characters[1] = 'F';
          vfd_displayed_characters[2] = ' ';
          vfd_displayed_characters[3] = 'F';
          vfd_displayed_characters[4] = ' ';
        } else if (setting_value == PARTY_TIMES_SIZE - 1){
          vfd_displayed_characters[0] = 'C';
          vfd_displayed_characters[1] = 'O';
          vfd_displayed_characters[2] = ' ';
          vfd_displayed_characters[3] = 'N';
          vfd_displayed_characters[4] = 't';
        
        } else {
          vfd_displayed_characters[0] = ' ';
          vfd_displayed_characters[1] = ' ';
          vfd_displayed_characters[2] = ' ';
          vfd_displayed_characters[3] = pt_digit_1;
          vfd_displayed_characters[4] = pt_digit_2;
        }
      }
      if (button_state == 2) { 
        party_init_millis = current_millis-PARTY_INIT_INTERVAL; //get out of party_init as soon as button pressed
        setting_value += 1;
        if (setting_value >= PARTY_TIMES_SIZE + 1/*sizeof(PARTY_TIMES)/sizeof(PARTY_TIMES[0])*/) setting_value = 0; //Black magic: that's only the lenght of the array: sizeof(a)/sizeof(a[0])
      } else if (button_state == 1 || current_millis - last_input_millis > PARTY_SET_INTERVAL) {
        
        if (setting_value != PARTY_TIMES_SIZE) {
          
        } else {
         
        } 

        if (setting_value == PARTY_TIMES_SIZE){
           party_time_is_on = false;
          effect_state = NO_EFFECT;
          control_state = DISPLAY_TIME; 
        } else if (setting_value == PARTY_TIMES_SIZE - 1){
          party_time = 69; //hihi I'm a mature adult now...
          party_time_is_on = true;
          effect_state = SEC_FLASH_EFFECT;
          control_state = DISPLAY_TIME;
        } else {
          party_time = PARTY_TIMES[setting_value];
          party_time_is_on = true;
          effect_state = SEC_FLASH_EFFECT;
          control_state = DISPLAY_TIME;
        }
      }
      break;
     case LOW_BATTERY:
      vfd_displayed_characters[0] = 'B';
      vfd_displayed_characters[1] = 'A';
      vfd_displayed_characters[2] = ' ';
      vfd_displayed_characters[3] = 'L'; 
      vfd_displayed_characters[4] = 'o';
      break;
  }
}

void display_stopwatch() {
  if (stopwatch_running) {
    colon_steady = false;
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
  

  if (month_before_day){
    vfd_displayed_characters[0] = month_digit_1;
    vfd_displayed_characters[1] = month_digit_2;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = day_of_month_digit_1;
    vfd_displayed_characters[4] = day_of_month_digit_2;
  } else {
    vfd_displayed_characters[0] = day_of_month_digit_1;
    vfd_displayed_characters[1] = day_of_month_digit_2;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = month_digit_1;
    vfd_displayed_characters[4] = month_digit_2;
  }
}

void display_seconds() {
  char seconds_digit_1 = current_time[0] / 10;
  char seconds_digit_2 = current_time[0] % 10;
  vfd_displayed_characters[0] = ' ';
  vfd_displayed_characters[1] = ' ';
  vfd_displayed_characters[2] = 1;
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
  /*if (board_sleeping) {
    //ignore_next_button_releases = 1;
    repower_vfd = true;
    board_sleeping = false;
    battery_level = 4;
    sleep_disable();
    ADCSRA = adcsra;
    control_state = DISPLAY_TIME;
    last_input_millis = current_millis;
    last_battery_read_millis = 0;
    TIMSK1 |= (1 << OCIE1A);
  }*/
  /*Moved this to the power_board_down: 
    ISR shouldn't contain too much code, it's for átbillenteni some flags :D
    And when an ISR is called the power_board_down continues after the sleep_board() 
    because... black magic 
    Also: this way the rock kills two birds: the alarm-bird and the button-bird*/
}


ISR(PCINT1_vect)
{
  if (!alarm_flag){
  alarm_start_millis = current_millis;
  control_state = ALARM;
  alarm_flag = true;
  alarm_counter ++;
  }
  //digitalWrite(LED_1_PIN, HIGH);
    //writeStatusRegister(B00000000);
    //EIMSK = 0;                     //disable external interrupts (only need one to wake up)
}

void power_board_down(bool permit_wakeup) {
  if (!board_sleeping && !alarm_flag){
    /*digitalWrite(LED_2_PIN, HIGH);
    delay(100);
    digitalWrite(LED_2_PIN,LOW);*/
    
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
      case PARTY_TIME:
        party_time = PARTY_TIMES[setting_value];
        party_time_is_on = true;
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
    cli();  //disable interrupts while we're configuring them
    if (permit_wakeup) {
      PCICR  |= 0b00000001;   // turn on port b for PCINTs
      PCMSK0 |= 0b00000100;   // turn on PCINT 2 mask
    } else {
      PCICR  = 0b00000000;    // turn off PCINT interrupt
      PCMSK0 = 0b00000000;
    } 
    //Set alarm interrupt
    PCICR |= (1 << PCIE1);    // set PCIE2 to enable PCMSK2 scan
    PCMSK1 |= (1 << PCINT11); 
      
    adcsra = ADCSRA;               //save the ADC Control and Status Register A
    ADCSRA = 0;                    //disable ADC
    mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
    mcucr2 = mcucr1 & ~_BV(BODSE);
    MCUCR = mcucr1;
    MCUCR = mcucr2;
    // Disable vfd heating timer interrupt
    TIMSK1 = 0;
    sei();  //reenable interrupts
    sleep_enable();
    clearAlarmStatusBits();
    sleep_cpu();                   //go to sleep
     //  Zzzz-Zzzz-Zzzz             
    if (board_sleeping) {         //wake up
      //ignore_next_button_releases = 1;
      battery_level = 4;
      sleep_disable();
      ADCSRA = adcsra;
      control_state = DISPLAY_TIME;
      last_input_millis = current_millis;
      last_battery_read_millis = 0;
      TIMSK1 |= (1 << OCIE1A);
      repower_vfd = true;
      board_sleeping = false;
      digitalWrite(POWER_MEASURE_PIN, HIGH);
    }
  }
}

//TODO this isn't the place for this
void clearAlarmStatusBits(){
  int buffer = readRTCRegister(0x0f);
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x0f);
  Wire.write(B11111100 & buffer); 
  Wire.endTransmission();
}

//TODO this isn't the place for this
int readRTCRegister(int address){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  return Wire.read();
}

//TODO this isn't the place for this
void writeRTCRegister(int address, int value){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(address);
  Wire.write(value); 
  Wire.endTransmission();
}


int extra_display_required = 3;

void show_displayed_character_array() {
  if (vfd_current_cell_id == 2) {
    vfd_current_cell_id += 1;
  }
  bool include_colon = ((current_millis - colon_millis) < COLON_BLINK_PERIOD || colon_steady) && vfd_displayed_characters[2] != ' ';
  // Group the colon light with turning on grid 3, grid 1 if 3 is empty
  if (vfd_displayed_characters[3] == ' ') {
    if (vfd_current_cell_id != 1) include_colon = false;
  } else if (vfd_current_cell_id != 3) include_colon = false;
  
  if (extra_display_required==0){
    set_vfd_cell(vfd_current_cell_id, char_to_segments(vfd_displayed_characters[vfd_current_cell_id]), include_colon);  
    if (vfd_current_cell_id == 0) extra_display_required = 3;
    if (vfd_current_cell_id == 4) extra_display_required = 4;
    vfd_current_cell_id += 1;
  } else {
    if (vfd_current_cell_id == 0){
      if (extra_display_required % 2 == 0){
        set_vfd_cell(4, char_to_segments(vfd_displayed_characters[4]), include_colon); 
      } else {
        set_vfd_cell(4, char_to_segments(vfd_displayed_characters[4]) & char_to_segments('3'), include_colon);
      }
    } else if (vfd_current_cell_id == 1){
      set_vfd_cell(0, char_to_segments(vfd_displayed_characters[0]) & char_to_segments('E'), include_colon);
    }
    extra_display_required -= 1;
  }
  
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
  //display_hertz_of_function();
}

void set_vfd_cell(uint8_t cell_num, /*char character_to_set*/ uint16_t segment_pattern, bool include_colon) {
  if (cell_num > 4) return;
  segment_pattern |= vfd_cells[cell_num];
  if (include_colon) segment_pattern |= vfd_cells[2];
  digitalWrite(VFD_BLANK_PIN, HIGH);
  bool out_bit = 0;
  for (char i = 0; i < 12; i++) {
    out_bit = 1 & (segment_pattern >> i);
    // Load data to pin
    if (out_bit) digitalWrite(VFD_DATA_PIN, HIGH);
    else digitalWrite(VFD_DATA_PIN, LOW);
    // Trigger write: CLK pin High->Low
    digitalWrite(VFD_LOAD_PIN, LOW);
    digitalWrite(VFD_LOAD_PIN, HIGH);
    digitalWrite(VFD_CLK_PIN, HIGH);
    digitalWrite(VFD_CLK_PIN, LOW);
    // Trigger shift: LOAD pin Low->High    
  }
  // Show output to display
  /*if (light_level == 0) digitalWrite(4, LOW);
  else analogWrite(4, light_level);*/
  analogWrite(VFD_BLANK_PIN, light_level);
  //digitalWrite(VFD_BLANK_PIN, LOW);
  
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
    case 'O':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_C | vfd_segment_D | vfd_segment_E | vfd_segment_F;
    break;
    case 'o':
      outputSegCode = vfd_segment_E | vfd_segment_G | vfd_segment_C | vfd_segment_D;
    break;
    case 'P':
      outputSegCode = vfd_segment_A | vfd_segment_B | vfd_segment_E | vfd_segment_F | vfd_segment_G;
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

void setDS3231Alamr(byte minute, byte hour){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  //TODO  
}

byte decToBcd(byte val) {
  return( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val) {
  return( (val/16*10) + (val%16) );
}

/*void read_light_level() {
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
}*/

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
  if (current_millis - last_battery_read_millis > BATTERY_READ_INTERVAL 
      || last_battery_read_millis == 0
      || party_time_is_on ) {
    last_battery_read_millis = current_millis;
    uint16_t bat_level_adc = analogRead(POWERSENSE_PIN);
    if (party_time_is_on && current_millis % (1000 * 10)  == 0) {
      battery_adc_measurement_count = 0;
      battery_adc_sum = 0;
      party_battery_measure_millis = current_millis;
    }
    if (battery_adc_measurement_count < BATTERY_READ_MAX_COUNT){
      battery_level == 4;
      battery_adc_sum += bat_level_adc;
      battery_adc_measurement_count += 1;
      if (battery_adc_measurement_count == BATTERY_READ_MAX_COUNT) {
        battery_adc_sum = battery_adc_sum / battery_adc_measurement_count;
      }
    } else if (battery_adc_sum > HIGH_BATTERY_ADC_VALUE)   battery_level = 0;
    else if (battery_adc_sum > MEDIUM_BATTERY_ADC_VALUE)   battery_level = 1;
    else if (battery_adc_sum > LOW_BATTERY_ADC_VALUE)      battery_level = 2;
    else battery_level = 3;
  }
}

void flash_leds() {
  long led_millis = current_millis - wake_board_millis;
  //if (party_time_is_on) led_millis = current_millis - party_battery_measure_millis;
  if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION && led_millis % (2*LED_FLASH_INTERVAL) < LED_FLASH_INTERVAL) {
    digitalWrite(LED_1_PIN, HIGH);
    digitalWrite(LED_2_PIN, HIGH);
    digitalWrite(LED_3_PIN, HIGH);
    digitalWrite(LED_4_PIN, HIGH);
  } else /*if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION)*/ { //B:I'm sure what't the purpose of this so commented
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_2_PIN, LOW);
    digitalWrite(LED_3_PIN, LOW);
    digitalWrite(LED_4_PIN, LOW);
  }
}

/*void buzz_buzzer(){  //Currenty unused function currently interruptgeneration is the way
  int buzzer_micros = current_micros - wake_board_millis*1000;
  if (buzzer_micros % (2*BUZZER_BUZZ_INTERVAL) < BUZZER_BUZZ_INTERVAL) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }   
}*/

float celsius_to_fahrenheit(float celsius) {
  float fh = (celsius * 9.0) / 5.0;
  fh += 32;
  return fh;
}

/*Set the display charaters to a floats 4 digits (at these places: thousands, hundreds, tens and ones) on the VFD
for debug purposes.
If the float is greater than 9999.0 then didplays 10th of the input (the ':' is used as thousands's coma)*/
void debug_4_digit(float n){
  if (n>9999.9999){
    uint8_t n_tenthousands = int(n / 10000) % 10;
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens     = int(n / 10) % 10;
    uint8_t n_ones     = int(n)  % 10;
    vfd_displayed_characters[0] = n_tenthousands;
    vfd_displayed_characters[1] = n_thousands;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = n_hundreds; 
    vfd_displayed_characters[4] = n_tens;
  } else {
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens     = int(n / 10) % 10;
    uint8_t n_ones     = int(n)  % 10;
    vfd_displayed_characters[0] = n_thousands;
    vfd_displayed_characters[1] = n_hundreds;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = n_tens; 
    vfd_displayed_characters[4] = n_ones;  
  }  
}

volatile int disp_counter=1;

ISR (TIMER2_COMPA_vect)
{
  /*if (current_millis % 10 < 5) {
    show_displayed_character_array();
  }*/
  /*if (disp_counter<10){
    if (!display_debug){
      show_displayed_character_array();
    }
    disp_counter += 30;
  }
  disp_counter -= 12;*/
  //Buzzer controlling this runs on ~2700HZ
  //Option 1: The hertz is set in the interrupt_setup then it looks like this:
  if(buzzer_is_on){
    buzzing = !buzzing;
    if (buzzing) {
      digitalWrite(BUZZER_PIN, buzzing);
    } else {
      digitalWrite(BUZZER_PIN, buzzing);
    }
  }
  
  //Option 2: We only use this to update current_micros, and call for buzzer if needed:
  /*current_micros = micros();
  if(buzzer_is_on){
    buzz_buzzer();
  }*/
  //display_hertz_of_function();
}

void setup_interrupts() {
  // Processor timer interrupt setup
  cli(); // Stop existing interrupts
  // Set timer1 interrupt at 50Hz
  TCCR1A = 0;// set entire TCCR0A register to 0
  TCCR1B = 0;// same for TCCR0B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 50Hz increments
  OCR1A = 249;//312;// = (16*10^6) / (50*64) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 bit for 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  //sei(); // Allow interrupts
  
  //And a second timer for the buzzer:
  /* OCR = processor_clock_hertz/(prescaler*desired_hertz)-1 
   * Warning! Because of reasons you need four times the hertz hat you want
   * You can choose prescaler and OCR with with this help:
   * https://docs.google.com/spreadsheets/d/1G_DHwnsCHbE1ZFQqTacAgZ9cqcwEszhTkyKdCZyILc4/edit?usp=sharing 
   * We needed 2730 Hz (for hardware reasons...), I set the OCR and the prescaler to get 10860 Hz (10860/4~=2730 roughly)
   * TODO (if you are bored): Implement a function which gives back the best OCR and prescaler given the desired frequency  */
  /*TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 22;//22  // = (16*10^6) / (10860*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22 bit for 64 prescaler
  TCCR2B |= (1 << CS22) ;   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  */  
  /*3rd timer for the VFD display, instead of the previous way: doing it in the loop()
   */
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  OCR0A = 249;  //124  Around 20 the shift registers can't keep up with the frequency, above 50 it starts to vibrate
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS01) /*| (1 << CS00)*/ ;   
  TIMSK0 |= (1 << OCIE0A);
  sei();
}

volatile int per_four = 4; //with 249 on OCR, and 8 on prescaler it's gonna be a 8 MHz, the screen is good with 2MHz
volatile int per_2 = 0;
ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
{
  per_four +=1;
  if (per_four%4==0){
    milliss += 1;
    /*
    debug_4_digit(battery_adc_sum);
    vfd_displayed_characters[0] = int(battery_adc_sum)/10;
    vfd_displayed_characters[1] = battery_adc_sum/100;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = int(battery_adc_sum/10)%10; 
    vfd_displayed_characters[4] = int(battery_adc_sum)%10;*/
    //if (battery_level != 2  and battery_level != 4 and !display_debug /*&& per_2 % 2 ==0*/){ show_displayed_character_array();
    //colon_steady = false;
    //per_2 = 0;
    //}
    //per_2 = per_2+1;
    per_four=0;
  }
  
  
  /*if (!display_debug ){
  }*/
  if (per_four %2 == 0){
    if (battery_level != 2  and battery_level != 4 and !display_debug /*&& per_2 % 2 ==0*/){ show_displayed_character_array();
    //colon_steady = false;
    //per_2 = 0;
    }
    if(buzzer_is_on){
      buzzing = !buzzing;
      if (buzzing) {
        digitalWrite(BUZZER_PIN, buzzing);
      } else {
        digitalWrite(BUZZER_PIN, buzzing);
      }
    }
  }
  //
   /*if(buzzer_is_on){
    buzzing = !buzzing;
    if (buzzing) {
      digitalWrite(BUZZER_PIN, buzzing);
    } else {
      digitalWrite(BUZZER_PIN, buzzing);
    }
   }
   */
}

volatile float h1 = 0.0; //I didn't want to pullute with this the already overpopulated declarations at the top
volatile float h2 = 0.0;
volatile float h3 = 0.0;
/*For Debugging, it show the frequency of a function (can be used in the loop() and the interrupt generation ISR ones)
  I might overcomplicated it sorry, but it works now and didn't bothered to simplify*/
void display_hertz_of_function(){
  //debug_float = hertz_counter;
  hertz_counter++;
  long led_millis = current_millis - wake_board_millis;
  if (led_millis % 3000 < 1000 && led_millis % 3000 >= 0) {
    display_debug = true; //This isn't really belong here, for now 
    h1 = hertz_counter;
    debug_float = fabs(h3-h2);
  } else if ( led_millis % 3000 < 2000 && led_millis % 3000 >= 1000) {
    display_debug = true; //This isn't really belong here, for now 
    h2 = hertz_counter;
    debug_float = fabs(h1-h3);
  } else {
    display_debug = true; //This isn't really belong here, for now 
    h3 = hertz_counter;
    debug_float = fabs(h2-h1);
  }
}
