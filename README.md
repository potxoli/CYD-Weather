# CYD-Weather
A simple weather widget for a CYD ESP32 Display

The link to my CYD from Amazon is here: https://www.amazon.com/dp/B0CLR7MQ91?th=1

I used the code from Volos R as the basis of this display. You can find the original code here:

https://github.com/VolosR/tDisplayS3WeatherStation

And a video about the original code here:

https://www.youtube.com/watch?v=VntDY9Mg7T0

I'm sure there is a more appropriate way to give him credit on GitHub, but this is my first project so I just copied his code over and made changes to it for the formating to work on a CYD and I also fixed a bug with histogram display.

I used VScode with PlatformIO extension to compile and upload this and I'm checking in all the files needed for Platform IO to do the same. 

You will need to create a User_Setup.h file in the libdeps/TFT_eSPI.  You can copy User_SetupCYD.h from the src directory and that should work with a CYD.

If you need general help getting a CYD to work, Random Nerd Tutorials are very good.

https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/

