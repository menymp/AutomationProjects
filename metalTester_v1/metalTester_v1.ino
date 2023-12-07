/*
 *  metal tester fixture controller v1
 *  
 *  Simple controller for metal bracket tester
 *  
 *  menymp
 *  Dic 2023
 */
typedef enum {
  READY,
  SUPERVISOR_INSPECTION,
  BUSY,
  DEFECTIVE_STATE,
  LATCH_ACTIVE
}SYSTEM_STATE;

#define METAL_SENSOR_ACTIVE     true
#define LAMP_ON                 true
#define LAMP_OFF                false
#define CLAMP_LOCKED            true
#define CLAMP_FREE              false
#define SUPERVISOR_LOCK_ACTIVE  true
#define BUTTON_PUSHED           true

#define TICK_TIME            100
#define BUSY_TICKS            20
#define INTERLOCK_MAX_TIME     3
/*log messages definitions*/
#define HARD_FAULT_MSG      "HARD_FAULT_ERR"
#define SUPERVISOR_MSG      "SUPERVISOR_STATE"
#define DEFECTIVE_STATE_MSG "BUSY_STATE"
#define READY_STATE_MSG     "READY_STATE"
#define BUSY_STATE_MSG      "BUSY_STATE_MSG"
#define LATCH_MSG           "INTERLOCK_REACHED"

//Operator buttons
const int BUTTON_A_PIN = 12;
const int BUTTON_B_PIN = 11;
const int SUPERVISOR_LOCK_PIN = 10;
const int METAL_SENSOR_PIN = 9;
//outputs
const int GREEN_LAMP_PIN = 7;
const int RED_LAMP_PIN = 6;
const int CLAMP_PIN = 5;

SYSTEM_STATE state = READY;
int busy_cnt = 0;
int latch_cnt = 0;

/*init io and log by serial*/
void setup() {
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(SUPERVISOR_LOCK_PIN, INPUT_PULLUP);
  pinMode(METAL_SENSOR_PIN, INPUT_PULLUP);

  pinMode(GREEN_LAMP_PIN, OUTPUT);
  pinMode(RED_LAMP_PIN, OUTPUT);
  pinMode(CLAMP_PIN, OUTPUT);
  //Start in debugging mode
  Serial.begin(9600);
}

/*main state machine*/
void loop() {
  switch(state){
    case READY:
      if(digitalRead(SUPERVISOR_LOCK_PIN) == SUPERVISOR_LOCK_ACTIVE){
        state = SUPERVISOR_INSPECTION;
      }
      if(digitalRead(BUTTON_A_PIN) != digitalRead(BUTTON_B_PIN)){
        latch_cnt++;
      }
      if(latch_cnt > INTERLOCK_MAX_TIME){
        /*TIMEOUT reached, going to latch*/
        state = LATCH_ACTIVE;
      }else if (digitalRead(BUTTON_A_PIN) == BUTTON_PUSHED && digitalRead(BUTTON_B_PIN) == BUTTON_PUSHED) {
        state = BUSY;
      }
      digitalWrite(GREEN_LAMP_PIN, LAMP_ON);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(CLAMP_PIN, CLAMP_FREE);
      Serial.println(READY_STATE_MSG);
      break;
      
    case LATCH_ACTIVE:
      if(digitalRead(BUTTON_A_PIN) != BUTTON_PUSHED && digitalRead(BUTTON_B_PIN) != BUTTON_PUSHED){
        state = READY;
        latch_cnt = 0;
      }
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      digitalWrite(RED_LAMP_PIN, !digitalRead(RED_LAMP_PIN));
      digitalWrite(CLAMP_PIN, CLAMP_FREE);
      Serial.println(LATCH_MSG);
      break;
      
    case SUPERVISOR_INSPECTION:
      /*in this state, system waits for the supervisor to remove the key*/
      if(digitalRead(SUPERVISOR_LOCK_PIN) != SUPERVISOR_LOCK_ACTIVE){
        state = SUPERVISOR_INSPECTION;
      }
      digitalWrite(GREEN_LAMP_PIN, !digitalRead(GREEN_LAMP_PIN));
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(CLAMP_PIN, CLAMP_FREE);
      Serial.println(SUPERVISOR_MSG);
      break;
      
    case BUSY:
      if(digitalRead(SUPERVISOR_LOCK_PIN) == SUPERVISOR_LOCK_ACTIVE){
        state = SUPERVISOR_INSPECTION;
      }
      if (digitalRead(METAL_SENSOR_PIN) != METAL_SENSOR_ACTIVE){
        state = DEFECTIVE_STATE;
      } else if (busy_cnt > BUSY_TICKS) {
        state = READY;
      } else {
        busy_cnt ++;
      }
      digitalWrite(GREEN_LAMP_PIN, !digitalRead(GREEN_LAMP_PIN));
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(CLAMP_PIN, CLAMP_LOCKED);
      Serial.println(BUSY_STATE_MSG);
      break;
      
    case DEFECTIVE_STATE:
      if(digitalRead(SUPERVISOR_LOCK_PIN) == SUPERVISOR_LOCK_ACTIVE){
        state = SUPERVISOR_INSPECTION;
      }
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      digitalWrite(RED_LAMP_PIN, LAMP_ON);
      digitalWrite(CLAMP_PIN, CLAMP_LOCKED);
      Serial.println(DEFECTIVE_STATE_MSG);
      break;
      
    default:
      digitalWrite(GREEN_LAMP_PIN, LAMP_OFF);
      digitalWrite(RED_LAMP_PIN, LAMP_OFF);
      digitalWrite(CLAMP_PIN, CLAMP_FREE);
      Serial.println(HARD_FAULT_MSG);
      break;
  }
  delay(TICK_TIME);
}
