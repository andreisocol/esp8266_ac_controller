static const char System_Components[] PROGMEM =
  "\r\n==System Components (HW)==\r\n"
  "Wemos D1 R2 ESP 8266 Board\r\n"
  "5V Micro-USB Power for ESP 8266 Board\r\n"
  "2 Relay Module SRD-05VDC-SL-C\r\n"
  "Power Module HW-DY02 DC-DC 3.3V/5V/12V for Relay Board\r\n"
  "Temperature Sensor LM35DZ\r\n"
  "10k ohm Pull-up Resistor for Confirmation Relay\r\n"
  "7.5V Power for HW-DY02\r\n\r\n";

static const char System_Components_Web[] PROGMEM =
  "\r\n==System Components (HW)==<br>\r\n"
  "Wemos D1 R2 ESP 8266 Board<br>\r\n"
  "5V Micro-USB Power for ESP 8266 Board<br>\r\n"
  "2 Relay Module SRD-05VDC-SL-C<br>\r\n"
  "Power Module HW-DY02 DC-DC 3.3V/5V/12V for Relay Board<br>\r\n"
  "Temperature Sensor LM35DZ<br>\r\n"
  "10k ohm Pull-up Resistor for Confirmation Relay<br>\r\n"
  "7.5V Power for HW-DY02<br>\r\n<br>\r\n";

static const char External_Components[] PROGMEM =
  "==External Components (SW)==\r\n"
  "MongoDB 3.4, HTTP FW Filestore,\r\n"
  "SMTP Server, NTP Server, DNS Server\r\n\r\n";

static const char External_Components_Web[] PROGMEM =
  "==External Components (SW)==<br>\r\n"
  "MongoDB 3.4, HTTP FW Filestore,<br>\r\n"
  "SMTP Server, NTP Server, DNS Server<br>\r\n";

static const char Interfaces[] PROGMEM =
  "==Interfaces==\r\nWi-Fi 802.11 b/g/n Radio Network\r\n(HTTP Server for GUI & API, Timekeeping,\r\nData Store connector to MongoDB,\r\nFW Filestore connector, Mail Notifications)\r\nWired CH341 USB to Serial\r\n(Debug at 115200 baud rate)\r\nWired Relays & Temp Sensor\r\n\r\n";

static const char Interfaces_Web[] PROGMEM =
  "==Interfaces==<br>\r\nWi-Fi 802.11 b/g/n Radio Network<br>\r\n(HTTP Server for GUI & API, Timekeeping,<br>\r\nData Store connector to MongoDB,<br>\r\nFW Filestore connector, Mail Notifications)<br>\r\nWired CH341 USB to Serial<br>\r\n(Debug at 115200 baud rate)<br>\r\nWired Relays & Temp Sensor<br>\r\n<br>\r\n";

static const char System_pinout[] PROGMEM =
  "===Wiring===\r\n"
  "=Power <-> Esp 8266="
  "5V micro USB - ESP 8266 micro USB slot"
  "=ESP8266 <-> 2 Relay Module Control=\r\n"
  "D5 Output - IN2 (Relay 2)\r\n"
  "D6 Output - IN1 (Relay 1)\r\n"
  "3.3V - VCC Control\r\n"
  "=ESP8266 <-> 2 Relay Module K2=\r\n"
  "3.3V - K2 NO\r\n"
  "D7 Input + 10k ohm Resistor to GND - K2 NO\r\n"
  "=Heating System <-> 2 Relay Module K1=\r\n"
  "Heating System 220V electrical connections - K1 NO\r\n"
  "=Power <-> 2 Relay Module=\r\n"
  "5V OUT - JD-VCC\r\n"
  "GND - GND\r\n"
  "=ESP8266 <-> Temp sensor=\r\n"
  "A0 Input - Center leg\r\n"
  "5V - Right leg\r\n"
  "GND - Left leg\r\n\r\n";

static const char System_pinout_Web[] PROGMEM =
  "===Wiring===<br>\r\n"
  "=Power <-> Esp 8266=<br>\r\n"
  "5V micro USB - ESP 8266 micro USB slot<br>\r\n"
  "=ESP8266 <-> 2 Relay Module Control=<br>\r\n"
  "D5 Output - IN2 (Relay 2)<br>\r\n"
  "D6 Output - IN1 (Relay 1)<br>\r\n"
  "3.3V - VCC Control<br>\r\n"
  "=ESP8266 <-> 2 Relay Module K2=<br>\r\n"
  "3.3V - K2 NO<br>\r\n"
  "D7 Input + 10k ohm Resistor to GND - K2 NO<br>\r\n"
  "=Heating System <-> 2 Relay Module K1=<br>\r\n"
  "Heating System 220V electrical connections - K1 NO<br>\r\n"
  "=Power <-> 2 Relay Module=<br>\r\n"
  "5V OUT - JD-VCC<br>\r\n"
  "GND - GND<br>\r\n"
  "=ESP8266 <-> Temp sensor=<br>\r\n"
  "A0 Input - Center leg<br>\r\n"
  "5V - Right leg<br>\r\n"
  "GND - Left leg<br><br>\r\n";

