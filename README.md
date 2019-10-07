# OTTO

  | Folder          | Description                                      |
  |-----------------|--------------------------------------------------|
  | initial_sketch/ | initial sketch for the s77 otto builder workshop |



## initial_sketch/

This sketch supports two modes:

 - walk and detect obstacles
 - calibration mode

### walk (default mode)

In this mode OTTO walks and he detects a obstacle,
he makes a turn and walks again

### calibration mode

#### steps to active this mode

  - obstacle expected ~14cm
  - pause for 1500ms
  - no obstacle expected
  - pause for 1500ms
  - obstacle expected ~14cm
  - pause for 1500ms
  - no obstacle expected

#### calibration

for each servo, this mode executes the following sequence

  - juggle the actual servo
  - calibrate the actual servo
    - calculate the trims from the obstacle distance
    - play with the distance and you see the servos moves
    - if you don't change the distance for ~2 seconds, the
      next servo are selected
  - pause for 1000ms


if otto detects a obstacle for ~4 seconds for his eyes,
the new values will saved to EEPROM (otto beeps the whole time).
if you remove the obstacle in this period, nothing will saved.
