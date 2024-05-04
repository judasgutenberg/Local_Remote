# Local_Remote
This is a local remote client for the ESP8266-based remote control. I've written it to use another ESP8266, though it could be made to work on any internet-capable Arduino board. It uses the two endpoints /readLocalData and /writeLocalData on a ESP8266 programmed with the Remote Controller software (https://github.com/judasgutenberg/Esp8266_RemoteControl) to locally change the on/off state of device_features. Such changes are sent back to the server.

![alt text](localremote.jpg?raw=true)


This local remote uses a 20 by 4 line LCD display (with a YwRobot I2C backpack) and four pushbuttons as a user interface.  The specific information about what is controlled is retrieved via JSON, and this can be scrolled through by a user to turn devices on and off.

The four buttons are mode, up, down, and switch.  Mode allows you to switch the screen from checking on your energy data (from your solar panel and inverter, though for now all I support is SolArk inverters) to monitoring the weather data of some other ESP8266 on the local network to controlling the device_features on a remote controller (possibly a different one from the source of weather data).  The up/down buttons are for moving up and down the scrolling display and the switch allows you to turn an on device off or an off device on.
