#include "VFDManager.h"

VFDManager::VFDManager()
{
  // VFD futes pinek
  pinMode(HEAT1_PIN, OUTPUT);
  pinMode(HEAT2_PIN, OUTPUT);
  // VFD adat pinek
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LOAD_PIN, OUTPUT);
  pinMode(BLANK_PIN, OUTPUT);
  // VFD futes tap
  pinMode(POWER_SWITCH_PIN, OUTPUT);
  digitalWrite(POWER_SWITCH_PIN, LOW);

  digitalWrite(HEAT1_PIN, HIGH);
  digitalWrite(HEAT2_PIN, LOW);

  setup_interrupt_for_heating();

  current_cell_id = 0;
  is_outer_needed = false;
  delay_needed = false;
  colon_millis = 0;
  colon_steady = false;
  heat_counter = 0;
  repower = false;
}

void VFDManager::set_cell(uint8_t cell_num, char character_to_set, bool include_colon)
{ //display
  if (cell_num > 4)
    return;
  uint16_t segment_pattern;
  
  if (current_cell_id == 0) {
    if (is_outer_needed){
      segment_pattern = char_to_segments(character_to_set) & char_to_segments('/'); 
      delay_needed = true;
      is_outer_needed = false;
    } else {
      segment_pattern = char_to_segments(character_to_set);
      is_outer_needed = true;
    }
  } else if (current_cell_id == 4){
    if (is_outer_needed){
      segment_pattern = char_to_segments(character_to_set) & char_to_segments('I'); 
      delay_needed = true;
      is_outer_needed = false;
      if (counter_f <1){
        delay_needed = true;
        is_outer_needed = true;
        counter_f ++;
      }  else {
        counter_f = 0;
        delay_needed = true;
        is_outer_needed = false;
      }
    } else {
      segment_pattern = char_to_segments(character_to_set);
      is_outer_needed = true;
    }
  } else {
    segment_pattern = char_to_segments(character_to_set);
  }

  segment_pattern |= cells[cell_num];
  if (include_colon)
    segment_pattern |= cells[2];
  digitalWrite(BLANK_PIN, HIGH);
  //
  bool out_bit = 0;
  for (char i = 0; i < 12; i++)
  {
    out_bit = 1 & (segment_pattern >> i);
    // Load data to pin
    if (out_bit)
      digitalWrite(DATA_PIN, HIGH);
    else
      digitalWrite(DATA_PIN, LOW);
    // Trigger write: CLK pin High->Low
    // Trigger shift: LOAD pin Low->High
    clrPin(LOAD_PIN);
    setPin(LOAD_PIN);

    setPin(CLK_PIN); // else
    clrPin(CLK_PIN);
  }
  // Show output to display
  //analogWrite(BLANK_PIN, 100);
  
  digitalWrite(BLANK_PIN, LOW);
}

void VFDManager::setup_interrupt_for_heating()
{
  cli(); // Stop existing interrupts
  // Set timer1 interrupt at 50Hz
  TCCR1A = 0; // set entire TCCR0A register to 0
  TCCR1B = 0; // same for TCCR0B
  TCNT1 = 0;  //initialize counter value to 0
  // set compare match register for 50Hz increments
  OCR1A = 30; //312;// = (16*10^6) / (50*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // Allow interrupts
}

void VFDManager::heating()
{
  if ((heat_counter >= 5 && heat_counter < 10) || heat_counter >= 15)
  {
    digitalWrite(HEAT1_PIN, LOW);
    digitalWrite(HEAT2_PIN, LOW);
  }
  else if (heat_counter < 5)
  {
    digitalWrite(HEAT1_PIN, HIGH);
    digitalWrite(HEAT2_PIN, LOW);
  }
  else if (heat_counter >= 10 && heat_counter < 15)
  {
    digitalWrite(HEAT1_PIN, HIGH);
    digitalWrite(HEAT2_PIN, LOW);
  }
  heat_counter += 1;
  if (heat_counter == 20)
  {
    heat_counter = 0;
  }
}

