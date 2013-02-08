# This script generates the lookup table for the standby animation
# from a simple 128xn image. It will break if an image with a different
# width is used. It is also important that the maximum RGB value of the
# used image is 31.

import Image
img = Image.open("default_small.jpeg")

def main():

  red = "static const uint8_t pgmRedGradient[] PROGMEM = {"
  for x in range(0,128):
	red = red + str(img.getpixel((x,0))[0]) + ","
  red = red + "};"
  print red
  green = "static const uint8_t pgmGreenGradient[] PROGMEM = {"
  for x in range(0,128):
	green = green + str(img.getpixel((x,0))[1]) + ","
  green = green + "};"
  print green
  blue = "static const uint8_t pgmBlueGradient[] PROGMEM = {"
  for x in range(0,128):
	blue = blue + str(img.getpixel((x,0))[2]) + ","
  blue = blue + "};"
  print blue
 
if __name__ == '__main__': main()
