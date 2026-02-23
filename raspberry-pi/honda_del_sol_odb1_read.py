import RPi.GPIO as GPIO
import time
import json

""" This is a proof of concept and definitely not in a 'prod' state.
Just saving stuff here in case someone finds this and decides to start using it.
BE CAREFUL!
"""

# GPIO setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)  # GPIO 18 is connected to the MIL

# Honda OBD1 blink timing thresholds
LONG_BLINK_THRESHOLD = 1.0   # seconds — blinks longer than this are "long"
INTER_CODE_PAUSE     = 2.0   # seconds — LOW this long signals end of a code group
BLINK_TIMEOUT        = 3.0   # seconds — max time to wait for pin to go LOW
POLL_INTERVAL        = 0.01  # seconds — polling interval

def read_blinks():
    """
    Read one full DTC code from the MIL pin.

    Honda OBD1 encodes each code as:
      - N long blinks  (~1.2 s ON) = tens digit
      - M short blinks (~0.5 s ON) = units digit
      - ~0.5 s pause between blinks within a code
      - ~2.5 s pause between codes

    Returns (long_blinks, short_blinks) after the inter-code pause is detected.
    """
    long_blinks = 0
    short_blinks = 0

    # Wait for the first blink to begin
    while GPIO.input(18) == GPIO.LOW:
        time.sleep(POLL_INTERVAL)

    while True:
        if GPIO.input(18) == GPIO.HIGH:
            start_time = time.time()
            # Wait for pin to go LOW, with timeout to avoid locking up
            while GPIO.input(18) == GPIO.HIGH:
                if time.time() - start_time > BLINK_TIMEOUT:
                    break
            blink_duration = time.time() - start_time

            if blink_duration > LONG_BLINK_THRESHOLD:
                long_blinks += 1
            else:
                short_blinks += 1

            # Measure how long the pin stays LOW after the blink.
            # A pause longer than INTER_CODE_PAUSE means this code group is done.
            low_start = time.time()
            while GPIO.input(18) == GPIO.LOW:
                if time.time() - low_start > INTER_CODE_PAUSE:
                    return long_blinks, short_blinks
                time.sleep(POLL_INTERVAL)
        else:
            time.sleep(POLL_INTERVAL)

try:
    with open('honda_del_sol_codes.json', 'r') as f:
        dtc_codes = json.load(f)

    while True:
        long_blinks, short_blinks = read_blinks()
        dtc_code = long_blinks * 10 + short_blinks
        dtc_message = dtc_codes.get(str(dtc_code), "Unknown DTC")
        print(f"DTC Code: {dtc_code}, Message: {dtc_message}")
        time.sleep(5)  # Wait before reading the next code

except KeyboardInterrupt:
    pass
except Exception as e:
    print(f"Error: {e}")
finally:
    GPIO.cleanup()
