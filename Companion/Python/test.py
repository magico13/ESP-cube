import companion
import time
import random

cube = companion.Cube('http://esp-cube.local')
print(cube.get_color())
red = random.randint(0, 64)
green = random.randint(0, 64)
blue = random.randint(0, 64)
cube.set_color(red, green, blue)
print(cube.get_color())

time.sleep(1)

blink = companion.Blink(4)
cube.animate(blink)

time.sleep(1)

breathe = companion.Breathe(2)
cube.animate(breathe)

time.sleep(1)

cube.set_tap(blink)