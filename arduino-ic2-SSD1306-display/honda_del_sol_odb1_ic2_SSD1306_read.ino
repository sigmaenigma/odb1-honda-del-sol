#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin setup
const int milPin = 18;  // Pin 18 is connected to the MIL

// Honda OBD1 blink timing thresholds (milliseconds)
const unsigned long LONG_BLINK_THRESHOLD = 1000;  // blinks longer than this are "long"
const unsigned long INTER_CODE_PAUSE     = 2000;  // LOW this long = end of code group
const unsigned long BLINK_TIMEOUT        = 3000;  // max time to wait for pin to go LOW

// DTC lookup table — index == DTC code number (0–92)
const char* dtc_codes[] = {
  "Electronic control module (ECM)",    // 0
  "Heated oxygen sensor A",             // 1
  "Oxygen content B",                   // 2
  "Manifold absolute pressure",         // 3
  "Crank position sensor",              // 4
  "Manifold absolute pressure",         // 5
  "Engine coolant temperature",         // 6
  "Throttle position sensor",           // 7
  "Top dead center sensor",             // 8
  "No.1 cylinder position sensor",      // 9
  "Intake air temperature sensor",      // 10
  "Electronic control module (ECM)",    // 11
  "Exhaust recirculation system",       // 12
  "Barometric pressure sensor",         // 13
  "Idle air control valve or bad ECM",  // 14
  "Ignition output signal",             // 15
  "Fuel Injector",                      // 16
  "Vehicle speed sensor",               // 17
  "Unknown DTC",                        // 18
  "A/T lock-up control solenoid",       // 19
  "Electric load detector",             // 20
  "V-TEC control solenoid",             // 21
  "V-TEC pressure solenoid",            // 22
  "Knock sensor",                       // 23
  "Unknown DTC",                        // 24
  "Unknown DTC",                        // 25
  "Unknown DTC",                        // 26
  "Unknown DTC",                        // 27
  "Unknown DTC",                        // 28
  "Unknown DTC",                        // 29
  "A/T FI signal A",                    // 30
  "A/T FI signal B",                    // 31
  "Unknown DTC",                        // 32
  "Unknown DTC",                        // 33
  "Unknown DTC",                        // 34
  "Unknown DTC",                        // 35
  "Unknown DTC",                        // 36
  "Unknown DTC",                        // 37
  "Unknown DTC",                        // 38
  "Unknown DTC",                        // 39
  "Unknown DTC",                        // 40
  "Heated oxygen sensor heater",        // 41
  "Unknown DTC",                        // 42
  "Fuel supply system",                 // 43
  "Unknown DTC",                        // 44
  "Fuel supply metering",               // 45
  "Unknown DTC",                        // 46
  "Unknown DTC",                        // 47
  "Heated oxygen sensor",               // 48
  "Unknown DTC",                        // 49
  "Unknown DTC",                        // 50
  "Unknown DTC",                        // 51
  "Unknown DTC",                        // 52
  "Unknown DTC",                        // 53
  "Unknown DTC",                        // 54
  "Unknown DTC",                        // 55
  "Unknown DTC",                        // 56
  "Unknown DTC",                        // 57
  "Unknown DTC",                        // 58
  "Unknown DTC",                        // 59
  "Unknown DTC",                        // 60
  "Front heated oxygen sensor",         // 61
  "Unknown DTC",                        // 62
  "Rear heated oxygen sensor",          // 63
  "Unknown DTC",                        // 64
  "Rear heated oxygen sensor heater",   // 65
  "Unknown DTC",                        // 66
  "Catalytic converter system",         // 67
  "Unknown DTC",                        // 68
  "Unknown DTC",                        // 69
  "Automatic transaxle",                // 70
  "Misfire detected cylinder 1",        // 71
  "Misfire detected cylinder 2",        // 72
  "Misfire detected cylinder 3",        // 73
  "Misfire detected cylinder 4",        // 74
  "Misfire detected cylinder 5",        // 75
  "Misfire detected cylinder 6",        // 76
  "Unknown DTC",                        // 77
  "Unknown DTC",                        // 78
  "Unknown DTC",                        // 79
  "Exhaust recirculation system",       // 80
  "Unknown DTC",                        // 81
  "Unknown DTC",                        // 82
  "Unknown DTC",                        // 83
  "Unknown DTC",                        // 84
  "Unknown DTC",                        // 85
  "Coolant temperature",                // 86
  "Unknown DTC",                        // 87
  "Unknown DTC",                        // 88
  "Unknown DTC",                        // 89
  "Unknown DTC",                        // 90
  "Unknown DTC",                        // 91
  "Evaporative emission control system" // 92
};

const int DTC_TABLE_SIZE = 93;

void setup() {
  Serial.begin(9600);
  pinMode(milPin, INPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds
  display.clearDisplay();
}

/*
 * Reads one full DTC code from the MIL pin.
 *
 * Honda OBD1 protocol:
 *   - N long blinks  (~1.2 s ON) = tens digit
 *   - M short blinks (~0.5 s ON) = units digit
 *   - ~0.5 s LOW pause between blinks within a code
 *   - ~2.5 s LOW pause between codes
 *
 * Populates long_blinks and short_blinks, returns when inter-code pause detected.
 */
void readBlinks(int &long_blinks, int &short_blinks) {
  long_blinks = 0;
  short_blinks = 0;

  // Wait for first blink to begin
  while (digitalRead(milPin) == LOW) {}

  while (true) {
    if (digitalRead(milPin) == HIGH) {
      unsigned long start_time = millis();
      // Wait for pin to go LOW, with timeout to avoid locking up
      while (digitalRead(milPin) == HIGH) {
        if (millis() - start_time > BLINK_TIMEOUT) break;
      }
      unsigned long blink_duration = millis() - start_time;

      if (blink_duration > LONG_BLINK_THRESHOLD) {
        long_blinks++;
      } else {
        short_blinks++;
      }

      // Measure how long the pin stays LOW after the blink.
      // A pause longer than INTER_CODE_PAUSE means this code group is done.
      unsigned long low_start = millis();
      while (digitalRead(milPin) == LOW) {
        if (millis() - low_start > INTER_CODE_PAUSE) {
          return;
        }
      }
    }
    delay(10);
  }
}

void loop() {
  int long_blinks = 0;
  int short_blinks = 0;

  readBlinks(long_blinks, short_blinks);

  int dtc_code = long_blinks * 10 + short_blinks;
  const char* dtc_message;
  if (dtc_code >= 0 && dtc_code < DTC_TABLE_SIZE) {
    dtc_message = dtc_codes[dtc_code];
  } else {
    dtc_message = "Unknown DTC";
  }

  // Print to Serial Monitor
  Serial.print("DTC Code: ");
  Serial.print(dtc_code);
  Serial.print(", Message: ");
  Serial.println(dtc_message);

  // Print to OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("DTC Code: ");
  display.println(dtc_code);
  display.print("Message: ");
  display.println(dtc_message);
  display.display();

  delay(5000);  // Wait before reading the next code
}
