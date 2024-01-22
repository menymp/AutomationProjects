/*
 *  metal tester fixture controller v1
 *  
 *  Simple controller for metal bracket tester
 *  
 *  menymp
 *  Dic 2023
 *  
 *  YYYY/MM/DD  NAME    CHANGE
 *  2024/01/05  menymp  change logic to add two sensors and prove extension
 *                      I/O layout change to use ESP32 carrier board
 *  2024/01/21  menymp  add intermediate supervisor state to avoid race
 *                      conditions.
 *  2024/01/21  menymp  fix ready mode race condition, adjust times, adjusts
 *                      default sensor detection states, fix supervisor msg
 */
/* FSM STATES */
typedef enum {
  READY,
  LATCH,
  SET_LOCK,
  SET_PROVE,
  TEST,
  UNLOCK,
  DEFECT,
  SUPERVISOR,
  WAIT_EXIT
}SYSTEM_STATE;

/* INPUTS DEFINITIONS */
#define METAL_SENSOR_ACTIVE     false
#define BUTTON_PUSHED           true
#define SUPERVISOR_ACTIVE       true
/* OUTPUTS DEFINITIONS */
#define LAMP_ON                 true
#define LAMP_OFF                false
#define SET_LOCK_OUT            true
#define RESET_LOCK              false
#define SET_PROVE_OUT           true
#define RESET_PROVE             false
/* CLOCK DELAY */
#define CLOCK_DELAY             100
/* STATES DELAY COUNTS */
/* estimated with pneumatic moves since there are no sensors */
#define LATCH_TIME              4
#define LOCK_TIME               4
#define PROVE_TIME              4
#define TEST_TIME               4

/*log messages definitions*/
#define READY_MSG      "READY_STATE"
#define LATCH_MSG      "LATCH_STATE"
#define LOCK_MSG       "LOCK_STATE"
#define SET_LOCK_MSG   "SET_LOCK_STATE"
#define SET_PROVE_MSG  "SET_PROVE_STATE"
#define TEST_MSG       "TEST_STATE"
#define DEFECT_MSG     "DEFECT_STATE"
#define UNLOCK_MSG     "UNLOCK_STATE"
#define SUPERVISOR_MSG "SUPERVISOR_STATE"
#define WAIT_EXIT_MSG  "WAIT_EXIT_STATE"
#define HARD_FAULT_MSG "HARD_FAULT"

/* INPUT PINS */
#define SENSOR_1_PIN        18
#define SENSOR_2_PIN        19
#define BUTTON_A_PIN        15
#define BUTTON_B_PIN        2
#define SUPERVISOR_LOCK_PIN 4

/* OUTPUT PINS */
#define LOCK_PIN        13
#define PROVE_PIN       12
#define RED_LAMP_PIN    27
#define GREEN_LAMP_PIN  14

/* Reset signal counters */
SYSTEM_STATE state = READY;
int latch_cnt = 0;
int lock_move_cnt = 0;
int prove_move_cnt = 0;
int test_wait_cnt = 0;


