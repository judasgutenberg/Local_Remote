# Local_Remote
This is a local remote client for the ESP8266-based remote control. I've written it to use another ESP8266, though it could be made to work on any internet-capable Arduino board. It uses the two endpoints /readLocalData and /writeLocalData on a ESP8266 programmed with the Remote Controller software (https://github.com/judasgutenberg/Esp8266_RemoteControl) to locally change the on/off state of device_features. Such changes are sent back to the server.

This local remote uses a 20 by 4 line LCD display (with a YwRobot I2C backpack) and four pushbuttons as a user interface.  The specific information about what is controlled is retrieved via JSON, and this can be scrolled through by a user to turn devices on and off.
