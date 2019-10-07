//
// calibration
//
#include <Otto.h>

#define CAL_TRIGGER_SEQ_COUNT 2
#define CAL_TRIGGER_SEQ_MATE_BOTTLE_HEIGHT 24
#define CAL_TRIGGER_SEQ_OTTO_BODY_DEPTH 7
#define CAL_TRIGGER_SEQ_DIST (CAL_TRIGGER_SEQ_MATE_BOTTLE_HEIGHT - CAL_TRIGGER_SEQ_OTTO_BODY_DEPTH)
#define CAL_TRIGGER_SEQ_DIST_TOLERANCE 10
#define CAL_TRIGGER_SEQ_TONE note_C5
#define CAL_TRIGGER_SEQ_TONE_DUR 100
#define CAL_TRIGGER_SEQ_OK_TONE note_C5
#define CAL_TRIGGER_SEQ_OK_TONE_DUR 1000
#define CAL_TRIGGER_SEQ_ABORT_TONE note_C2
#define CAL_TRIGGER_SEQ_ABORT_TONE_DUR 1000


#define CAL_DIST_THRESHOLD 1
#define CAL_DIST_OFFSET 50
#define CAL_DIST_FACTOR 1.5


#define CAL_DONE_HOLD_COUNTER 4
#define CAL_DONE_HOLD_TONE note_C5
#define CAL_DONE_HOLD_TONE_DUR 100

#define CAL_SAVE_HOLD_DIST 50
#define CAL_SAVE_HOLD_DIST_TOLERANCE 50
#define CAL_SAVE_HOLD_COUNTER 4
#define CAL_SAVE_HOLD_TONE note_C5
#define CAL_SAVE_HOLD_TONE_DUR 500

#define CAL_SAVE_DONE_TONE note_C5
#define CAL_SAVE_DONE_TONE_DUR 1000

#define CAL_SAVE_ABORT_TONE note_C2
#define CAL_SAVE_ABORT_TONE_DUR 1000


/*
 * calibration sequence
 *
 * this functions checks if the following sequence was successful and
 * returns the check result.
 *
 *   for `n` times where n = CAL_TRIGGER_SEQ_COUNT
 *    - obstacle expected (at distance: CAL_TRIGGER_SEQ_DIST (+/- CAL_TRIGGER_SEQ_DIST_TOLERANCE))
 *    - sleep 1500ms
 *    - no obstacle expected
 *    - sleep 1500ms
 *
 */
boolean isCalStartTriggerSequence() {
  boolean isCalTriggerSeq = true;

  for(int i = 0; i < CAL_TRIGGER_SEQ_COUNT; i++) {
    debug("calibration start sequence (%d/%d) ...", i + 1, CAL_TRIGGER_SEQ_COUNT);
    if(isObstacleAtCalTriggerPos()) {
      Otto._tone(CAL_TRIGGER_SEQ_TONE, CAL_TRIGGER_SEQ_TONE_DUR, 0);
      delay(1500);
      if(isObstacleAtCalTriggerPos()) {
        isCalTriggerSeq = false;
        break;
      }
      Otto._tone(CAL_TRIGGER_SEQ_TONE, CAL_TRIGGER_SEQ_TONE_DUR, 0);
      delay(1500);
    } else {
      isCalTriggerSeq = false;
      break;
    }
  }
  debug("calibration start sequence done - start sequence successful: %b", isCalTriggerSeq);

  if(isCalTriggerSeq) {
    Otto._tone(CAL_TRIGGER_SEQ_OK_TONE, CAL_TRIGGER_SEQ_OK_TONE_DUR, 0);
  } else {
    Otto._tone(CAL_TRIGGER_SEQ_ABORT_TONE, CAL_TRIGGER_SEQ_ABORT_TONE_DUR, 0);
  }

  return isCalTriggerSeq;
}

/*
 * check if a obstacle is at CAL_TRIGGER_SEQ_DIST (+/- CAL_TRIGGER_SEQ_DIST_TOLERANCE))
 */
