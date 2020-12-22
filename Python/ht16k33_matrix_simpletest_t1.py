# Basic example of clearing and drawing a pixel on a LED matrix display.
# This example and library is meant to work with Adafruit CircuitPython API.
# Author: Tony DiCola
# License: Public Domain

# Import all board pins.
import time
import board
import busio

# Import the HT16K33 LED matrix module.
from adafruit_ht16k33 import matrix


# Create the I2C interface.
i2c1 = busio.I2C(board.SCL, board.SDA)
i2c2 = busio.I2C(board.SCL, board.SDA)

# Create the matrix class.
# This creates a 16x8 matrix:
#matrix = matrix.Matrix16x8(i2c)
# Or this creates a 16x8 matrix backpack:
matrix1 = matrix.MatrixBackpack16x8(i2c1, address=0x70)
matrix2 = matrix.MatrixBackpack16x8(i2c2, address=0x71)
# Or this creates a 8x8 matrix:
# matrix = matrix.Matrix8x8(i2c)
# Or this creates a 8x8 bicolor matrix:
# matrix = matrix.Matrix8x8x2(i2c)
# Finally you can optionally specify a custom I2C address of the HT16k33 like:
# matrix = matrix.Matrix16x8(i2c, address=0x70)

# Clear the matrix.
matrix1.fill(0)
matrix2.fill(0)

# Setting the Brightness
matrix1.brightness = 0.1
matrix2.brightness = 0.1

# Set a pixel in the origin 0, 0 position.
matrix1[0, 0] = 1
matrix2[0, 0] = 1
# Set a pixel in the middle 8, 4 position.
matrix1[8, 4] = 1
matrix2[8, 4] = 1
# Set a pixel in the opposite 15, 7 position.
matrix1[15, 7] = 1
matrix2[15, 7] = 1

time.sleep(2)

# Draw a Smiley Face
matrix1.fill(0)
matrix2.fill(0)

for row in range(2, 6):
    matrix1[row, 0] = 1
    matrix2[row, 0] = 1
    matrix1[row, 7] = 1
    matrix2[row, 7] = 1

for column in range(2, 6):
    matrix1[0, column] = 1
    matrix2[0, column] = 1
    matrix1[7, column] = 1
    matrix2[7, column] = 1

matrix1[1, 1] = 1
matrix2[1, 1] = 1
matrix1[1, 6] = 1
matrix2[1, 6] = 1
matrix1[6, 1] = 1
matrix2[6, 1] = 1
matrix1[6, 6] = 1
matrix2[6, 6] = 1
matrix1[2, 5] = 1
matrix2[2, 5] = 1
matrix1[5, 5] = 1
matrix2[5, 5] = 1
matrix1[2, 3] = 1
matrix2[2, 3] = 1
matrix1[5, 3] = 1
matrix2[5, 3] = 1
matrix1[3, 2] = 1
matrix2[3, 2] = 1
matrix1[4, 2] = 1
matrix2[4, 2] = 1

# Move the Smiley Face Around
while True:
    for frame in range(0, 8):
        matrix1.shift_right(True)
        matrix2.shift_right(True)
        time.sleep(0.05)
    for frame in range(0, 8):
        matrix1.shift_down(True)
        matrix2.shift_down(True)
        time.sleep(0.05)
    for frame in range(0, 8):
        matrix1.shift_left(True)
        matrix2.shift_left(True)
        time.sleep(0.05)
    for frame in range(0, 8):
        matrix1.shift_up(True)
        matrix2.shift_up(True)
        time.sleep(0.05)