/*init io and log by serial*/
void setup() {
  /* INPUT SETUP */
  pinMode(SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(SUPERVISOR_LOCK_PIN, INPUT_PULLUP);
  /* OUTPUT SETUP */
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(PROVE_PIN, OUTPUT);
  pinMode(RED_LAMP_PIN, OUTPUT);
  pinMode(GREEN_LAMP_PIN, OUTPUT);
  /* OUTPUT INITIAL STATES */
  digitalWrite(LOCK_PIN, RESET_LOCK);
  digitalWrite(PROVE_PIN, RESET_PROVE);
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
  int supervisor_lock = digitalRead(SUPERVISOR_LOCK_PIN);
  Serial.print(button_a);
  Serial.print(",");
  Serial.print(button_b);
  Serial.print(",");
  Serial.print(sensor_1);
  Serial.print(",");
  Serial.print(sensor_2);
  Serial.print(",");
  Serial.println(supervisor_lock);
  
  
  switch(state){
     case WAIT_EXIT:
      if (button_a != BUTTON_PUSHED && button_b != BUTTON_PUSHED) {
        state = READY;
      }
      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(WAIT_EXIT_MSG);
      break;
    
    case READY:
      if (supervisor_lock != SUPERVISOR_ACTIVE && (button_a == BUTTON_PUSHED || button_b == BUTTON_PUSHED)) {
        state = LATCH;
      }
      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(READY_MSG);
      break;
      
    case LATCH:
      if (supervisor_lock == SUPERVISOR_ACTIVE || (button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED)) {
        state = READY;
      }
      if (supervisor_lock != SUPERVISOR_ACTIVE && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && latch_cnt < LATCH_TIME) {
        state = SET_LOCK;
      }
      if (latch_cnt < LATCH_TIME) latch_cnt ++;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(LATCH_MSG);
      break;
      
    case SET_LOCK:
      if (supervisor_lock == SUPERVISOR_ACTIVE || (button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED)) {
        state = READY;
      }
      if (supervisor_lock != SUPERVISOR_ACTIVE && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && lock_move_cnt == LOCK_TIME) {
        state = SET_PROVE;
      }
      
      latch_cnt = 0;
      if (lock_move_cnt < LOCK_TIME) lock_move_cnt ++;
      prove_move_cnt = 0;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(SET_LOCK_MSG);
      break;
      
    case SET_PROVE:
      if (supervisor_lock == SUPERVISOR_ACTIVE || (button_a != BUTTON_PUSHED || button_b != BUTTON_PUSHED)) {
        state = UNLOCK;
      }
      if (supervisor_lock != SUPERVISOR_ACTIVE && button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && prove_move_cnt == PROVE_TIME) {
        state = TEST;
      }
      
      latch_cnt = 0;
      lock_move_cnt = 0;
      if (prove_move_cnt < PROVE_TIME) prove_move_cnt ++;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, SET_PROVE_OUT);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(SET_PROVE_MSG);
      break;
      
    case TEST:
      if (supervisor_lock == SUPERVISOR_ACTIVE) {
        state = SUPERVISOR;
      }
      if ((sensor_1 != METAL_SENSOR_ACTIVE || sensor_2 != METAL_SENSOR_ACTIVE) && test_wait_cnt  >= TEST_TIME) {
        state = DEFECT;
      }
      if (sensor_1 == METAL_SENSOR_ACTIVE && sensor_2 == METAL_SENSOR_ACTIVE && test_wait_cnt  >= TEST_TIME) {
        state = UNLOCK;
      }
      
      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      if (test_wait_cnt < TEST_TIME) test_wait_cnt ++;
      
      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, SET_PROVE_OUT);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(TEST_MSG);
      break;
      
    case UNLOCK:
      if (button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED && lock_move_cnt == LOCK_TIME) {
        state = WAIT_EXIT;
      }

      latch_cnt = 0;
      if (lock_move_cnt < LOCK_TIME) lock_move_cnt ++;
      prove_move_cnt = 0;
      test_wait_cnt = 0;

      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(UNLOCK_MSG);
      break;
      
    case DEFECT:
      if (supervisor_lock == SUPERVISOR_ACTIVE) {
        state = SUPERVISOR;
      }

      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;

      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(DEFECT_MSG);
      break;
      
    case SUPERVISOR:
      if (supervisor_lock != SUPERVISOR_ACTIVE && (button_a == BUTTON_PUSHED && button_b == BUTTON_PUSHED)) {
        state = UNLOCK;
      }

      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;

      digitalWrite(LOCK_PIN, SET_LOCK_OUT);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      Serial.println(SUPERVISOR_MSG);
      break;
      
    default:
      latch_cnt = 0;
      lock_move_cnt = 0;
      prove_move_cnt = 0;
      test_wait_cnt = 0;
      
      digitalWrite(LOCK_PIN, RESET_LOCK);
      digitalWrite(PROVE_PIN, RESET_PROVE);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      Serial.println(HARD_FAULT_MSG);
      break;
  }
  delay(CLOCK_DELAY);
}
