#!/usr/bin/env python3

import sys
import logging
from zmod4510 import zmod4510, ZMODStatus

# Vendor sample: concentrations are not valid until 50 measurement cycles
# have completed. Each get_data() call already waits one sample inside the
# library (about 6 s). Do not add another delay here.
WARMUP_CYCLES = 50
# OK samples taken after warm-up. The script prints each one, then their mean.
VALID_SAMPLES = 10

print(sys.version)
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(name)-12s %(levelname)-8s %(message)s",
    datefmt="%m-%d %H:%M:%S",
    filename="zmod4510.log",
    filemode="w",
)

# Bus comes from ~/.comet/config.yaml key i2c.smb inside ZMOD4510.
# Do not pass busnum; that argument is discarded when the file loads.
sensor = zmod4510.ZMOD4510(log_level=logging.INFO)
sensor.logger = logging.getLogger("zmod4510.zmod4510")


def report(message):
    sensor.logger.info(message)
    print(message)


def fail(message):
    sensor.logger.error(message)
    print(message, file=sys.stderr)


exit_code = 1
try:
    report("Start logging ...")
    report(f"I2C bus {sensor.busnum} from ~/.comet/config.yaml, sensor address 0x33")
    if not sensor.start():
        raise RuntimeError(f"Failed to start sensor on bus {sensor.busnum}.")
    report(
        f"Sensor started. Warm-up is {WARMUP_CYCLES} samples, "
        f"then {VALID_SAMPLES} readings and their average."
    )

    samples = []
    for cycle in range(1, WARMUP_CYCLES + 1):
        data = sensor.get_data()
        if data.status == ZMODStatus.DAMAGE:
            fail(f"Damaged during warm-up {cycle}/{WARMUP_CYCLES}.")
            break
        if data.status not in (ZMODStatus.STABILIZATION, ZMODStatus.OK):
            fail(f"Unknown status {data.status} during warm-up {cycle}/{WARMUP_CYCLES}.")
            break
        # An early OK is still inside the 50-cycle window. Do not keep it.
        report(f"Warm-up {cycle}/{WARMUP_CYCLES}")
    else:
        for index in range(1, VALID_SAMPLES + 1):
            data = sensor.get_data()
            if data.status != ZMODStatus.OK:
                name = data.status
                try:
                    name = ZMODStatus(data.status).name
                except ValueError:
                    pass
                fail(f"Sample {index}/{VALID_SAMPLES} is {name}, not OK.")
                break
            samples.append(data)
            report(
                f"{index}/{VALID_SAMPLES}  O3: {data.o3_ppb:.2f} ppb | "
                f"NO2: {data.no2_ppb:.2f} ppb | "
                f"Fast AQI: {data.fast_aqi} | EPA AQI: {data.epa_aqi}"
            )
        if len(samples) == VALID_SAMPLES:
            n = float(len(samples))
            report(
                "Average  "
                f"O3: {sum(s.o3_ppb for s in samples) / n:.2f} ppb | "
                f"NO2: {sum(s.no2_ppb for s in samples) / n:.2f} ppb | "
                f"Fast AQI: {sum(s.fast_aqi for s in samples) / n:.2f} | "
                f"EPA AQI: {sum(s.epa_aqi for s in samples) / n:.2f}"
            )
            exit_code = 0

except KeyboardInterrupt:
    fail("Stopped.")
except RuntimeError as exc:
    fail(str(exc))
finally:
    sensor.stop()

sys.exit(exit_code)
