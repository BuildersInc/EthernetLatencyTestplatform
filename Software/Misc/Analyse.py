from pathlib import Path
from datetime import datetime

import pandas as pd

SCRIPT_PATH = Path(__file__).parent

# FILE = SCRIPT_PATH / "MessungsDaten" / "MessungenUDP" / "digital.csv"
FILE = SCRIPT_PATH / "MessungsDaten" / "MessungenHTTP" / "digital.csv"

data_frame = pd.read_csv(FILE, sep=",", decimal=".", encoding="utf-8")
print(data_frame)
times = []
master = slave = False
time_master = time_slave = None


for index, row in data_frame.iterrows():
    if row["Master"] == 1:
        master = True
        time_master = datetime.fromisoformat(row["Time [s]"])
    if row["Slave"] == 1:
        slave = True
        time_slave = datetime.fromisoformat(row["Time [s]"])

    if master and slave:
        master = slave = False
        times.append(time_slave - time_master)
        time_master = time_slave = None


average_time = sum(times, datetime.min - datetime.min) / len(times)
print(f"Average time difference: {average_time.microseconds}")
print(f"Max time difference: {max(times).microseconds}")
print(f"Min time difference: {min(times).microseconds}")
print(f"Number of measurements: {len(times)}")


print(f"Average time difference: {average_time}")
print(f"Max time difference: {max(times)}")
print(f"Min time difference: {min(times)}")
print(f"Number of measurements: {len(times)}")

