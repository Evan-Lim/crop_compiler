// ============================================================
// simulate.c – Simulation harness for CROP programs
// ============================================================
// Compile with:
//   gcc -o sim hello.c simulate.c -lm
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ============================================================
// MOCK SENSOR STATE
// ============================================================

float mock_moisture = 50.0;
float mock_temp = 25.0;
bool mock_button = false;
bool mock_flow = false;
bool mock_reset_button = false;

// ============================================================
// OVERRIDE SENSOR READ FUNCTIONS
// ============================================================

float sensor_moisture_read_opt(void) {
    return mock_moisture;
}

float sensor_temp_read_opt(void) {
    return mock_temp;
}

bool sensor_button_read_opt(void) {
    return mock_button;
}

bool sensor_flow_meter_read_opt(void) {
    return mock_flow;
}

bool sensor_reset_button_read_opt(void) {
    return mock_reset_button;
}

// ============================================================
// OVERRIDE DIGITALWRITE TO CAPTURE OUTPUTS
// ============================================================

// The generated code's digitalWrite is weak, so this strong definition will override.
static bool last_led = false;
static bool last_water_pump = false;
static bool last_heater = false;
static bool last_status_led = false;
static bool last_valve = false;

void digitalWrite(int pin, int val) {
    bool state = (val == 1);
    char* pin_name = "unknown";

    // Map pin numbers to names (matching the generated code's #defines)
    switch (pin) {
        case 4:  pin_name = "D4 (led)"; break;
        case 2:  pin_name = "D2 (water_pump)"; break;
        case 3:  pin_name = "D3 (heater)"; break;
        case 14: pin_name = "A0"; break;
        case 15: pin_name = "A1"; break;
        case 16: pin_name = "A2"; break;
        case 17: pin_name = "A3"; break;
        case 18: pin_name = "A4"; break;
        case 19: pin_name = "A5"; break;
        default: break;
    }

    if (pin == 4 && state != last_led) {
        printf("[OUTPUT] led -> %s\n", state ? "ON" : "OFF");
        last_led = state;
    }
    if (pin == 2 && state != last_water_pump) {
        printf("[OUTPUT] water_pump -> %s\n", state ? "ON" : "OFF");
        last_water_pump = state;
    }
    if (pin == 3 && state != last_heater) {
        printf("[OUTPUT] heater -> %s\n", state ? "ON" : "OFF");
        last_heater = state;
    }
    if (pin == 5 && state != last_valve) {
        printf("[OUTPUT] valve -> %s\n", state ? "ON" : "OFF");
        last_valve = state;
    }
}

// ============================================================
// OVERRIDE ANALOGREAD (if needed)
// ============================================================

int analogRead(int pin) {
    return 512;  // midpoint
}

// ============================================================
// OVERRIDE MILLIS TO SIMULATE TIME
// ============================================================

static unsigned long sim_millis = 0;

unsigned long millis() {
    return sim_millis;
}

// ============================================================
// FUNCTIONS TO CHANGE MOCK VALUES FROM OUTSIDE
// ============================================================

void set_moisture(float v) { mock_moisture = v; }
void set_temp(float v)     { mock_temp = v; }
void set_button(bool v)    { mock_button = v; }
void set_flow(bool v)      { mock_flow = v; }
void set_reset(bool v)     { mock_reset_button = v; }
void advance_time(unsigned long ms) { sim_millis += ms; }

// ============================================================
// MAIN INTERACTIVE LOOP (OVERRIDES WEAK main)
// ============================================================

// The generated main is weak, so this strong one will be used.
int main() {
    // Call the weak setup (which calls init_crop)
    extern void setup() __attribute__((weak));
    setup();

    printf("============================================================\n");
    printf("  CROP Simulation Environment\n");
    printf("============================================================\n");
    printf("Commands:\n");
    printf("  moisture <float>  - set moisture level (0-100)\n");
    printf("  temp <float>      - set temperature\n");
    printf("  button <0|1>      - set button state\n");
    printf("  flow <0|1>        - set flow trigger\n");
    printf("  reset <0|1>       - set reset button\n");
    printf("  s / step          - advance time by 1 second\n");
    printf("  q / quit          - exit\n");
    printf("\n");

    char line[256];
    while (1) {
        // Call the weak loop (which processes rules, every, etc.)
        extern void loop() __attribute__((weak));
        loop();

        // Advance time by 10ms per iteration (adjustable)
        advance_time(10);

        // Check for user input
        printf("> ");
        if (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\n")] = '\0';
            if (strlen(line) == 0) continue;

            char cmd[32];
            float val;
            int ival;

            if (sscanf(line, "%31s %f", cmd, &val) == 2) {
                if (strcmp(cmd, "moisture") == 0) { set_moisture(val); printf("Moisture set to %.1f\n", val); }
                else if (strcmp(cmd, "temp") == 0) { set_temp(val); printf("Temperature set to %.1f\n", val); }
                else if (strcmp(cmd, "button") == 0) { set_button((val != 0)); printf("Button set to %s\n", (val != 0) ? "ON" : "OFF"); }
                else if (strcmp(cmd, "flow") == 0) { set_flow((val != 0)); printf("Flow set to %s\n", (val != 0) ? "ON" : "OFF"); }
                else if (strcmp(cmd, "reset") == 0) { set_reset((val != 0)); printf("Reset button set to %s\n", (val != 0) ? "ON" : "OFF"); }
                else { printf("Unknown command: %s\n", cmd); }
            } else if (strcmp(line, "s") == 0 || strcmp(line, "step") == 0) {
                advance_time(1000);
                printf("Time advanced by 1 second (now %lu ms)\n", sim_millis);
            } else if (strcmp(line, "q") == 0 || strcmp(line, "quit") == 0) {
                printf("Exiting simulation.\n");
                break;
            } else {
                printf("Unknown command. Type 'q' to quit.\n");
            }
        }
    }
    return 0;
}
