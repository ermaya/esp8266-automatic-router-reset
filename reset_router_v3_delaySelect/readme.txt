router reset for: wemos relay board

The Wemos will connect to Wi-Fi to ping www.google.com (editable in wificonfig) every 30 seconds, if any Ping fails the LED will light up, if the situation persists during the time set in wificonfig it activates the relay for 5 seconds.

Ledflashing 0.5 seconds: wifiConfig
Led on: alarm (no connection)
Led flash every 30 seconds: normal operation

to access wifi config connect Gpio0 (D3) to ground for at least 3 seconds

added reset delay selection in wificonfig (5.10 or 15 minutes)

Always erase flash with flashtool before programming.
See screenshot for programming parameters.