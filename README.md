# Project Title
ESP8266 Smart AC Controller

## Arduino IDE Setup
Start Arduino and open Preferences window.
Enter http:arduino.esp8266.com/stable/package_esp8266com_index.json into File -> Preferences -> Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
Install Boards -> Open Tools -> Boards Manager Board -> esp8266@3.0.2 platform
Install Libraries -> !!! WakeOnLan@1.1.7 & !!! EMailSender@3.0.13
Select Tools -> Board -> esp8266 -> Lolin (Wemos) D1 R2 & mini
Setup in Config_Secrets.h Router connection details and e-mail credentials for EMailSender
*For USB Port Driver : http:www.wch.cn/downloads/CH341SER_ZIP.html

## ESP Exception Decodder
Read from bottom to top
New:
https:github.com/dankeboy36/esp-exception-decoder?tab=readme-ov-file
C:\Users\<username>\.arduinoIDE\plugins
Ctrl + Shift + P -> Search ESP Excep...
*When tracing the root cause of the problem does not work, try Closing the Arduino IDE, Re-opening and Re-compiling everything

## Test:
TARGET_IP="192.168.1.4"
watch -n 0 "curl $TARGET_IP/; curl $TARGET_IP/alarms; curl $TARGET_IP/historical_temp; curl $TARGET_IP/history; curl $TARGET_IP/admin; curl $TARGET_IP/config; curl $TARGET_IP/sys_params; curl $TARGET_IP/api; curl $TARGET_IP/api/status; curl $TARGET_IP/api/ext_comp_status; curl $TARGET_IP/api/refresh; curl $TARGET_IP/api/alarms; curl $TARGET_IP/api/historical_temp; curl $TARGET_IP/api/refresh; curl $TARGET_IP/api/history;"
