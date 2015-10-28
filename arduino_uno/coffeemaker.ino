/*
 * Circuit to test coffee maker control
 */

#define ERROR_LED 4
#define LOG_DELAY 1000

#define CM_SWITCH 2
#define CM_POWER_LED 6
#define PIN_DELAY 50

#define TRANSISTOR 7
#define T_CTRL 8
#define T_HOLD 200

#define POWER_READ 0
#define POWER_READ_T 300
#define POWER_DETECT 10


int cm_switch_val = HIGH;
int cm_switch_cur = LOW;
int cm_switch_delay = 0;

int t_ctrl_val = HIGH;
int t_ctrl_cur = LOW;
int t_ctrl_delay = 0;

boolean cm_power = false;
boolean t_power = false;
int t_time = 0;

int power_read = 0;

int log_time = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ERROR_LED, OUTPUT);
  pinMode(CM_POWER_LED, OUTPUT);
  pinMode(CM_SWITCH, INPUT);
  pinMode(T_CTRL, INPUT);
  pinMode(TRANSISTOR, OUTPUT);
  digitalWrite(TRANSISTOR, LOW);

  pinMode(POWER_DETECT, OUTPUT);
}

/*
 * Generic function to switch LED state from on to off and off to on
 */
void switchLED(int pin) {
  int val = digitalRead(pin);
  if (val == 0) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

/*
 * Toggle power of the coffee maker
 * Reads power led to ensure it matches internal state
 * Turns on or off coffee maker depending on previous state
 * If there is an issue with the state, the error led turns on
 */
void toggleCMPower() {
  int val = digitalRead(CM_POWER_LED);
  if ((cm_power == true && val == HIGH)
      || (cm_power == false && val == LOW)) {
    cm_power = !cm_power;
    switchLED(CM_POWER_LED);      
  } else {
    digitalWrite(ERROR_LED, HIGH);
  }
}

/*
 * Verify transistor power should be called as part of main loop
 * When the transistor is blipped the power state is turned on and
 * a counter is set to determine how long the transistor should
 * stay powered on. Verify transistor power will decrement the
 * counter and eventually turn off the transistor after it has been
 * held high, long enough for the coffee maker to be powered on.
 */
void verifyTransistorPower() {
  int val = digitalRead(TRANSISTOR);
  if (t_power == true && val == HIGH) {
    if (t_time == 0) {
      t_power = false;
      digitalWrite(TRANSISTOR, LOW);
    } else {
      t_time--;
    }
  } else if (t_power == true && val == LOW) {
    t_time = T_HOLD;
    digitalWrite(TRANSISTOR, HIGH);
  } else if (t_power == false && val == HIGH) {
    digitalWrite(ERROR_LED, HIGH);
  }
}

/*
 * Blimp transistor simply sets state so that on the next iteration of
 * the main loop, the verify transistor power function can turn on the
 * transistor.
 */
void blipTransistor() {
  if (t_power == true) { return; }
  t_power = true;
}

/*
 * Logic for handling button state.
 * Delays prevent multiple button presses actions from occuring for a
 * single long press of the button.
 */
void toggleButtons() {
  if (((cm_switch_cur = digitalRead(CM_SWITCH)) == LOW)
      && (cm_switch_cur != cm_switch_val)
      && (cm_switch_delay == 0)) {
    //Serial.println("CM_SWITCH pressed");
    cm_switch_val = cm_switch_cur;
    cm_switch_delay = PIN_DELAY;
    toggleCMPower();
  } else if (cm_switch_cur == HIGH) {
    cm_switch_val = HIGH;
  }

  if (((t_ctrl_cur = digitalRead(T_CTRL)) == LOW)
      && (t_ctrl_cur != t_ctrl_val)
      && (t_ctrl_delay == 0)) {
    //Serial.println("T_CTRL pressed");
    t_ctrl_val = t_ctrl_cur;
    t_ctrl_delay = PIN_DELAY;
    blipTransistor();
  } else if (t_ctrl_cur == HIGH) {
    t_ctrl_val = HIGH;
  }

  if (cm_switch_delay != 0) { cm_switch_delay--; }
  if (t_ctrl_delay != 0) { t_ctrl_delay--; }
}

void detectPower() {
  if ((power_read = analogRead(POWER_READ)) > POWER_READ_T) {
    digitalWrite(POWER_DETECT, HIGH);    
  } else {
    digitalWrite(POWER_DETECT, LOW);
  }
  if (log_time == 0) {
    Serial.print(power_read);
    Serial.println(" power read");
  }
}

/*
 * Toggle buttons will check the input buttons for changes
 * If a change in the button is detected, counters are set to prevent
 * multiple button press events from occuring for a single press.
 * If the transistor button is pressed then state is set so that the
 * transistor can be blipped. The verify transistor power will detect
 * state change and properly adjust transistor power and decrement
 * transistor time counter so the transistor will be blipped for the
 * correct amount of time.
 */
void loop() {
  toggleButtons();
  verifyTransistorPower();
  detectPower();
  if (log_time == 0) {
    log_time = LOG_DELAY;
  } else {
    log_time--;
  }
  delay(1);
}
