/*
 *  Code finalised in July 2019.
 */


/* 
 *  The Brushless DC HDD motor is three phased meaning it has
 *  three control lines that need to be pulsed correctly
 *  Pulses to the three motor phases overlap as follows.
 * 
 *  stage  0      1      2   |   0      1      2      ...
 *        __________            __________
 *  p1 __|          |__________|          |_________  ...
 *               __________            __________
 *  p2 _________|          |__________|          |___ ...
 *     ______            __________            _____
 *  p3       |__________|          |__________|       ...
 * 
 *  Notice how the pattern repeats after three "stages".
 * 
 *  BLDC motor takes 12 steps (4 phase cycles) to complete a full revolution.
 */

//TODO:
// get rid of floats
// slower speed

//digital pin nums of the three motor pins: p1, p2, p3
#define P1_PIN 2
#define P2_PIN 3
#define P3_PIN 4

//analogue pin nums for the two LDRs
#define LDR1_PIN 6
#define LDR2_PIN 7

//how many time is a pin 
#define STEPS_PER_REV 12

//some little macros
#define CLAMP(X) ((X) > 1 ? 1 : X)
#define ABS(X) ((X) < 0 ? -X : X)
//gives pulse interval time from rpm speed
#define PULSE_US_FROM_RPM(R) ((1000000 * 60) / ((R) * STEPS_PER_REV)) //[1000us for 5000rpm]
//gives accel interval from accel rate
#define INTERVAL_US_FROM_RATE(R) (1000000 / ABS((R)))
//gives pulse end time form start time
#define P_END_US(T) ((T) + (1 + 2 * overlap) * pulse_length_us)

#define PID_EVAL_US 1000 //time between re-evaluating PIDs

/////////////////////////////////////////////////////////////////////
/*** TUNING ***/
/* 
 *  In theory, these are the only constants that need changing.
 */

//PID tuning
#define KP 2
#define KD 0

#define INITIAL_ACCEL 270 //unit: rpm/sec
#define STARTUP_RPM 40
#define MAX_RPM 5000 //really 5400 but that is pushing it!

/*
 *  The overlap of the motor phases is designed to be proportional
 *  to the current rpm... when at slowest speed, the amount of
 *  overlap as a proportion is MAX_OVERLAP and max speed, MIN_OVERLAP.
 *  This is because more overlap is required at slower speeds. But a
 *  higher overlap draws more current, so the motor heats up more.
 */

#define MAX_OVERLAP 0.15  //overlap when current_rpm = 0
#define MIN_OVERLAP -0.2 //overlap when current_rpm = MAX_RPM

//////////////////////////////////////////////////////////////////////

//fraction of pulse_length that overlaps into next phase's pulse
float overlap = MAX_OVERLAP;

uint16_t current_rpm = STARTUP_RPM;
int16_t acceleration = INITIAL_ACCEL;

uint8_t has_done_initial_speed_up = 0;

//see the comment at start of loop() for reason for these
//the `us` suffix indicates a time in microseconds (10^{-6} seconds)
uint32_t last_pulse_us;
uint32_t last_accel_us;
uint32_t last_pid_us;
uint32_t loop_us;

//length of a pulse; ignoring the overlap - so really length of a stage
uint32_t pulse_length_us;
uint8_t stage = 0; //refer to ASCII for meaning; stage in {0,1,2}
//start times for each of the three phases
uint32_t p1_start_us, p2_start_us, p3_start_us;
//end times for each of the three phases
uint32_t p1_end_us, p2_end_us, p3_end_us;

//last error reading from the LDRs for derivative term of PID
int16_t last_error = 0;

//mainloop returns immediate if truthy (set in finish() function)
uint8_t done = 0;

//DEBUGGING
#define DEBUG 0
#define SAMPLE_INTERVAL_MS 10
#define MEM_SIZE 500
int16_t memory[MEM_SIZE];
uint16_t memory_ind = 0;
uint8_t has_splurged = 0;
uint32_t last_mem_us;

void setup() {
    //set all pulses as outputs
    pinMode(P1_PIN, OUTPUT);
    pinMode(P2_PIN, OUTPUT);
    pinMode(P3_PIN, OUTPUT);
    pinMode(13, OUTPUT); //on-board 'L' LED will light when demo is live
    last_pulse_us = last_accel_us = last_pid_us = last_mem_us = micros();
    p1_start_us = p2_start_us = p3_start_us = UINT32_MAX;
    if (DEBUG) Serial.begin(9600);
    pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
    digitalWrite(13, LOW);
}

