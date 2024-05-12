/*
 *  plastoc dispose fixture controller v1
 *  
 *  Controller for fixture plastic mold dispose
 *  in depuration stage FSM
 *  
 *  menymp
 *  Apr 2024
 *  
 *  YYYY/MM/DD  NAME    CHANGE
 *  2024/05/04  menymp  add validation routine and input map
 */
/* FSM STATES */
typedef enum {
  READY,
  VALIDATION,
  LATCH,
  SET_LOCK,
  TEST,
  UNLOCK,
  DEFECT,
  RAZOR_UP,
  RAZOR_CUT,
  WAIT_EXIT
}SYSTEM_STATE;

/* INPUTS DEFINITIONS */
#define SENSORS_INPUT           true
#define BUTTON_PUSHED           true
#define DOOR_CLOSED             true
/* OUTPUTS DEFINITIONS */
#define LAMP_ON                 HIGH
#define LAMP_OFF                LOW
#define SET_LOCK_OUT            true
#define RESET_LOCK              false
#define SET_RAZOR               true
#define RESET_RAZOR             false
/* CLOCK DELAY */
#define CLOCK_DELAY             100
/* STATES DELAY COUNTS */
/* estimated with pneumatic moves since there are no sensors */
#define LATCH_TIME              4
#define LOCK_TIME               10
#define RAZOR_CUT_TIME          10
#define TEST_TIME               10
#define DEFECT_TIME             10

/*log messages definitions*/
#define READY_MSG      "READY_STATE"
#define LATCH_MSG      "LATCH_STATE"
#define VALIDATION_MSG "VALIDATION_STATE"
#define SET_LOCK_MSG   "SET_LOCK_STATE"
#define TEST_MSG       "TEST_STATE"
#define DEFECT_MSG     "DEFECT_STATE"
#define UNLOCK_MSG     "UNLOCK_STATE"
#define WAIT_EXIT_MSG  "WAIT_EXIT_STATE"
#define HARD_FAULT_MSG "HARD_FAULT"
#define RAZOR_UP_MSG   "RAZOR_UP_STATE"
#define RAZOR_CUT_MSG  "RAZOR_CUT_MSG"

/* INPUT PINS */
#define SENSOR_1_PIN        18
#define SENSOR_2_PIN        17
#define BUTTON_A_PIN        19 //ok
#define BUTTON_B_PIN        23 //ok
#define VALIDATION_PIN      22 //ok

/* OUTPUT PINS */
#define LOCK_PIN        12
#define RAZOR_PIN       4
#define RED_LAMP_PIN    5
#define GREEN_LAMP_PIN  2

/* Reset signal counters */
SYSTEM_STATE state = READY;
int latch_cnt = 0;
int lock_move_cnt = 0;
int razor_move_cnt = 0;
int test_wait_cnt = 0;
int defect_time = 0;
bool flag_validation_mode = false;