void VFDManager::update_char_array(char *characters)
{ //TODO the char_array parameter shouldn't be there
  for (unsigned int i = 0; i < 5; i++)
  { //TODO 5?
    displayed_characters[i] = characters[i];
  }
}

void VFDManager::update_char_array(char c1, char c2, char c3, char c4, char c5)
{
  displayed_characters[0] = c1;
  displayed_characters[1] = c2;
  displayed_characters[2] = c3;
  displayed_characters[3] = c4;
  displayed_characters[4] = c5;
}

void VFDManager::show_displayed_character_array(unsigned long current_millis) {//display
  
  if (current_cell_id == 2) {
    current_cell_id += 1;
  }
  
  if (delay_needed) {
    //delay(1);
    delay_needed = false;
  }
  //unsigned long current_millis = millis(); //TODO 
  bool include_colon = ((current_millis - colon_millis) < COLON_BLINK_PERIOD || colon_steady) && displayed_characters[2] != ' ';
  // Group the colon light with turning on grid 3, grid 1 if 3 is empty
  if (displayed_characters[3] == ' ') {
    if (current_cell_id != 1) include_colon = false;
  } else if (current_cell_id != 3) include_colon = false;
  set_cell(current_cell_id, displayed_characters[current_cell_id] & displayed_characters[current_cell_id], include_colon);  
  
  if (is_outer_needed && (current_cell_id == 0 || current_cell_id == 4)){
  } else {
    current_cell_id ++;
  }
  
  if (current_millis - colon_millis > 2 * COLON_BLINK_PERIOD) colon_millis = current_millis;
  if (current_cell_id == 5) current_cell_id = 0;
}

