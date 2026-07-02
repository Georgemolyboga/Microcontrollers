# FSM Plan

## Basic idea
```c
Example with simple “radar distance” data:
DATA:80   -> enough space      -> move_forward()
DATA:35   -> getting close     -> move_left()
DATA:10   -> obstacle too near -> stop_motors()
```

## Modelling variables 
```c
typedef enum {
    STATE_WAIT_FOR_DATA,
    STATE_PARSE_DATA,
    STATE_DECIDE_ACTION,
    STATE_EXECUTE_ACTION,
    STATE_ERROR
} control_state_t;

typedef enum {
    MOVE_FORWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_BACKWARD,
    MOVE_STOP
} robot_movement_t;
```

## State transitions
1. Initialize variables and system. Enter STATE_WAIT_FOR_DATA
2. STATE_WAIT_FOR_DATA
    - If nothing happens, stay in the same state
    - Else, if data is received / buffer not empty, enter STATE_PARSE_DATA
3. STATE_PARSE_DATA
    - Parse one block, and if valid enter STATE_DECIDE_ACTION
    - Else, if invalid, enter STATE_ERROR (prints error, skips actions, and enters STATE_WAIT_FOR_DATA)
4. STATE_DECIDE_ACTION
    - Process the parsed data and give an output of type robot_movement_t
    - Enter STATE_EXECUTE_ACTION
5. STATE_EXECUTE_ACTION
    - switch(condition), and call the motor blueprint functions