static const char Separator[] PROGMEM =
  "=====================================";

static const char Buffer_overflow[] PROGMEM =
  "WARN: Buffer Overflow requested through TCP avoided";

static const char tcp_data_to_read[] PROGMEM =
  "DEBUG: TCP Data size available for reading : ";

static const char Email_prep[] PROGMEM =
  "DEBUG: Preparing e-mail with requested command";

static const char Alarms_purge[] PROGMEM =
  "DEBUG: Alarms String purged";

static const char Inbound_purge[] PROGMEM =
  "Inbound History String purged";

static const char Outbound_purge[] PROGMEM =
  "Outbound History String purged";

static const char Relay_purge[] PROGMEM =
  "Relay History String purged";

static const char Heating_timeout_reached[] PROGMEM =
  "Timeout reached";

static const char Heating_max_temp_reached[] PROGMEM =
  "Max Temp reached";

static const char Heating_auto_timeout_reached[] PROGMEM =
  "Auto Timeout reached";

static const char Heating_auto_max_temp_reached[] PROGMEM =
  "Auto Max Temp reached";

static const char Heating_auto_min_temp_reached[] PROGMEM =
  "Auto Min Temp reached";

static const char Encryption_Types[] PROGMEM =
  "DEBUG: Encryption Types :\r\n"
  "DEBUG: 2 : ENC_TYPE_TKIP - WPA / PSK   4 : ENC_TYPE_CCMP - WPA2 / PSK\r\n"
  "DEBUG: 5 : ENC_TYPE_WEP - WEP   7 : ENC_TYPE_NONE - open network\r\n"
  "DEBUG: 8 : ENC_TYPE_AUTO - WPA / WPA2 / PSK";

