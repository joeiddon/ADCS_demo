/*
 * Code finalised in July 2019.
 */


/* Pulses to the three phases overlap as follows.
 * 
 *     0      1      2   |   0      1      2      ...
 *    __________            __________
 * __|          |__________|          |_________  ...
 *           __________            __________
 * _________|          |__________|          |___ ...
 * ______            __________            _____
 *       |__________|          |__________|       ...
 * 
 * BLDC motor takes 12 steps (4 phase stages) to complete a full revolution.
 */

//TODO:
// get rid of floats
// slower speed

#define P1_PIN 2
#define P2_PIN 3
#define P3_PIN 4

#define LDR1_PIN 6
#define LDR2_PIN 7

#define STEPS_PER_REV 12

//some little macros
#define CLAMP(X) ((X) > 1 ? 1 : X)
#define ABS(X) ((X) < 0 ? -X : X)
//gives pulse interval time from rpm speed
#define PULSE_US_FROM_RPM(R) ((1000000 * 60) / ((R) * STEPS_PER_REV)) //[1000us for 5000rpm]
//gives accel interval from accel rate
#define INTERVAL_US_FROM_RATE(R) (1000000 / ABS((R)))

#define PID_EVAL_US 1000 //time between re-evaluating PIDs

//PID tuning
#define KP 2
#define KD 1

#define MAX_RPM 5000 //really 5400 but that is pushing it!

#define DEBUG 0

//fraction of pulse_length that overlaps into next phase's pulse
float overlap = 0.25;

int16_t accel_rate = 270; //revs/min/sec

uint16_t current_rpm = 60;
uint16_t start_rpm_target = 5000;
uint8_t has_done_initial_speed_up = 0;

uint32_t last_pulse_us;
uint32_t last_accel_us;
uint32_t last_pid_us;

uint32_t pulse_length_us;
uint32_t start_time_us;
uint8_t stage = 0;
//start times for each of the three phases
uint32_t p1_start_us, p2_start_us, p3_start_us;

int16_t last_error = 0;

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
    start_time_us = micros();
    last_pulse_us = start_time_us;
    last_accel_us = start_time_us;
    last_mem_us = start_time_us;
    Serial.begin(9600);
    pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
    digitalWrite(13, LOW);
}

void loop() {
    uint32_t loop_us = micros();
    if ((loop_us - last_pulse_us) >= pulse_length_us){
        last_pulse_us += pulse_length_us;
        switch (stage = (stage + 1) % 3){
            //in each case, we set pulse start for the next phase
            case 0:
                p2_start_us = last_pulse_us + (1 - overlap) * pulse_length_us;
                break;
            case 1:
                p3_start_us = last_pulse_us + (1 - overlap) * pulse_length_us;
                break;
            case 2:
                p1_start_us = last_pulse_us + (1 - overlap) * pulse_length_us;
                break;
        }
    }
    digitalWrite(P1_PIN, (p1_start_us < loop_us) && (loop_us < p1_start_us + (1 + 2 * overlap) * pulse_length_us));
    digitalWrite(P2_PIN, (p2_start_us < loop_us) && (loop_us < p2_start_us + (1 + 2 * overlap) * pulse_length_us));
    digitalWrite(P3_PIN, (p3_start_us < loop_us) && (loop_us < p3_start_us + (1 + 2 * overlap) * pulse_length_us));
    if (accel_rate && (loop_us - last_accel_us) >= INTERVAL_US_FROM_RATE(accel_rate)){
        last_accel_us += INTERVAL_US_FROM_RATE(accel_rate);
        if (has_done_initial_speed_up){
            current_rpm += accel_rate > 0 ? 1 : -1;
            if (current_rpm > MAX_RPM) current_rpm = MAX_RPM;
            pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
        } else {
            if (current_rpm == start_rpm_target){
                has_done_initial_speed_up = 1;
                overlap = -0.2;
                accel_rate = 0;
                digitalWrite(13, HIGH);
                last_pid_us = loop_us;
                last_mem_us = loop_us;
            } else {
                current_rpm += 1;
                if (current_rpm & 0xff == 0) accel_rate += 1; //accelerate some more to speed up initial accel
                pulse_length_us = PULSE_US_FROM_RPM(current_rpm);
            }
        }
    }

    if (has_done_initial_speed_up && (loop_us - last_pid_us) >= PID_EVAL_US){
        last_pid_us += PID_EVAL_US;
        int16_t error = get_error();
        int16_t p = -KP * error;
        int16_t i = -120; //offset to counteract wind constant error - should really include a proper integral calc
        int16_t d =  KD * (last_error - error);
        accel_rate = p + i + d;
        last_error = error;
    }

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
            has_splurged = 1;
        } else {
            memory[memory_ind++] = get_error();
            memory[memory_ind++] = accel_rate;
            memory[memory_ind++] = 0;
        }
    }
}

int16_t get_error(){
    return analogRead(LDR1_PIN) - analogRead(LDR2_PIN);
}
