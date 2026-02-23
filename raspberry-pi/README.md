# OBD1 Data Reader for 1994 Honda Del Sol (Raspberry Pi)

This repository contains a proof-of-concept Python script designed to read OBD1 data from a 1994 Honda Del Sol using a Raspberry Pi. This script is not in a production state and should be used with caution.

## Description

The script reads input from a specified GPIO pin on the Raspberry Pi, counts the number of blinks, and matches the count to a corresponding OBD1 code from a JSON file. The JSON file contains diagnostic trouble codes (DTCs) specific to the 1994 Honda Del Sol.

## Hardware Requirements

- Raspberry Pi
- GPIO setup for reading OBD1 data
- 1994 Honda Del Sol with a B16A3 Engine
- **Voltage regulator** to step down 12V to 3.3V

## Software Requirements

- Python 3.x
- RPi.GPIO library
- JSON file with OBD1 codes

## Installation

1. Clone this repository:
    ```bash
    git clone https://github.com/sigmaenigma/odb1-honda-del-sol.git
    cd .\odb1-honda-del-sol\raspberry-pi\
    ```

2. Install the required Python libraries:
    ```bash
    pip install RPi.GPIO
    ```

3. Ensure the `honda_del_sol_codes.json` file is in the same directory as the script.

## Voltage Regulation

To protect your Raspberry Pi from the 12V output of the Honda Del Sol, you need to use a voltage regulator to step down the voltage to 3.3V. Here’s how you can do it:

1. **Components Needed**: (I'll try and create a schematic soon)
    - Voltage regulator (e.g., LM7805 for 5V output, then a resistor divider or a 3.3V regulator like LD1117-3.3)
    - Resistors (if using a resistor divider)
    - Breadboard and jumper wires

3. **Wiring**:
    - Connect the 12V output from the Honda Del Sol to the input of the voltage regulator.
    - Connect the ground of the voltage regulator to the ground of the Raspberry Pi.
    - Connect the output of the voltage regulator to the GPIO pin (pin 18) on the Raspberry Pi.

4. **Example Circuit**:
    - If using an LM7805:
        - Input: 12V from the car
        - Output: 5V
        - Use a resistor divider to step down from 5V to 3.3V, or use a 3.3V regulator like LD1117-3.3 directly.

## Usage

1. Connect the Raspberry Pi to the OBD1 port of the 1994 Honda Del Sol using the voltage regulator setup.
2. Run the script:
    ```bash
    python obd1_reader.py
    ```

## Code

```python
import RPi.GPIO as GPIO
import time
import json

""" This is a proof of concept and definitely not in a 'prod' state. 
Just saving stuff here in case someone finds this and decides to start using it. 
BE CAREFUL! 
"""

# GPIO setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)  # Assuming GPIO 18 is connected to the MIL

with open('honda_del_sol_codes.json','r') as f:
    dtc_codes = json.load(f)
    
def read_blinks():
    long_blinks = 0
    short_blinks = 0
    while True:
        if GPIO.input(18) == GPIO.HIGH:
            start_time = time.time()
            while GPIO.input(18) == GPIO.HIGH:
                pass
            blink_duration = time.time() - start_time
            if blink_duration > 1:  # Assuming long blink is > 1 second
                long_blinks += 1
            else:
                short_blinks += 1
        time.sleep(0.1)  # Debounce delay
        if long_blinks > 0 and short_blinks > 0:
            break
    return long_blinks, short_blinks

try:
    while True:
        long_blinks, short_blinks = read_blinks()
        dtc_code = long_blinks * 10 + short_blinks
        dtc_message = dtc_codes.get(str(dtc_code), "Unknown DTC")
        print(f"DTC Code: {dtc_code}, Message: {dtc_message}")
        time.sleep(5)  # Wait before reading the next code
except KeyboardInterrupt:
    GPIO.cleanup()
```

## JSON File Format

The `honda_del_sol_codes.json` file should be formatted as follows:

```json
{
  "0": "Electronic control module (ECM)",
  "1": "Heated oxygen sensor A",
  "2": "Oxygen content B",
  "3": "Manifold absolute pressure",
  "4": "Crank position sensor",
  "5": "Manifold absolute pressure",
  "6": "Engine coolant temperature",
  "7": "Throttle position sensor",
  "8": "Top dead center sensor",
  "9": "No.1 cylinder position sensor",
  "10": "Intake air temperature sensor",
  "11": "Electronic control module (ECM)",
  "12": "Exhaust recirculation system",
  "13": "Barometric pressure sensor",
  "14": "Idle air control valve or bad ECM",
  "15": "Ignition output signal",
  "16": "Fuel Injector",
  "17": "Vehicle speed sensor",
  "19": "A/T lock-up control solenoid",
  "20": "Electric load detector",
  "21": "V-TEC control solenoid",
  "22": "V-TEC pressure solenoid",
  "23": "Knock sensor",
  "30": "A/T FI signal A",
  "31": "A/T FI signal B",
  "41": "Heated oxygen sensor heater",
  "43": "Fuel supply system",
  "45": "Fuel supply metering",
  "48": "Heated oxygen sensor",
  "61": "Front heated oxygen sensor",
  "63": "Rear heated oxygen sensor",
  "65": "Rear heated oxygen sensor heater",
  "67": "Catalytic converter system",
  "70": "Automatic transaxle",
  "71": "Misfire detected cylinder 1",
  "72": "Misfire detected cylinder 2",
  "73": "Misfire detected cylinder 3",
  "74": "Misfire detected cylinder 4",
  "75": "Misfire detected cylinder 5",
  "76": "Misfire detected cylinder 6",
  "80": "Exhaust recirculation system",
  "86": "Coolant temperature",
  "92": "Evaporative emission control system"
}
```

## Disclaimer

This script is a proof of concept and should be used with caution. It is not intended for production use. Always ensure you understand the code and its implications before using it on your vehicle.
