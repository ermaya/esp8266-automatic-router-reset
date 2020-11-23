router reset for: esp 01 relay board

connect Led between Gpio2 and + 3.3v.

LED behavior changed.
Ledflashing 0.5 seconds: wifiConfig
Led on: alarm (no connection)
Led wink every 15 seconds: normal operation "Ping Ok"

to access WiFiConfig connect Gpio0 to ground for at least 3 seconds

added reset delay selection in wificonfig (5, 10 or 15 minutes)

