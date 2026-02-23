# OBD1 Data Reader for 1994 Honda Del Sol

This repository contains proof-of-concept code for reading OBD1 diagnostic trouble codes (DTCs) from a 1994 Honda Del Sol. Implementations are provided for both Arduino and Raspberry Pi.

> **Note:** These scripts are not in a production state and should be used with caution.

## How OBD1 Blink Codes Work

Unlike modern OBD2 systems, OBD1 communicates faults by blinking the Malfunction Indicator Lamp (MIL) — the Check Engine Light. To read codes, you turn the ignition to the ON position (engine off) and count the blinks.

The blink pattern encodes a two-digit DTC:

- **Long blinks** (>1 second ON) = tens digit
- **Short blinks** (<1 second ON) = ones digit

**Example:** 1 long blink followed by 2 short blinks = **Code 12** (Exhaust recirculation system)

After one code is displayed, there is a pause before the next code begins. If there are no faults, the MIL will blink code 0.

## Connecting to the OBD1 Port

On a 1994 Honda Del Sol, the OBD1 diagnostic connector is typically located under the dashboard on the driver's side. The MIL signal wire carries 12V when the lamp is ON.

> **Important:** Both the Arduino and Raspberry Pi GPIO pins operate at 5V and 3.3V respectively. You must use a voltage divider or voltage regulator to step down the 12V signal before connecting to your board. See the sub-project READMEs for details.

## Implementations

| Folder | Platform | Output |
|---|---|---|
| [arduino/](arduino/) | Arduino (e.g. Uno) | Serial Monitor |
| [arduino-ic2-SSD1306-display/](arduino-ic2-SSD1306-display/) | Arduino + SSD1306 OLED | Serial Monitor + OLED display |
| [raspberry-pi/](raspberry-pi/) | Raspberry Pi (Python) | Terminal output |

## Repository Structure

```
odb1-honda-del-sol/
├── arduino/                        # Basic Arduino sketch (serial output)
├── arduino-ic2-SSD1306-display/    # Arduino sketch with I2C OLED display
└── raspberry-pi/                   # Python script for Raspberry Pi
```

## Disclaimer

These scripts are proof-of-concept and not intended for production use. Always ensure you understand the code and its implications before connecting anything to your vehicle's electrical system.
