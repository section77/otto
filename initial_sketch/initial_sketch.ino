#include <Servo.h>
#include <Oscillator.h>
#include <US.h>
#include <Otto.h>

/*
 * servo pins
 *
 *          +---------------+
 *          |     O   O     |
 *          |---------------|
 * YR 3==>  |               | <== YL 2
 *          +---------------+
 *              ||     ||
 * RR 5==>     +--+   +--+    <== RL 4
 *             +--+   +--+
 */
#define PIN_YL 2 // linkes bein
#define PIN_YR 3 // rechtes bein
#define PIN_RL 4 // linker fuss
#define PIN_RR 5 // rechter fuss

// logging on / off (true / false)
#define SERIAL_DEBUGGING_ENABLED true

enum mode_t {
             NOOP,         // make nothing
             WALK,         // walk
             CALIBRATION,  // calibration modus
};
mode_t mode = NOOP;


Otto Otto;
void setup(){
  if(SERIAL_DEBUGGING_ENABLED) {
    Serial.begin(9600);
    Serial.println();
    Serial.println();
  }

  // initialize servo pins
  debug("initialize servo pins - YL: %d, YR: %d, RL: %d, RR: %d", PIN_YL, PIN_YR, PIN_RL, PIN_RR);
  Otto.init(PIN_YL,PIN_YR,PIN_RL,PIN_RR, true);

  Otto.home();

  if(isCalStartTriggerSequence()) {
    mode = CALIBRATION;
    return;
  }

  // default program
  mode = WALK;
  Otto.sing(S_connection);
}


void loop() {
  switch(mode) {
  case NOOP:
    debug("NOOP");
    delay(1000);
    break;
  case WALK:
    if(obstacleDetected()) {
      // shake
      Otto.moonwalker(20, 100, 30);

      // turn
      Otto.turn(4, 1000);

      // JUMP!
      Otto.jump(1, 150);
    }

    Otto.walk(2, 500);
    break;
  case CALIBRATION:
    calibrate_otto();
    mode = NOOP;
    break;
  default:
    debug("unhandled mode - number: %d", mode);
  }
}


int obstacleDetected() {
  const int threshold = 20;
  const int dist = Otto.getDistance();
  debug("distance sensor-value: %d, obstacle detected: %b", dist, dist < threshold);
  return dist < threshold;
}