void loop() {
    /*
     *  The mainloop is design to never block for anything. It cycles
     *  as fast as possible - mainly relooping without changing anything.
     *  This is because it must coordinate different tasks at the same
     *  time: updating the timings for the three motor pins (and setting
     *  the digital pins to what they should be); updating the current RPM
     *  depending on what the current acceleration is; and updating the
     *  acceleration when it is time to via the PID system.
     *
     *  There is also an additional scope for printing some debugging info
     *  to Serial. This only runs when DEBUG is truthy.
     */
    if (done) return;
    loop_us = micros();
    if ((loop_us - last_pulse_us) >= pulse_length_us){
        last_pulse_us += pulse_length_us;
        switch (stage = (stage + 1) % 3){
            //in each case, we set the time to start the pulse for the next phase
            case 0:
                p2_start_us = last_pulse_us + (1 - overlap) * pulse_length_us; break;
            case 1:
                p3_start_us = last_pulse_us + (1 - overlap) * pulse_length_us; break;
            case 2:
                p1_start_us = last_pulse_us + (1 - overlap) * pulse_length_us; break;
        }
    }
    
    //Only set a pin high if it is after its "start time" and before the
    //end time which is calculated on-the-fly.
    digitalWrite(P1_PIN, (p1_start_us < loop_us) && (loop_us < P_END_US(p1_start_us)));
    digitalWrite(P2_PIN, (p2_start_us < loop_us) && (loop_us < P_END_US(p2_start_us)));
    digitalWrite(P3_PIN, (p3_start_us < loop_us) && (loop_us < P_END_US(p3_start_us)));
    
    if (acceleration && (loop_us - last_accel_us) >= INTERVAL_US_FROM_RATE(acceleration)){
        last_accel_us += INTERVAL_US_FROM_RATE(acceleration);
        if (has_done_initial_speed_up){
            current_rpm += acceleration > 0 ? 1 : -1;
            if (current_rpm > MAX_RPM) current_rpm = MAX_RPM;
            if (current_rpm == 0) finish(); //unsigned so no point in <= 0
            pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
            overlap = MAX_OVERLAP - (MAX_OVERLAP - (MIN_OVERLAP)) * current_rpm / MAX_RPM;
        } else {
            if (current_rpm == MAX_RPM){
                has_done_initial_speed_up = 1;
                acceleration = 0;
                digitalWrite(13, HIGH);
                last_pid_us = last_mem_us = loop_us;
            } else {
                current_rpm += 1;
                pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
                overlap = MAX_OVERLAP - (MAX_OVERLAP - (MIN_OVERLAP)) * current_rpm / MAX_RPM;
            }
        }
    }
    
    if (has_done_initial_speed_up && (loop_us - last_pid_us) >= PID_EVAL_US){
        last_pid_us += PID_EVAL_US;
        
        int16_t error = get_error();
        int16_t p = -KP * error;
        int16_t i = -120; //offset to counteract wind constant error - should really include a proper integral calc
        int16_t d =  KD * (last_error - error);
        acceleration = p + i + d;
        
        last_error = error;
    }

    //DEBUGGING
    if (DEBUG && !has_splurged && has_done_initial_speed_up && (loop_us - last_mem_us) >= SAMPLE_INTERVAL_MS * 1000){
        last_mem_us += SAMPLE_INTERVAL_MS * 1000;
        if (memory_ind + 3 >= MEM_SIZE){
            for (uint16_t i = 0; i < MEM_SIZE; i+=3){
                Serial.print(memory[i]);
                Serial.print(" ");
                Serial.print(memory[i+1]);
                Serial.print(" ");
                Serial.print(memory[i+2]);
                Serial.println();
            }
            finish();
        } else {
            memory[memory_ind++] = get_error();
            memory[memory_ind++] = acceleration;
            memory[memory_ind++] = 0;
        }
    }
}

void finish(){
    digitalWrite(P1_PIN, LOW);
    digitalWrite(P2_PIN, LOW);
    digitalWrite(P3_PIN, LOW);
    digitalWrite(13, LOW);
    done = 1;
}

int16_t get_error(){
    return analogRead(LDR1_PIN) - analogRead(LDR2_PIN);
}

