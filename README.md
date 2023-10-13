# Sunny - IOT Sunrise Alarm Clock

## Materials:
1. WS2815 12V RGB LED Strip (1m - 144led/m)
2. 12V 5A DC Power Supply
3. Female DC Power plug (5.5 x 2.5mm)
4. ESP 8266 Wemos D1 Mini Dev Board
5. Acrylic Panel
6. PLA filament
7. Super glue
8. 3D Printer
### Apps:
1. Sleep as Android
2. Tasker
#### Sleep as Android Tasker integration:
1. In Sleep as Android, Enable tasker automation with the following:
      Settings > Services > Automation > Select Tasker
2. Set adequite delay and HTTP Request to http://**incert IP here**/sunrise on Sleep Smart period event| For sunrise alarm
3. Set adequite delay and HTTP Request to http://**incert IP here**/off on Sleep Alarm dismiss event| For sunrise off after dismiss
