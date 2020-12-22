Python sample script for NORA mini LED Badge with Raspberry Pi.

Before to use it, enable I2C setting on Raspberry Pi.

To install and run:

```bash
$ pip3 install adafruit-circuitpython-ht16k33
$ git clone https://github.com/kitazaki/nora_badge_3rd
$ cd nora_badge_3rd/Python
$ python3 ht16k33_matrix_simpletest_t1.py
```

This script is based on sample script of Adafruit_CircuitPython_HT16K33 python library.

```bash
$ git clone https://github.com/adafruit/Adafruit_CircuitPython_HT16K33
$ cd Adafruit_CircuitPython_HT16K33/example
```

NORA mini LED Badge  
![M5Atom lite](https://github.com/kitazaki/nora_badge_3rd/raw/master/UIFlow/NORA_LED_Badge.png)

NORA mini LED Badge attached on Raspberry Pi 3 Model B+
![Raspberry Pi](https://github.com/kitazaki/nora_badge_3rd/raw/master/Python/RPi_MINI_LED_BADGE.png)

<A HREF="https://youtu.be/eIRYAXf6tqQ">Movie</A>  

I2C Address (0x70 and 0x71), Arduino IDE library (Adafruit-LED-Backpack)
![I2C Address](https://github.com/kitazaki/nora_badge_3rd/raw/master/UIFlow/I2C_address.png)

==========
(in Japanese)
野良ミニLEDバッジをラズパイで動かすPythonサンプルスクリプトです。

