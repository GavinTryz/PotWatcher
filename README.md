# PotWatcher
__"A watched pot never boils."__
My stove is incredibly slow, so I built this program so that I can tell my Google Home to "watch my pot for me". From there, an ESP32 will begin to monitor humidity around my stove, and notify me via my Google Home when my water is done boiling.

Code by Gavin Tryzbiak

https://github.com/GavinTryz

Note that this requires some setup outside of the ESP32, involving IFTTT, AdafruitIO (or some MQTT broker), and a Google Home (optional). It's not terribly involved but likely not worth the effort to make a tutorial, but I'd be happy to explain if someone was interested.
