# PotWatcher
Just a personal program built for an ESP32 placed above a stove. A watched pot never boils, so this program is built so that I can tell my Google Home to "watch my pot for me" and the ESP32 will start monitoring the humidity above my stove. When a certain threshold is reached, my Google Home will speak up, letting me know my pot of water is now boiling.

Code by Gavin Tryzbiak

https://github.com/GavinTryz

Note that this requires some setup outside of the ESP32, involving IFTTT, AdafruitIO (or some MQTT broker), and a Google Home (optional). It's not terribly involved but likely not worth the effort to make a tutorial, but I'd be happy to explain if someone was interested.
