# Renesas ZMOD4510 Sensor Firmware for Raspberry Pi5

This is a modified version of the firmware provided by Renesas for the ZMOD4510 O3 and NO2 sensor.
The following changes have been made:
- the dependency on PiGPIO library is removed, instead the Linux kernel's native I2C interface is used,
- the code related to temperature and relative humidity sensor is removed,
- the algorithm libraries are provided only for Raspberry Pi,
- cmake is used for compilation management

# Compile the Sample Application

From the sources root folder:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the generated binary with:

```bash
build/no2_o3-example
```

# Compile and Install the Python Module

* Optionally, create and activate a Python virtual environment

```bash
python3 -m venv .venv-test
source ./.venv-test/bin/activate
```

* Compile and install from the sources root folder

```bash
pip install .
```

* Verify that the Python module is installed

```bash
python3 -c "import zmod4510; print(zmod4510.__file__); s = zmod4510.ZMOD4510(); print(dir(s))"
```

# My commands for OrangePI 3
```bash
comet@orangepizero3:~/renesas_zmod4510/lib$ ls
lib_no2_o3.a  lib_zmod4xxx_cleaning.a
(.venv-test) comet@orangepizero3:~/renesas_zmod4510/lib$ strings lib_no2_o3.a | grep -E "GCC:|clang|GNU"
GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109
.note.GNU-stack
GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109
.note.GNU-stack
GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109
.note.GNU-stack
GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109
.note.GNU-stack


echo "deb http://archive.debian.org/debian stretch main" | sudo tee /etc/apt/sources.list.d/stretch.list
sudo apt update -o Acquire::Check-Valid-Until=false
sudo apt install -y gcc-6 g++-6
 
cd renesas_zmod4510/
sudo apt upgrade
sudo apt install -y git build-essential cmake
mkdir build && cd build
cmake -S .. -B .   -DCMAKE_C_COMPILER=gcc-6   -DCMAKE_BUILD_TYPE=Release
make -j4

python3 -m venv .venv-test
source ./.venv-test/bin/activate
pip install .
python3 -c "import zmod4510; print(zmod4510.__file__); s = zmod4510.ZMOD4510(); print(dir(s))"

(.venv-test) comet@orangepizero3:~/renesas_zmod4510/lib$ pip list
Package    Version
---------- -------
pip        23.0.1
setuptools 66.1.1
zmod4510   0.1.0
```
