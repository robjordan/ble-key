# ble-key
Mediation device between Lekato Wireless Page Turner and Tablets that lack the hid-multitouch input device.

The tablet connects with the ESP32 device, which acts as a HID keyboard. ESP32 and the Lekato pedal connect via a specialised USB cable (non-standard USB). When a pedal switch is pressed, the ESP32 sends a keystroke to the table. ESP32 tracks the landscape / portrait mode on the Lekato. 

| |Left button|Right button|
|-|-----------|------------|
|Portrait mode|Left arrow|Right arrow|
|Landscape mode|Up arrow|Down arrow|
