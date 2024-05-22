# Local_Remote
This is a local remote client for the ESP8266-based remote control. It's basically doing what management from a web page would do, but it runs on much less powerful hardware.  It also doesn't have to worry about authorization because the assumption is that if it has access to your WiFi network, you're giving it permission to manage the remote-control device on the local network specified by locationId (which refers to the device_id of the remote controller). I've written it to run on an ESP8266, though it could be made to work on any internet-capable Arduino board. It uses the two endpoints /readLocalData and /writeLocalData on a ESP8266 programmed with the Remote Controller software (https://github.com/judasgutenberg/Esp8266_RemoteControl) to locally change the on/off state of device_features. Such changes are sent back to the server.

![alt text](localremote.jpg?raw=true)


This local remote uses a NodeMCU controller board (with ESP8266), a 20 by 4 line LCD display (with a YwRobot I2C backpack, with I2C address at decimal 27) and four pushbuttons as a user interface.  
![alt text](local_remote_schematic.jpg?raw=true)

The specific information about what is controlled is retrieved via JSON, and this can be scrolled through by a user to turn devices on and off.

The four buttons are mode, up, down, and change (aka switch).  Mode allows you to switch the screen from checking on your energy data (from your solar panel and inverter, though for now all I support is SolArk inverters) to monitoring the weather data of some other ESP8266 on the local network to controlling the device_features on a remote controller (possibly a different one from the source of weather data).  The up/down buttons are for moving up and down the scrolling display and the switch allows you to turn an on device off or an off device on.