boolean isObstacleAtCalTriggerPos() {
  int dist_diff = abs(Otto.getDistance() - CAL_TRIGGER_SEQ_DIST);
  debug("obstacle detection - dist: %d, result: %b", dist_diff, (dist_diff < CAL_TRIGGER_SEQ_DIST_TOLERANCE));
  return dist_diff < CAL_TRIGGER_SEQ_DIST_TOLERANCE;
}


/*
 * calibrate all servos
 *
 *  - for each servo
 *    - juggle the actual servo
 *    - start the calibration for the actual servo
 *  - if we see a obstacle within CAL_SAVE_HOLD_DIST for CAL_SAVE_HOLD_COUNTER times,
 *    save the new values to the EEPROM
 */
void calibrate_otto() {
  debug("calibrate otto - distance threshold: %d, distance offset: %d, distance factor: %f",
        CAL_DIST_THRESHOLD, CAL_DIST_OFFSET, CAL_DIST_FACTOR);

  // calibrate each servo
  int trims[4] = {0};
  for(int idx = 0; idx < 4; idx++) {
    juggle_servo(idx);
    calibrate(idx, trims);
    delay(1000);
  }

  // ask the user if we should save the new offsets / trims
  for(int i = 0; i < CAL_SAVE_HOLD_COUNTER; i++) {
    int dist = Otto.getDistance();
    if(abs(dist - CAL_SAVE_HOLD_DIST) < CAL_SAVE_HOLD_DIST_TOLERANCE) {
      debug("hold... (%d/%d)", i + 1, CAL_SAVE_HOLD_COUNTER);
      Otto._tone(CAL_SAVE_HOLD_TONE, CAL_SAVE_HOLD_TONE_DUR, 0);
      delay(500);
      } else {
      // abort
      debug("abort - don't save to EEPROM");
      Otto._tone(CAL_SAVE_ABORT_TONE, CAL_SAVE_ABORT_TONE_DUR, 0);
      return;
    }
  }

  // save the offsets / trims
  debug("save trims (%d, %d, %d, %d) to EEPROM",
        trims[0], trims[1], trims[2], trims[3]);
  Otto._tone(CAL_SAVE_DONE_TONE, CAL_SAVE_DONE_TONE_DUR, 0);
  Otto.saveTrimsOnEEPROM();
}

/*
 * calibrate a single servo
 *
 */
void calibrate(int idx, int trims[4]) {
  int hold_counter = 0;

  int dist_last = Otto.getDistance();
  while(true) {
    int dist = Otto.getDistance();
    int dist_diff = abs(dist - dist_last);
    debug("idx: %d, dist: %d, dist_last: %d, diff: %d", idx, dist, dist_last, dist_diff);

    if(dist_diff > 100) {
      debug("difference > 100 - ignore measurement");
      continue;
    }

    // distance difference detected?
    if(dist_diff > CAL_DIST_THRESHOLD) {
      hold_counter = 0;

      int trim = (dist - CAL_DIST_OFFSET) * CAL_DIST_FACTOR;
      debug("idx: %d, trim: %d", idx, trim);
      trims[idx] = trim;

      Otto.setTrims(trims[0], trims[1], trims[2], trims[3]);
      Otto.setRestState(false);
      Otto.home();
    } else if(hold_counter >= CAL_DONE_HOLD_COUNTER) {
      Otto._tone(CAL_SAVE_DONE_TONE, CAL_SAVE_DONE_TONE_DUR, 0);
      delay(500);
      break;
    } else {
      hold_counter++;
      Otto._tone(CAL_DONE_HOLD_TONE, CAL_DONE_HOLD_TONE_DUR, 0);
    }
    dist_last = dist;
    delay(500);
  }
}



/*
 * juggle a single servo
 *
 *  - expect the servo index from 0 to 3
 */
void juggle_servo(int idx) {
  debug("juggle_servo(idx = %d)", idx);

  // start with the "home" position
  int servos[4] = {90, 90, 90, 90};
  Otto._moveServos(100, servos);
  servos[idx] = 30;
  Otto._moveServos(100, servos);
  servos[idx] = 90;
  Otto._moveServos(100, servos);
}