unsigned int VFDManager::char_to_segments(char inputChar)
{
  unsigned int outputSegCode = 0x0000;
  switch (inputChar)
  {
  case ' ':
    outputSegCode = 0;
    break;
  case '/':
    outputSegCode = segment_E | segment_F;
    break;
  case '*':
    outputSegCode = segment_A | segment_B | segment_F | segment_G;
    break;
  case 0:
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
  case 1:
    outputSegCode = segment_B | segment_C;
    break;
  case 2:
    outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;
  case 3:
    outputSegCode = segment_A | segment_B | segment_C | segment_G | segment_D;
    break;
  case 4:
    outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
  case 5:
    outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
  case 6:
    outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 7:
    outputSegCode = segment_A | segment_B | segment_C;
    break;
  case 8:
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 9:
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
  case '0':
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
  case '1':
    outputSegCode = segment_B | segment_C;
    break;
  case '2':
    outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;
  case '3':
    outputSegCode = segment_A | segment_B | segment_C | segment_G | segment_D;
    break;
  case '4':
    outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
  case '5':
    outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
  case '6':
    outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case '7':
    outputSegCode = segment_A | segment_B | segment_C;
    break;
  case '8':
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case '9':
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
  case 'a':
    outputSegCode = segment_A | segment_B | segment_C | segment_E | segment_F | segment_G;
    break;  
  case 'A':
    outputSegCode = segment_A | segment_B | segment_C | segment_E | segment_F | segment_G;
    break;
  case 'b':
    outputSegCode = segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 'B':
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 'c':
    outputSegCode = segment_G | segment_E | segment_D;
    break;
  case 'C':
    outputSegCode = segment_A | segment_E | segment_F | segment_D;
    break;
  case 'd':
    outputSegCode = segment_B | segment_C | segment_D | segment_E | segment_G;
    break;
  case 'D':
    outputSegCode = segment_A |segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
  case 'e':
    outputSegCode = segment_A | segment_B | segment_D | segment_E | segment_F | segment_G;
    break;
  case 'E':
    outputSegCode = segment_A | segment_D | segment_E | segment_F | segment_G;
    break; 
  case 'f':
    outputSegCode = segment_A | segment_E | segment_F | segment_G;
    break;
  case 'F':
    outputSegCode = segment_A | segment_E | segment_F | segment_G;
    break;
  case 'g':
    outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 'G':
    outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
  case 'h':
    outputSegCode = segment_F | segment_E | segment_G | segment_C;
    break;
  case 'H':
    outputSegCode = segment_F | segment_E | segment_G | segment_C | segment_B;
    break;
  case 'i':
    outputSegCode = segment_E;
    break;
  case 'I':
    outputSegCode = segment_B | segment_C;
    break;
  case 'j':
    outputSegCode = segment_A | segment_B | segment_C | segment_D;
    break;
  case 'J':
    outputSegCode = segment_A | segment_B | segment_C | segment_D;
    break;
  case 'l':
    outputSegCode = segment_D | segment_E | segment_F;
    break;  
  case 'L':
    outputSegCode = segment_D | segment_E | segment_F;
    break;
  case 'm':
    outputSegCode = segment_G | segment_C;
    break;
  case 'M':
    outputSegCode = segment_A | segment_C | segment_B;
    break;
  case 'n':
    outputSegCode = segment_E | segment_G | segment_C;
    break;
  case 'N':
    outputSegCode = segment_E | segment_A | segment_C | segment_B | segment_F;
    break;
  case 'o':
    outputSegCode = segment_E | segment_G | segment_C | segment_D;
    break;
  case 'O':
    outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
  case 'p':
    outputSegCode = segment_A | segment_B | segment_E | segment_F | segment_G;
    break;  
  case 'P':
    outputSegCode = segment_A | segment_B | segment_E | segment_F | segment_G;
    break;
  case 'r':
    outputSegCode = segment_E | segment_G;
    break;
  case 'R':
    outputSegCode = segment_A | segment_B | segment_C | segment_E | segment_F | segment_G;
    break;
  case 's':
    outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
  case 'S':
    outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
  case 't':
    outputSegCode = segment_F | segment_E | segment_D | segment_G;
    break;
  case 'T':
    outputSegCode = segment_F | segment_E | segment_D | segment_G;
    break;
  case 'u':
    outputSegCode = segment_E | segment_D | segment_C;
    break;
  case 'U':
    outputSegCode = segment_E | segment_D | segment_C | segment_B | segment_F;
    break;
  case 'v':
    outputSegCode = segment_E | segment_D | segment_C;
    break;
  case 'V':
    outputSegCode = segment_E | segment_D | segment_C | segment_B | segment_F;
    break;
  case 'w':
    outputSegCode = segment_D | segment_C;
    break;
  case 'W':
    outputSegCode = segment_D | segment_C | segment_B;
    break;
  case 'x':
    outputSegCode = segment_F | segment_E | segment_G | segment_C | segment_B;
    break;
  case 'X':
    outputSegCode = segment_F | segment_E | segment_G | segment_C | segment_B;
    break;
  case 'y':
    outputSegCode = segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
  case 'Y':
    outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
  case 'z':
    outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;
  case 'Z':
    outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;  
    //segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
  }
  return outputSegCode;
}

void VFDManager::turn_on()
{
  digitalWrite(POWER_SWITCH_PIN, HIGH);
}

void VFDManager::turn_off()
{
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(LOAD_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(BLANK_PIN, LOW);
  digitalWrite(POWER_SWITCH_PIN, LOW);
  digitalWrite(HEAT1_PIN, LOW);
  digitalWrite(HEAT2_PIN, LOW);
}

void VFDManager::debug_4_digit(float n)
{
  if (n > 9999.9999)
  {
    uint8_t n_tenthousands = int(n / 10000) % 10;
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens = int(n / 10) % 10;
    uint8_t n_ones = int(n) % 10;
    update_char_array(n_tenthousands, n_thousands, '1', n_hundreds, n_tens);
  }
  else
  {
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens = int(n / 10) % 10;
    uint8_t n_ones = int(n) % 10;
    update_char_array(n_thousands, n_hundreds, ' ', n_tens, n_ones);
  }
}