static const char HTML_200_GUI_Response_Code_Headers[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n";

static const char HTML_200_API_Response_Code_Headers[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Connection: close\r\n\r\n";

static const char HTML_404_Response_Code_Headers[] PROGMEM =
  "HTTP/1.1 404 Not Found\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n";

static const char HTML_414_Response_Code_Headers[] PROGMEM =
  "HTTP/1.1 414 Request-URI Too Long\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML>\r\n"
  "<html>\r\n"
  "<CENTER><p>414 Request-URI Too Long</p></CENTER>\r\n"
  "</HTML>\r\n";

static const char HTML_503_Response_Code_Headers[] PROGMEM =
  "HTTP/1.1 503 Service Unavailable\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML>\r\n"
  "<html>\r\n"
  "<CENTER><p>503 Service Unavailable</p></CENTER>\r\n"
  "</HTML>\r\n";

static const char HTML_Root_Head_Body[] PROGMEM =
  "<!DOCTYPE HTML>\r\n"
  "<html>\r\n"
  "<head>\r\n"
  "<style>\r\n"
  "div.scrollmenu {\r\n"
  "    background-color: #333;\r\n"
  "    overflow: auto;\r\n"
  "    white-space: nowrap;\r\n"
  "}\r\n"
  "div.scrollmenu a {\r\n"
  "    display: inline-block;\r\n"
  "    color: white;\r\n"
  "    text-align: center;\r\n"
  "    padding: 14px;\r\n"
  "    text-decoration: none;\r\n"
  "}\r\n"
  "div.scrollmenu a:hover {\r\n"
  "    background-color: #777;\r\n"
  "}\r\n"
  ".btn {\r\n"
  "    border:  1px solid #33C7FF;\r\n"
  "    background-color: inherit;\r\n"
  "    padding: 12px 24px;\r\n"
  "    font-size: 12px;\r\n"
  "    cursor: pointer;\r\n"
  "    display: inline-block;\r\n"
  "}\r\n"
  "/* Green */\r\n"
  ".button_ON {\r\n"
  "   color: green;\r\n"
  "   background-color: rgba(0,255,0,0.2);\r\n"
  "   border-radius: 12px;\r\n"
  "   margin: 10px;\r\n"
  "}\r\n"
  ".button_ON:hover {\r\n"
  "   background-color: #4CAF50;\r\n"
  "   color: white;\r\n"
  "}\r\n"
  "/* Red */\r\n"
  ".button_OFF {\r\n"
  "   color: red;\r\n"
  "   background-color: rgba(255,0,0,0.2);\r\n"
  "   border-radius: 12px;\r\n"
  "   margin: 10px;\r\n"
  "}\r\n"
  ".button_OFF:hover {\r\n"
  "   background: #f44336;\r\n"
  "   color: white;\r\n"
  "}\r\n"
  "/* Green */\r\n"
  ".button_AUTO {\r\n"
  "   color: blue;\r\n"
  "   background-color: rgba(0,0,255,0.4);\r\n"
  "   border-radius: 12px;\r\n"
  "   margin: 10px;\r\n"
  "}\r\n"
  ".button_AUTO:hover {\r\n"
  "   background-color: rgba(0,0,255,0.8);\r\n"
  "   color: white;\r\n"
  "}\r\n"
  "</style>\r\n"
  "</HEAD>\r\n"
  "<body>\r\n"
  "<CENTER><div class=\"scrollmenu\">\r\n"
  "  <a href=\"/\">Home</a>\r\n"
  "  <a href=\"/alarms\">Alarms</a>\r\n"
  "  <a href=\"/historical_temp\">Temperature</a>\r\n"
  "  <a href=\"/history\">History</a>\r\n"
  "  <a href=\"/admin\">Admin</a>\r\n"
  "  <a href=\"/config\">Config</a>\r\n"
  "  <a href=\"/sys_params\">Sys Params</a>\r\n"
  "</div></CENTER>\r\n"
  "<br><CENTER>\r\n"
  "<h3>Smart Heating System Controller</h3>\r\n"
  "<script type=\"text/javascript\">"
  "console.log(\"Browser version \" + navigator.userAgent)"
  "</script>";

static const char HTML_Root_Head_Body_End[] PROGMEM =
  "<br><br>\r\n"
  "Control the Heating System :<br>\r\n"
  "<button class=\"btn button_ON\" onclick=\"window.location.href = '/relay_status=on'\">START</button>\r\n"
  "<button class=\"btn button_OFF\" onclick=\"window.location.href = '/relay_status=off'\">STOP</button>\r\n"
  "<br>\r\n"
  "<button class=\"btn button_AUTO\" onclick=\"window.location.href = '/relay_status=auto'\">AUTO</button>\r\n"
  "</p></CENTER>\r\n"
  "</body>\r\n"
  "</HTML>\r\n";

static const char HTML_Menu_Response_Head[] PROGMEM =
  "<!DOCTYPE HTML>\r\n"
  "<html>\r\n"
  "<head>\r\n"
  "<style>\r\n"
  "div.scrollmenu {\r\n"
  "    background-color: #333;\r\n"
  "    overflow: auto;\r\n"
  "    white-space: nowrap;\r\n"
  "}\r\n"
  "div.scrollmenu a {\r\n"
  "    display: inline-block;\r\n"
  "    color: white;\r\n"
  "    text-align: center;\r\n"
  "    padding: 14px;\r\n"
  "    text-decoration: none;\r\n"
  "}\r\n"
  "div.scrollmenu a:hover {\r\n"
  "    background-color: #777;\r\n"
  "}\r\n"
  "</style>\r\n"
  "</HEAD>\r\n"
  "<body>\r\n"
  "<CENTER><div class=\"scrollmenu\">\r\n"
  "  <a href=\"/\">Home</a>\r\n"
  "  <a href=\"/alarms\">Alarms</a>\r\n"
  "  <a href=\"/historical_temp\">Temperature</a>\r\n"
  "  <a href=\"/history\">History</a>\r\n"
  "  <a href=\"/admin\">Admin</a>\r\n"
  "  <a href=\"/config\">Config</a>\r\n"
  "  <a href=\"/sys_params\">Sys Params</a>\r\n"
  "</div></CENTER>\r\n"
  "<br><CENTER>\r\n"
  "<h3>Smart Heating System Controller</h3>\r\n";

static const char HTML_STOP_START_AUTO_Redirect_Success[] PROGMEM =
  "<p>Relay command Successful <br><br>\r\n";

static const char HTML_STOP_START_AUTO_Redirect_Failure[] PROGMEM =
  "<p>Relay command Failure <br><br>\r\n";

static const char Inbound_History_Response_Head[] PROGMEM =
  "Inbound Web Server Requests :<br>";

static const char Outbound_History_Response_Head[] PROGMEM =
  "<br>Outbound MongoDB, Mail, FW Update, Ping, DNS Requests :<br>";

static const char Relay_History_Response_Head[] PROGMEM =
  "<br>Relay Commands :<br>";

static const char Temp_History_Response_Head[] PROGMEM =
  "<br>Temperature Sensor Reads :<br>";

static const char HTML_Alarms_Response_Title[] PROGMEM =
  "<p>Alarms History<br><br>\r\n";

static const char HTML_History_Response_Title[] PROGMEM =
  "<p>History<br><br>\r\n";

static const char HTML_Admin_Response_Title[] PROGMEM =
  "<p>Administration<br><br>\r\n";

static const char HTML_Admin_Response_Content2[] PROGMEM =
  "<br><button onclick=\"window.location.href = '/read_and_list_memory'\">Read and list memory</button><br>\r\n"
  "<br><button onclick=\"window.location.href = '/update'\">FW Update</button><br>\r\n"
  "<br><button onclick=\"window.location.href = '/restart'\">Restart</button><br><br>\r\n";

static const char HTML_Config_Response_Title[] PROGMEM =
  "<p>Configuration<br><br>\r\n";

static const char HTML_Sys_Params_Response_Title[] PROGMEM =
  "<p>System Parameters<br><br>\r\n";

static const char HTML_Tools_Response_Title[] PROGMEM =
  "<p>Tools<br><br>\r\n";

static const char HTML_Config_Change_Response_Title[] PROGMEM =
  "<p>Config Change<br><br>\r\n";

static const char HTML_Restart_Response_Content[] PROGMEM =
  "<p>Restart initiated<br><br>\r\n";

static const char HTML_Update_Response_Content[] PROGMEM =
  "<p>FW Update initiated<br><br>\r\n";

static const char HTML_Read_Memory_Response_Content[] PROGMEM =
  "<p>Read and List Memory initiated<br><br>\r\n";

static const char HTML_Temp_Response_Content_Head_p1[] PROGMEM =
  "<p>Temperature 15 days History<br><br>\r\n"
  "<p align=\"center\">\r\n"
  "<canvas id=\"load_Canvas\" width=\"1530\" height=\"480\" style=\"border:1px solid #d3d3d3;\">\r\n"
  "Your browser does not support the HTML5 canvas tag.</canvas>\r\n"
  "</p>\r\n\r\n"
  "<script>\r\n"
  "var c1 = document.getElementById(\"load_Canvas\");\r\n"
  "var ctx1 = c1.getContext(\"2d\");\r\n"
  "var temperature_val = 0;\r\n"
  "var y=80;\r\n"
  "while (y < 400){\r\n"
  "ctx1.beginPath();ctx1.moveTo(0, y);ctx1.lineTo(1530, y);ctx1.fillText((480-y)/10+\" C\", 2, y-5);ctx1.lineWidth = 1;ctx1.stroke();y=y+100;\r\n"
  "}\r\n"
  "var x=30;\r\n"
  "var i=15;\r\n"
  "while (x < 1600){\r\n"
  "ctx1.beginPath();ctx1.moveTo(x, 0);ctx1.lineTo(x, 480);ctx1.fillText(\"Day \"+i, x-10, 10);ctx1.lineWidth = 1;ctx1.stroke();x=x+100;i=i-1;\r\n"
  "}\r\n"
  "var split_str = [];\r\n";

static const char HTML_Temp_Response_Content_Head_p2[] PROGMEM =
  "var split_str = value_str.split(\",\");\r\n"
  "var reverse_split_str = [];\r\n"
  "while (split_str.length != 0){\r\n"
  "reverse_split_str.push(split_str.pop());\r\n"
  "}\r\n"
  "var i=30;\r\n"
  "while (reverse_split_str.length != 0){\r\n"
  "temperature_val = reverse_split_str.pop();\r\n"
  "ctx1.strokeStyle = 'red';\r\n"
  "ctx1.beginPath();\r\n"
  "ctx1.moveTo(5+i, 482 - temperature_val*10 - 10);\r\n"
  "ctx1.lineTo(5+i, 482 - temperature_val*10);\r\n"
  "ctx1.fillText(Math.round(temperature_val*10)/10, 5+i-7, 482 - temperature_val*10-15);\r\n"
  "ctx1.lineWidth = 10;\r\n"
  "ctx1.stroke();\r\n"
  "i=i+25;\r\n"
  "}\r\n"
  "</script>\r\n";

static const char HTML_Not_Found_Response_Content[] PROGMEM =
  "<p>404 Page Not found<br><br>\r\n";

static const char HTML_Response_Bottom_Page[] PROGMEM =
  "<button onclick=\"window.location.href = '/'\">Back to Main Page</button><br>\r\n"
  "</p></CENTER>\r\n"
  "</body>\r\n"
  "</HTML>\r\n";