/*init io and log by serial*/
void setup() {
  /* INPUT SETUP */
  pinMode(SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(VALIDATION_PIN, INPUT_PULLUP);
  /* OUTPUT SETUP */
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(RAZOR_PIN, OUTPUT);
  pinMode(RED_LAMP_PIN, OUTPUT);
  pinMode(GREEN_LAMP_PIN, OUTPUT);
  /* OUTPUT INITIAL STATES */
  digitalWrite(LOCK_PIN, RESET_LOCK);
  digitalWrite(RAZOR_PIN, RESET_RAZOR);
  digitalWrite(RED_LAMP_PIN, LAMP_OFF);
  digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
  /* SERIAL LOG */
  Serial.begin(9600);
}

/*main state machine*/
void loop() {
  int button_a = digitalRead(BUTTON_A_PIN);
  int button_b = digitalRead(BUTTON_B_PIN);
  int sensor_1 = digitalRead(SENSOR_1_PIN);
  int sensor_2 = digitalRead(SENSOR_2_PIN);
  int validation = digitalRead(VALIDATION_PIN);
  Serial.print(button_a);
  Serial.print(",");
  Serial.print(button_b);
  Serial.print(",");
  Serial.print(sensor_1);
  Serial.print(",");
  Serial.print(sensor_2);
  Serial.print(",");
  Serial.println(validation);

  
  
  switch(state){
     case WAIT_EXIT:
      if (button_a != BUTTON_PUSHED && button_b != BUTTON_PUSHED) {
        state = READY;
      }
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(WAIT_EXIT_MSG);
      break;
    
    case READY:
      if (sensor_2 == DOOR_CLOSED && (button_a == BUTTON_PUSHED || button_b == BUTTON_PUSHED)) {
        state = LATCH;
      }
      if (validation == BUTTON_PUSHED) {
        state = VALIDATION;
      }
      flag_validation_mode = false;
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(READY_MSG);
      break;

     case VALIDATION:
      if (sensor_2 == DOOR_CLOSED && (button_a == BUTTON_PUSHED || button_b == BUTTON_PUSHED)) {
        state = LATCH;
      }
      flag_validation_mode = true;
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(VALIDATION_MSG);
      break;
      
    case LATCH:
      if (sensor_2 != DOOR_CLOSED || button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED) {
        state = READY;
      }
      if (sensor_2 == DOOR_CLOSED && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && latch_cnt < LATCH_TIME) {
        state = SET_LOCK;
      }
      if (latch_cnt < LATCH_TIME) latch_cnt ++;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(LATCH_MSG);
      break;
      
    case SET_LOCK:
      if (sensor_2 != DOOR_CLOSED || button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED) {
        state = READY;
      }
      if (sensor_2 == DOOR_CLOSED && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && lock_move_cnt == LOCK_TIME) {
        state = TEST;
      }
      
      latch_cnt = 0;
      if (lock_move_cnt < LOCK_TIME) lock_move_cnt ++;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(SET_LOCK_MSG);
      break;
      
    case TEST:
      if ((sensor_1 != SENSORS_INPUT) && test_wait_cnt  >= TEST_TIME) {
        state = DEFECT;
      }
      if (sensor_1 == SENSORS_INPUT && test_wait_cnt  >= TEST_TIME) {
        state = UNLOCK;
      }
      
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      defect_time = 0;
      if (test_wait_cnt < TEST_TIME) test_wait_cnt ++;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(TEST_MSG);
      break;

    case DEFECT:
     // WAIT FOR USER HANDS TO BE SAFE
      if ((button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED)) {
        state = TEST;
      }
      if (button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && !flag_validation_mode) {
        state = RAZOR_CUT;
      }
      if (flag_validation_mode && defect_time == DEFECT_TIME) {
        state = UNLOCK;
      }
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      if (defect_time < DEFECT_TIME) defect_time ++;

      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(DEFECT_MSG);
      break;

     case RAZOR_CUT:
      if (sensor_2 != DOOR_CLOSED || button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED) {
        state = DEFECT;
      }
      if (sensor_2 == DOOR_CLOSED && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && razor_move_cnt == RAZOR_CUT_TIME) {
        razor_move_cnt = 0;
        state = RAZOR_UP;
      }
      if (razor_move_cnt < RAZOR_CUT_TIME) razor_move_cnt ++;
      lock_move_cnt = 0;
      latch_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(RAZOR_PIN, SET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(RAZOR_CUT_MSG);
      break;

     case RAZOR_UP:
      if (sensor_2 != DOOR_CLOSED || button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED) {
        state = RAZOR_UP;
      }
      if (sensor_2 == DOOR_CLOSED && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && razor_move_cnt == RAZOR_CUT_TIME) {
        razor_move_cnt = 0;
        state = UNLOCK;
      }
      if (razor_move_cnt < RAZOR_CUT_TIME) razor_move_cnt ++;
      lock_move_cnt = 0;
      latch_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(RAZOR_UP_MSG);
      break;
      
    case UNLOCK:
      if (sensor_2 == DOOR_CLOSED && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && lock_move_cnt == LOCK_TIME) {
        state = WAIT_EXIT;
      }

      latch_cnt = 0;
      if (lock_move_cnt < LOCK_TIME) lock_move_cnt ++;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;

      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(UNLOCK_MSG);
      break;
      
    default:
      latch_cnt = 0;
      lock_move_cnt = 0;
      razor_move_cnt = 0;
      test_wait_cnt = 0;
      defect_time = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(RAZOR_PIN, RESET_RAZOR);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(HARD_FAULT_MSG);
      break;
  }
  delay(CLOCK_DELAY);
}
