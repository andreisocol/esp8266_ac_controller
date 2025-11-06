//Arduino IDE Setup
//Start Arduino and open Preferences window.
//Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into File -> Preferences -> Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
//Install Boards -> Open Tools -> Boards Manager Board -> esp8266@3.0.2 platform
//Select Tools -> Board -> esp8266 -> Lolin (Wemos) D1 R2 & mini
//Install Libraries -> !!! WakeOnLan@1.1.7 & !!! EMailSender@3.0.13
//Setup in Config_Secrets.h Router connection details and e-mail credentials for EMailSender

//*For USB Port Driver : http://www.wch.cn/downloads/CH341SER_ZIP.html

//ESP Exception Decodder
//Read from bottom to top
//New:
//https://github.com/dankeboy36/esp-exception-decoder?tab=readme-ov-file
//C:\Users\<username>\.arduinoIDE\plugins
//Ctrl + Shift + P -> Search ESP Excep...
//*When tracing the root cause of the problem does not work, try Closing the Arduino IDE, Re-opening and Re-compiling everything

//Log levels : CRIT ERROR WARN DEBUG INFO
//Structured Logging w Log Levels, Health status, Performance metrics, Alerts

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266httpUpdate.h>
#include <EMailSender.h>
#include <lwipopts.h>
#include <WakeOnLan.h>
//#include "PingTrace.h"

extern "C" {
#include <lwip/etharp.h>
}

#include "NTPClient.h"
#include "ESP8266Ping.h"

#include "Config_Variables.h"
#include "Routes.h"
#include "HtmlPages.h"

EMailSender emailSend(email_login, email_password, Send_Mail_from);

WiFiServer Web_server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_Server);

WiFiUDP wolUDP;
WakeOnLan WOL(wolUDP);  // Pass WiFiUDP class

WiFiUDP rDNSUDP;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(serial_DebugOutput);
  Serial.println(F("\r\n\r\n======= Starting initial ESP Board setup sequence =======\r\n"));
  initial_esp_board_setup();
  Serial.println(F("\r\n\r\n======= Starting Wi-Fi sequence =======\r\n"));
  lwip_info();
  scan_for_wifi_networks();
  setup_wifi_networks();
  timeClient.begin();
  timeClient.setTimeOffset(TimeOffset);
  timeClient.forceUpdate();
  ntp_update();
  Serial.println(F("\r\n\r\n======= Starting Testing sequence =======\r\n"));
  temp_sensor_test();
  connectivity_tests();
  Serial.println(F("\r\n======= Starting up the TCP Web Server =======\r\n"));
  start_wifi_tcp_server();
  Serial.println(F("\r\n======= Startup sequence Complete =======\r\n"));
  list_general_system_params();
  Serial.println(F("\r\n\r\n======= READY =======\r\n"));
  mail_startup_message();
  //read_and_list_memory();
}



void loop() {
  handle_recurring_tasks();
  if (Web_server.hasClient()) {
    handle_wifi_tcp_requests();
  }
}



void handle_recurring_tasks() {
  if (mail_to_send_w_delay == true) {
    recurring_task_send_mail();
  }
  if (millis() % recurring_task_1_ms == 0) {  // ~1 hour
    recurring_task_1();
  }
  if (millis() % recurring_task_2_ms == 0) {  // ~6 hours
    recurring_task_2();
  }
  if (millis() % recurring_task_3_ms == 0) {  // ~7 days
    recurring_task_3();
  }
  if (millis() % recurring_task_4_ms == 0) {  // ~30 days
    recurring_task_4();
  }
}



void handle_wifi_tcp_requests() {

  tcp_connection_request_arrival_timestamp = millis();
  String HTTP_Status_Code = "";
  Inbound_History_counter = Inbound_History_counter + 1;
  minute_http_requests_counter++;
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: TCP Client connected"));

  WiFiClient Web_server_client = Web_server.available();
  Serial.println(F("DEBUG: WIFIClient Status : ") + String(Web_server_client.status()));
  Serial.println(F("DEBUG: TCP Connection ESTABLISHED with New Client : ") + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort());
  Serial.println(FPSTR(Separator));

  //Gestionare conexiuni concurente / paralele
  if (Web_server.hasClient()) {
    Serial.println(F("WARN: Another TCP client is waiting in the background"));
    //WiFiClient Web_server_client_bkg = Web_server.available();
    //Serial.println(F("DEBUG: TCP Connection ESTABLISHED with Background Client : ") + Web_server_client_bkg.remoteIP().toString() + ":" + Web_server_client_bkg.remotePort());
    request_response_end_timestamp = millis();
    HTTP_Status_Code = F("503 Another TCP client is waiting in the background");
    Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | - | - | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | | | \r\n";
    Inbound_History_counter = Inbound_History_counter + 1;
  }

  if (!Web_server_client) {
    Serial.println(F("WARN: Socket Failure. TCP client disconnected @step1. TCP connect Scan"));
    request_response_end_timestamp = millis();
    HTTP_Status_Code = F("TCP connect Scan");
    Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort() + " | | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | | | ";
    if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
      Inbound_History = Inbound_History + "Slow Response";
    }
    Inbound_History = Inbound_History + "\r\n";
    Inbound_History_dropped_req_counter = Inbound_History_dropped_req_counter + 1;
    return;  //Return takes you back to calling function (in this case to loop())
  }

  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Waiting for Data Stream from TCP Client"));
  //Web_server_client.setTimeout(500);
  Serial.println(F("DEBUG: Data Stream read timeout_ms : ") + String(Web_server_client.getTimeout()));
  Serial.println(String(FPSTR(tcp_data_to_read)) + String(Web_server_client.available()));

  while (!Web_server_client.available()) {
    if (!Web_server_client.connected()) {
      Serial.println(F("WARN: Socket Failure. TCP client disconnected @step2"));
      request_response_end_timestamp = millis();
      HTTP_Status_Code = F("TCP client disconnected");
      Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort() + " | | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | | | ";
      if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
        Inbound_History = Inbound_History + "Slow Response";
      }
      Inbound_History = Inbound_History + "\r\n";
      Inbound_History_dropped_req_counter = Inbound_History_dropped_req_counter + 1;
      return;  //Return takes you back to calling function (in this case to loop() ?)
    }
    if (millis() > tcp_connection_request_arrival_timestamp + inbound_tcp_data_stream_timeout_ms) {
      Serial.println(F("WARN: Socket Failure. Web Server TCP waiting for Data Stream Timeout reached. Resetting TCP Connection"));
      request_response_end_timestamp = millis();
      HTTP_Status_Code = "408";
      Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort() + " | | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | | | ";
      if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
        Inbound_History = Inbound_History + "Slow Response";
      }
      Inbound_History = Inbound_History + "\r\n";
      Inbound_History_dropped_req_counter = Inbound_History_dropped_req_counter + 1;
      delay(1);
      Web_server_client.flush();
      delay(1);
      //Web_server_client.stop();   //To be used only in conjunction with Header "Content-Length: <value>"
      return;  //Return takes you back to calling function (in this case to loop() ?)
    } else {
      delay(10);
    }
  }

  tcp_data_stream_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Started to receive Data from TCP Client"));
  if (Web_server_client.available() > tcp_buffer_overflow_threshold) {
    Serial.println(FPSTR(Buffer_overflow));
    Web_server_client.print(FPSTR(HTML_414_Response_Code_Headers));
    HTTP_Status_Code = "414";
    Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort() + " | | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | | | ";
    if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
      Inbound_History = Inbound_History + "Slow Response";
    }
    Inbound_History = Inbound_History + "\r\n";
    return;  //Return takes you back to calling function (in this case to loop() ?)
  }

  // Read the first line of the http_request_url
  String http_request_url = Web_server_client.readStringUntil('\r');
  Serial.println(F("DEBUG: Request URL length : ") + String(http_request_url.length()));
  Serial.println(String(http_request_url));
  network_incoming_bytes_counter = network_incoming_bytes_counter + http_request_url.length();

  String http_request_headers_and_body;
  while (Web_server_client.available() > 1) {
    //Serial.println(String(FPSTR(tcp_data_to_read)) + String(Web_server_client.available()));
    http_request_headers_and_body = http_request_headers_and_body + Web_server_client.readStringUntil('\r');
  }
  Web_server_client.flush();
  Serial.println(F("DEBUG: Request Headers and Body length : ") + String(http_request_headers_and_body.length()));
  Serial.println(String(http_request_headers_and_body));
  network_incoming_bytes_counter = network_incoming_bytes_counter + http_request_headers_and_body.length();

  request_process_start_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Started to process Request"));
  Serial.println(FPSTR(Separator));

  read_temp_sensor();
  Free_Heap = ESP.getFreeHeap();

  if (http_request_url.indexOf(FPSTR(GUI_Root_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Root Index Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Root_Head_Body));
    Web_server_client.print(F("<p>"));
    Web_server_client.println(String(timeClient.getFormattedDate()));
    Web_server_client.println(F("<br><br>"));
    Web_server_client.print(F("Ambient temperature :<br>"));
    Web_server_client.println(F("<font size=\"5\">") + String(temperature) + F(" C</font><br>"));
    if (Relay_Failure_counter > 0) {
      Web_server_client.println(F("&#9888; Relay Failures : ") + String(Relay_Failure_counter) + F("<br>"));
    }
    if (Alarms_counter > 0) {
      Web_server_client.println(F("&#9888; Alarms : ") + String(Alarms_counter) + F("<br>"));
    }
    Web_server_client.println(F("<br>Relay State :"));
    Central_Heating_State = digitalRead(Relay_State_Pin);  //Read Confirmation Relay's State
    if (Central_Heating_State == HIGH) {
      Web_server_client.println(F("<br><font size=\"5\" color=\"green\">STARTED</font>"));
    } else if (Central_Heating_State == LOW) {
      Web_server_client.println(F("<br><font size=\"5\" color=\"red\">STOPPED</font>"));
    }

    Web_server_client.println(F("<br>Operation Mode : "));
    if (heating_auto_switch == HIGH) {
      Web_server_client.println(F("<br>AUTO"));
    } else if (heating_auto_switch == LOW) {
      Web_server_client.println(F("<br>MANUAL"));
    }

    Web_server_client.print(FPSTR(HTML_Root_Head_Body_End));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Routes_Endpoint)) != -1 || http_request_url.indexOf(FPSTR(API_Routes_Endpoint1)) != -1 || http_request_url.indexOf(FPSTR(API_Routes_Endpoint2)) != -1) {
    Serial.println(F("DEBUG: Requested endpoints listing"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print("{\"endpoints\":\"" + String(FPSTR(GUI_Root_Endpoint)) + ", " + String(FPSTR(GUI_Routes_Endpoint)) + ", " + String(FPSTR(GUI_Alarms_Endpoint)) + ", " + String(FPSTR(GUI_Historical_Temp_Endpoint)) + ", " + String(FPSTR(GUI_History_Endpoint)) + ", " + String(FPSTR(GUI_Admin_Endpoint)) + ", " + String(FPSTR(GUI_Config_Endpoint)) + ", " + String(FPSTR(GUI_Sys_Params_Endpoint)) + ", " + String(FPSTR(GUI_Relay_status_on_Endpoint)) + ", " + String(FPSTR(GUI_Relay_status_off_Endpoint)) + ", " + String(FPSTR(GUI_Relay_status_auto_Endpoint)) + ", " + String(FPSTR(GUI_Restart_Endpoint)) + ", " + String(FPSTR(GUI_Update_Endpoint)) + ", " + String(FPSTR(API_Routes_Endpoint1)) + ", " + String(FPSTR(API_Refresh_Endpoint)) + ", " + String(FPSTR(API_Status_Endpoint)) + ", " + String(FPSTR(API_External_Components_Status_Endpoint)) + ", " + String(FPSTR(API_Alarms_Endpoint)) + ", " + String(FPSTR(API_Historical_Temp_Endpoint)) + ", " + String(FPSTR(API_History_Endpoint)) + ", " + String(FPSTR(API_Relay_status_on_Endpoint)) + ", " + String(FPSTR(API_Relay_status_off_Endpoint)) + ", " + String(FPSTR(API_Relay_status_auto_Endpoint)) + ", " + String(FPSTR(API_Restart_Endpoint)) + ", " + String(FPSTR(API_Update_Endpoint)) + ", " + String(FPSTR(API_Push_Notification_Endpoint)) + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_History_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Requests History through GUI"));
    Inbound_History.replace("\r\n", "<br>");
    Outbound_History.replace("\r\n", "<br>");
    Relay_History.replace("\r\n", "<br>");
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_History_Response_Title));
    Web_server_client.println(FPSTR(Inbound_History_Response_Head));
    Web_server_client.println(F("Total Requests : ") + String(Inbound_History_counter) + F("<br>"));
    Web_server_client.println("");
    Web_server_client.println(F("No. | Time | Timestamp | From | HTTP Reqest | Status Code | TCP Connection | Data Stream | Processing<br>"));
    Web_server_client.println(Inbound_History);
    Web_server_client.println(FPSTR(Outbound_History_Response_Head));
    Web_server_client.println(F("Total Requests : ") + String(Outbound_History_counter) + F("<br>"));
    Web_server_client.println("");
    Web_server_client.println(F("No. | Time | Timestamp | To | Operation | Status Code | Processing<br>"));
    Web_server_client.println(Outbound_History);
    Web_server_client.println(FPSTR(Relay_History_Response_Head));
    Web_server_client.println(F("Total Commands : ") + String(Relay_History_counter) + F("<br>"));
    Web_server_client.println(F("No. | Time | Timestamp | Operation | Result<br>"));
    Web_server_client.println(String(Relay_History));
    Inbound_History.replace("<br>", "\r\n");
    Outbound_History.replace("<br>", "\r\n");
    Relay_History.replace("<br>", "\r\n");
    Web_server_client.println(FPSTR(Temp_History_Response_Head));
    Web_server_client.println(F("Total Queries : ") + String(read_temp_sensor_counter) + F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Admin_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Admin Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_Admin_Response_Title));

    Web_server_client.println(FPSTR(System_Components_Web));
    Web_server_client.println(FPSTR(External_Components_Web));
    Web_server_client.print(F("<button onclick=\"window.location.href = '/api/ext_comp_status'\">Check External Components</button><br><br>\r\n"));
    Web_server_client.println(FPSTR(Interfaces_Web));
    Web_server_client.println(FPSTR(System_pinout_Web));

    Web_server_client.println(F("==HW Info=="));
    Web_server_client.print(F("<br>Board : "));
    Web_server_client.println(ARDUINO_BOARD);
    Web_server_client.print(F("<br>CpuFreqMHz : "));
    Web_server_client.println(ESP.getCpuFreqMHz());
    Web_server_client.print(F("<br>FlashChipRealSize : "));
    Web_server_client.println(ESP.getFlashChipRealSize());
    Web_server_client.print(F("<br>FlashChipSize : "));
    Web_server_client.println(ESP.getFlashChipSize());
    Web_server_client.print(F("<br>FlashChipSizeByChipId : "));
    Web_server_client.println(ESP.getFlashChipSizeByChipId());
    Web_server_client.print(F("<br>FlashChipSpeed : "));
    Web_server_client.println(ESP.getFlashChipSpeed());

    Web_server_client.println(F("<br><br>==SW Info=="));
    Web_server_client.print(F("<br>AppVersion : "));
    Web_server_client.println(String(FPSTR(Version_String)));
    Web_server_client.print(F("<br>Arduino IdeVersion : ") + String(ARDUINO, DEC));
    Web_server_client.print(F("<br>Arduino C++ Compiler : ") + String(__cplusplus));
    Web_server_client.print(F("<br>ESP "));
    Web_server_client.println(ESP.getFullVersion());
    Web_server_client.print(F("<br>Compiled : "__DATE__
                              ", "__TIME__
                              ", "__VERSION__));
    Web_server_client.print(F("<br>SketchSize : "));
    Web_server_client.println(ESP.getSketchSize());
    Web_server_client.print(F("<br>FreeSketchSpace : "));
    Web_server_client.println(ESP.getFreeSketchSpace());

    unsigned long measured_arrival_timestamp = millis();
    Web_server_client.print(F("<br>SketchMD5 : "));
    Web_server_client.println(ESP.getSketchMD5());
    unsigned long measured_response_timestamp = millis();
    Serial.println("DEBUG: Performance: MD5 command duration : " + String(measured_response_timestamp - measured_arrival_timestamp) + "ms");

    Web_server_client.println(F("<br><br>==Tools=="));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "DNS_lookup", "google.com", "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "Reverse_DNS", WiFi.dnsIP().toString(), "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "Ping", ESP_FW_Filestore_Server, "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "ARP_Ping", WiFi.gatewayIP().toString(), "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "TCP_Connect", String(ESP_FW_Filestore_Server + ":" + ESP_FW_Filestore_Port), "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "Wake_On_Lan", String("78:E3:B5:C4:24:59"), "Submit"));
    Web_server_client.println(render_html_simple_form_string(F("/operation"), "GET", "External_IP", "checkip.dyndns.com/api.ipify.org", "Submit"));  //checkip.dyndns.com/api.ipify.org
    Web_server_client.println(render_html_simple_6_select_string(F("/operation"), "GET", "MongoDB_Query", "Exception", "Hardware Watchdog", "Software Watchdog", "External System", "Software/System restart", "Power On", "Submit"));

    Web_server_client.print(FPSTR(HTML_Admin_Response_Content2));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }



  else if (http_request_url.indexOf(FPSTR(GUI_Config_Change_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Config Change Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Config_Change_Response_Title));
    String query_param = "";
    query_param = http_request_url.substring(http_request_url.indexOf("\?") + 1);
    String param = "";
    param = query_param.substring(query_param.indexOf("=") + 1, query_param.indexOf(" HTTP/1.1"));
    //Web_server_client.print("param: " + String(param) + "<br>");
    if (query_param.indexOf(F("Serial_Debug_Output")) != -1) {
      serial_DebugOutput = bool(param.toInt());
      Serial.setDebugOutput(serial_DebugOutput);
      Web_server_client.print(F("Serial_Debug_Output"));
    } else if (query_param.indexOf(F("Mail_Notifications")) != -1) {
      SMTP_Notifications = bool(param.toInt());
      Web_server_client.print(F("Mail_Notifications"));
    } else if (query_param.indexOf(F("MongoDB_Operations")) != -1) {
      MongoDB_Operations = bool(param.toInt());
      Web_server_client.print(F("MongoDB_Operations"));
    } else if (query_param.indexOf(F("MongoDB")) != -1) {
      param.replace("%3A", ":");
      MongoDB_Server = param.substring(0, param.indexOf(":")).c_str();
      MongoDB_Server_port = param.substring(param.indexOf(":") + 1, param.length()).toInt();
      Web_server_client.print(F("MongoDB"));
    } else if (query_param.indexOf(F("FW_Filestore")) != -1) {
      param.replace("%3A", ":");
      ESP_FW_Filestore_Server = param.substring(0, param.indexOf(":"));
      ESP_FW_Filestore_Port = param.substring(param.indexOf(":") + 1, param.length()).toInt();
      Web_server_client.print(F("FW_Filestore"));
    } else if (query_param.indexOf(F("Gateway")) != -1) {
      Serial.println(F("DEBUG: Network Gateway Change"));
      Web_server_client.print(F("Network Gateway Change"));
      IPAddress new_gw_addr, new_ip, new_dns;
      new_gw_addr.fromString(param);
      new_ip = WiFi.localIP();
      new_dns = WiFi.dnsIP();
      WiFi.disconnect();
      if (!WiFi.config(new_ip, new_dns, new_gw_addr)) {
        Serial.println(F("Failed to configure Network Interface"));
      }
      WiFi.begin(ssid_prod, password_prod);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println(F("\r\nDEBUG: WiFi connected"));
      Serial.println("");
      Serial.println(FPSTR(Separator));
      Serial.print(F("DEBUG: WiFi Interface IP : "));
      Serial.println(WiFi.localIP());
      Serial.print(F("DEBUG: WiFi Interface SubnetMask : "));
      Serial.println(WiFi.subnetMask());
      Serial.print(F("DEBUG: WiFi Interface GatewayIP : "));
      Serial.println(WiFi.gatewayIP());
      Serial.print(F("DEBUG: WiFi Interface DNS : "));
      Serial.println(WiFi.dnsIP().toString());
    } else if (query_param.indexOf(F("DNS")) != -1) {
      Serial.println(F("DEBUG: Network DNS Change"));
      Web_server_client.print(F("Network DNS Change"));
      IPAddress new_gw_addr, new_ip, new_dns;
      new_gw_addr = WiFi.gatewayIP();
      new_ip = WiFi.localIP();
      new_dns.fromString(param);
      WiFi.disconnect();
      if (!WiFi.config(new_ip, new_dns, new_gw_addr)) {
        Serial.println(F("Failed to configure Network Interface"));
      }
      WiFi.begin(ssid_prod, password_prod);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println(F("\r\nDEBUG: WiFi connected"));
      Serial.println("");
      Serial.println(FPSTR(Separator));
      Serial.print(F("DEBUG: WiFi Interface IP : "));
      Serial.println(WiFi.localIP());
      Serial.print(F("DEBUG: WiFi Interface SubnetMask : "));
      Serial.println(WiFi.subnetMask());
      Serial.print(F("DEBUG: WiFi Interface GatewayIP : "));
      Serial.println(WiFi.gatewayIP());
      Serial.print(F("DEBUG: WiFi Interface DNS : "));
      Serial.println(WiFi.dnsIP().toString());
    } else if (query_param.indexOf(F("Save_Temp_Interval_s")) != -1) {
      recurring_task_2_ms = param.toInt() * 1000;
      Web_server_client.print(F("Save_Temp_Interval set"));
    } else if (query_param.indexOf(F("Auto_Update_Interval_s")) != -1) {
      recurring_task_4_ms = param.toInt() * 1000;
      Web_server_client.print(F("Auto_Update_Interval set"));
    } else if (query_param.indexOf(F("Aliveness_Interval_s")) != -1) {
      recurring_task_3_ms = param.toInt() * 1000;
      Web_server_client.print(F("Aliveness_Interval set"));
    } else {
      Web_server_client.print(F("No matching variable name"));
    }
    Web_server_client.print(F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }



  else if (http_request_url.indexOf(FPSTR(GUI_Config_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Config Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_Config_Response_Title));

    Web_server_client.print(F("Timezone : UTC+") + String(TimeOffset / 3600));
    Web_server_client.print(F("<br><br>Manual Heating timeout : ") + String((heating_timeout_ms / 1000) / 3600) + "h");
    Web_server_client.print(F("<br>Manual Heating max temp : ") + String(heating_max_temp) + "C");
    Web_server_client.print(F("<br><br>Auto Heating timeout : ") + String((heating_auto_timeout_ms / 1000) / 3600) + "h");
    Web_server_client.print(F("<br>Auto Heating min temp : ") + String(heating_auto_min_temp) + "C");
    Web_server_client.print(F("<br>Auto Heating max temp : ") + String(heating_auto_max_temp) + "C");
    Web_server_client.print(F("<br><br>Temp Sensor warning value : < ") + String(temp_warn_value) + "C");
    Web_server_client.println(F("<br>"));
    Web_server_client.println(render_html_simple_form_bool(F("/config_change"), "GET", "Serial_Debug_Output", serial_DebugOutput));
    Web_server_client.println(render_html_simple_form_bool(F("/config_change"), "GET", "Mail_Notifications", SMTP_Notifications));
    Web_server_client.println(render_html_simple_form_bool(F("/config_change"), "GET", "MongoDB_Operations", MongoDB_Operations));

    Web_server_client.println(F("<br>Network Info"));
    Web_server_client.print(F("<br>IP_Forward : ") + String(IP_FORWARD));
    if (IP_FORWARD == true) {
      Web_server_client.println(" &#9989;");
    }
    Web_server_client.print(F("<br>IP_NAPT : ") + String(IP_NAPT));
    Web_server_client.println(render_html_simple_form_string(F("/config_change"), "GET", "Gateway", WiFi.gatewayIP().toString(), "Save"));
    Web_server_client.println(render_html_simple_form_string(F("/config_change"), "GET", "DNS", WiFi.dnsIP().toString(), "Save"));

    Web_server_client.print(F("NTP : ") + String(NTP_Server));

    Web_server_client.print(F("<br>Mail : ") + String(SMTP_Server) + F(":") + String(SMTP_PORT));

    Web_server_client.print(render_html_form_server_config(F("/config_change"), "GET", "MongoDB", MongoDB_Server, MongoDB_Server_port));

    Web_server_client.print(render_html_form_server_config(F("/config_change"), "GET", "FW_Filestore", ESP_FW_Filestore_Server, ESP_FW_Filestore_Port));

    Web_server_client.println(F("FW_Filestore_file : ") + Prod_ESP_FW_Filestore_file);
    Web_server_client.println(render_html_simple_form_string(F("/config_change"), "GET", "Save_Temp_Interval_s", String(recurring_task_2_ms / 1000), "Save"));
    Web_server_client.println(render_html_simple_form_string(F("/config_change"), "GET", "Aliveness_Interval_s", String(recurring_task_3_ms / 1000), "Save"));
    Web_server_client.println(render_html_simple_form_string(F("/config_change"), "GET", "Auto_Update_Interval_s", String(recurring_task_4_ms / 1000), "Save"));
    Web_server_client.println(F("Send_Mail_from : ") + String(Send_Mail_from));
    Web_server_client.println(F("<br>Send_Mail_to : ") + String(Send_Mail_to));
    Web_server_client.println(F("<br>Email_user : ") + String(email_login));
    Web_server_client.println(F("<br>Email_password : ") + String("email_password"));
    Web_server_client.println(F("<br>Email_subject : ") + String(FPSTR(email_subject) + WiFi.hostname()));

    Web_server_client.print(F("<br><br>PING timeout_ms : 1000"));
    Web_server_client.print(F("<br>DNS timeout_ms : 7000"));
    Web_server_client.print(F("<br>TCP Connection timeout_ms : 5000"));
    Web_server_client.print(F("<br>TCP Secure Connection timeout_ms : 15000"));
    Web_server_client.print(F("<br>TCP Web Srv Socket read timeout_ms : "));
    Web_server_client.print(String(Web_server_client.getTimeout()));
    Web_server_client.print(F("<br>TCP Mongo Socket read timeout_ms : "));
    Web_server_client.print(String(mongo_tcp_stream_read_timeout_ms));
    Web_server_client.print(F("<br>Web Server max_pending_clients_per_port : 5"));
    Web_server_client.println(F("<br><br>Wifi General Info"));
    Web_server_client.print(F("<br>Mode : "));
    Web_server_client.println(WiFi.getMode());
    Web_server_client.print(F("<br>PhyMode : "));
    Web_server_client.println(WiFi.getPhyMode());
    Web_server_client.print(F("<br>SleepMode : "));
    Web_server_client.println(WiFi.getSleepMode());
    Web_server_client.println(F("<br><br>WiFi Station Interface Info"));
    Web_server_client.print(F("<br>Status : "));
    Web_server_client.println(WiFi.status());
    Web_server_client.print(F("<br> isConnected : "));
    Web_server_client.println(WiFi.isConnected());
    Web_server_client.print(F("<br>AutoConnect : "));
    Web_server_client.println(WiFi.getAutoConnect());
    Web_server_client.print(F("<br>Channel : "));
    Web_server_client.println(WiFi.channel());
    Web_server_client.print(F("<br>RSSI : "));
    Web_server_client.println(WiFi.RSSI());
    Web_server_client.print(F("<br>SSID : "));
    Web_server_client.println(WiFi.SSID());
    Web_server_client.print(F("<br>PSK : "));
    Web_server_client.println(WiFi.psk());
    Web_server_client.print(F("<br>MAC : "));
    Web_server_client.println(WiFi.macAddress());
    Web_server_client.print(F("<br>IP : "));
    Web_server_client.println(WiFi.localIP());
    Web_server_client.print(F("<br>SubnetMask : "));
    Web_server_client.println(WiFi.subnetMask());
    Web_server_client.print(F("<br>GatewayIP : "));
    Web_server_client.println(WiFi.gatewayIP());
    Web_server_client.print(F("<br>DNS : "));
    Web_server_client.println(WiFi.dnsIP().toString());
    Web_server_client.print(F("<br>Hostname : "));
    Web_server_client.println(WiFi.hostname());
    Web_server_client.println(F("<br><br>WiFi Soft-AP Interface Info"));
    Web_server_client.print(F("<br>SSID : "));
    Web_server_client.println(WiFi.softAPSSID());
    Web_server_client.print(F("<br>PSK : "));
    Web_server_client.println(WiFi.softAPPSK());
    Web_server_client.print(F("<br>MAC : "));
    Web_server_client.println(WiFi.softAPmacAddress());
    Web_server_client.print(F("<br>IP : "));
    Web_server_client.println(WiFi.softAPIP());
    Web_server_client.print(F("<br>Stations connected to Soft-AP : "));
    Web_server_client.println(WiFi.softAPgetStationNum());

    Web_server_client.print(F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Sys_Params_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Sys Params Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_Sys_Params_Response_Title));

    Web_server_client.print(F("Uptime : "));
    Web_server_client.println(String(millis() / 1000 / 3600) + F(" hours"));

    Web_server_client.print(F("<br>Status : "));
    if (Relay_Failure_counter == 0) {
      Web_server_client.println(F("OK<br>"));
    } else {
      Web_server_client.println(F("Unhealthy<br>"));
    }

    Web_server_client.print(F("<br>Capacity Utilization (us+sy/time) : "));
    Web_server_client.print(String(last_processing_time / 1000) + "s + " + String(last_recurring_tasks_processing_time / 1000) + "s / " + String(processing_interval / 1000) + "s ");
    Web_server_client.print(F("<br>TPS : "));
    Web_server_client.println(String(last_minute_http_requests_counter) + " / " + String(processing_interval / 1000) + "s");
    Web_server_client.println(F("<br>Dropped Inbound Requests : ") + String(Inbound_History_dropped_req_counter) + F(" / ") + String(Inbound_History_counter));

    Web_server_client.print(F("<br><br>Free Heap Ratio : "));
    Web_server_client.println(String(Free_Heap * 100 / 81920) + "%");
    Web_server_client.print(F("<br>Free Heap Mem : "));
    Web_server_client.println(String(Free_Heap / 1024) + F(" / 80 KB (") + String(Free_Heap) + " / 81920 Bytes)");
    Web_server_client.print(F("<br>Free Heap Fragmentation : "));
    Web_server_client.println(String(ESP.getHeapFragmentation()) + "%");
    Web_server_client.print(F("<br>Max Free BlockSize : "));
    Web_server_client.println(String(ESP.getMaxFreeBlockSize()) + " Bytes");

    Web_server_client.print(F("<br><br>Network inbound traffic : "));
    Web_server_client.println(String(network_incoming_bytes_counter / 1024) + " KB");

    Web_server_client.print(F("<br><br>ResetReason : "));
    Web_server_client.println(ESP.getResetReason());
    Web_server_client.print(F("<br>ResetInfo : "));
    Web_server_client.println(ESP.getResetInfo());

    Web_server_client.print(F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Alarms_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Alarms History"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_Alarms_Response_Title));
    Web_server_client.println(F("Alarms : "));
    Web_server_client.println(F("<br>Relay Failures : ") + String(Relay_Failure_counter));
    Web_server_client.println(F("<br>Temp Sensor Failures : ") + String(Temp_Sens_Err_Counter));
    Web_server_client.println(F("<br>Flash CRC Failures : ") + String(Flash_CRC_Failure_counter));
    Free_Heap = ESP.getFreeHeap();
    if (Free_Heap < 30000) {
      Web_server_client.println(F("<br>Low Memory Warning (under 30KB)"));
    }
    Web_server_client.println(F("<br><br>"));
    Web_server_client.println(F("No. | Time | Timestamp | Alarm<br>"));
    Alarms_History.replace("\r\n", "<br>");
    Web_server_client.println(Alarms_History);
    Alarms_History.replace("<br>", "\r\n");
    Web_server_client.println(F("<br>Alerts : "));
    Web_server_client.println(F("<br>DNS Query Failures : ") + String(Dns_Query_Err_Counter));
    Web_server_client.println(F("<br>PING Failures : ") + String(Ping_Err_Counter));
    Web_server_client.println(F("<br>ARP PING Failures : ") + String(ARP_Ping_Err_Counter));
    Web_server_client.println(F("<br>External IP Failures : ") + String(External_IP_Err_Counter));
    Web_server_client.println(F("<br>WOL Failures : ") + String(WOL_Err_Counter));
    Web_server_client.println(F("<br>rDNS Failures : ") + String(rDNS_Err_Counter));
    Web_server_client.println(F("<br>TCP Connection Failures : ") + String(TCP_Connection_Err_Counter));
    Web_server_client.println(F("<br>NTP Failures : ") + String(NTP_Err_Counter));
    Web_server_client.println(F("<br>Mail Failures : ") + String(SMTP_Err_Counter));
    Web_server_client.println(F("<br>Mongo Failures : ") + String(Mongo_Err_Counter));
    Web_server_client.println(F("<br>FW Update Failures : ") + String(FW_Update_Err_Counter));
    Web_server_client.println(F("<br>Dropped Inbound Requests : ") + String(Inbound_History_dropped_req_counter) + F(" / ") + String(Inbound_History_counter));
    Web_server_client.println(F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Historical_Temp_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Temperature History"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.println(FPSTR(HTML_Temp_Response_Content_Head_p1));
    Web_server_client.print("var value_str = \"");
    for (byte i = 0; i < 59; i = i + 1) {
      Web_server_client.print(String(Temp_values_circular_buf[i]) + ",");
    }
    Web_server_client.println(String(Temp_values_circular_buf[59]) + "\";");
    Web_server_client.println(FPSTR(HTML_Temp_Response_Content_Head_p2));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Relay_status_on_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested START command through GUI"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    if (relay_start_command() == 0) {
      Web_server_client.print(FPSTR(HTML_STOP_START_AUTO_Redirect_Success));
      Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    } else {
      Web_server_client.print(FPSTR(HTML_STOP_START_AUTO_Redirect_Failure));
      Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    }
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Relay_status_off_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested STOP command through GUI"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    if (relay_stop_command() == 0) {
      Web_server_client.print(FPSTR(HTML_STOP_START_AUTO_Redirect_Success));
      Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    } else {
      Web_server_client.print(FPSTR(HTML_STOP_START_AUTO_Redirect_Failure));
      Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    }
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Relay_status_auto_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested AUTO w delay command through GUI"));
    heating_auto_switch = 1;
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_STOP_START_AUTO_Redirect_Success));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
    start_auto_heating();
  }

  else if (http_request_url.indexOf(FPSTR(API_Alarms_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Alarms History through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print("{\"alarms\":\"" + Alarms_History + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_Push_Notification_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Push Notifications through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    String JSON = "";
    JSON = http_request_headers_and_body.substring(http_request_headers_and_body.indexOf("{\"message\":"));
    Serial.println("JSON: " + String(JSON));

    if (String(JSON).length() > 0) {
      mail_to_send_w_delay = true;
      mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " " + JSON;
      Web_server_client.print("{\"status\":\"success\"}");
      HTTP_Status_Code = "200";
    } else {
      Web_server_client.print("{\"status\":\"failure, message has to be in JSON format and include as key the message keyword\"}");
      HTTP_Status_Code = "400";
    }

  }

  else if (http_request_url.indexOf(FPSTR(API_Relay_status_on_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested START command through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    if (relay_start_command() == 0) {
      Web_server_client.print(F("{\"relay_command\":\"success\"}"));
    } else {
      Web_server_client.print(F("{\"relay_command\":\"failure\"}"));
    }
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_Relay_status_off_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested STOP command through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    if (relay_stop_command() == 0) {
      Web_server_client.print(F("{\"relay_command\":\"success\"}"));
    } else {
      Web_server_client.print(F("{\"relay_command\":\"failure\"}"));
    }
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_Relay_status_auto_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested AUTO w delay command through API"));
    heating_auto_switch = 1;
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print(F("{\"relay_command\":\"auto\"}"));
    HTTP_Status_Code = "200";
    start_auto_heating();
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Restart_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Restart Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Restart_Response_Content));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    delay(1);                   //1ms instructed
    Web_server_client.flush();  //Need to test this
    delay(1);
    Web_server_client.stop();  //To be used only in conjunction with Header "Content-Length: <value>"
    notify_restart();
    ESP.restart();
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Update_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Update Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Update_Response_Content));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    delay(1);  //1ms instructed
    ESP_FW_Update();
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Tools_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Tools Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Tools_Response_Title));
    String query_param = "";
    query_param = http_request_url.substring(http_request_url.indexOf("\?") + 1);
    String param = "";
    param = query_param.substring(query_param.indexOf("=") + 1, query_param.indexOf(" HTTP/1.1"));
    //Web_server_client.print("param: " + param + "<br>");
    Serial.println(FPSTR(Separator));
    if (query_param.indexOf(F("DNS_lookup")) != -1) {
      Serial.println(F("DEBUG: Requested DNS_lookup Page"));
      IPAddress remote_addr = remote_host_dns_lookup(param.c_str());
      if (remote_addr) {
        Web_server_client.print(param + " " + String(remote_addr[0]) + "." + String(remote_addr[1]) + "." + String(remote_addr[2]) + "." + String(remote_addr[3]) + F("<br>DNS_lookup OK"));
      } else {
        Web_server_client.print(param + F("<br>DNS_lookup Failed"));
      }
    } else if (query_param.indexOf(F("ARP_Ping")) != -1) {
      Serial.println(F("DEBUG: Requested ARP_Ping Page"));
      remote_host_ping_check(param.c_str());
      Web_server_client.println(IP_String_to_IPAddress_w_arp_ping(param));
    } else if (query_param.indexOf(F("Ping")) != -1) {
      Serial.println(F("DEBUG: Requested Ping Page"));
      if (remote_host_ping_check(param.c_str()) == true) {
        Web_server_client.print(param + F("<br>Ping OK"));
      } else {
        Web_server_client.print(param + F("<br>Ping Failed"));
      }
    } else if (query_param.indexOf(F("TCP_Connect")) != -1) {
      Serial.println(F("DEBUG: Requested TCP_Connect Page"));
      param.replace("%3A", ":");
      String host = param.substring(0, param.indexOf(":"));
      int port = param.substring(param.indexOf(":") + 1, param.length()).toInt();
      bool connection = false;
      if (connection_test(host.c_str(), port, connection) == true) {
        Web_server_client.print(param + F("<br>Connection OK"));
      } else {
        Web_server_client.print(param + F("<br>Connection Failed"));
      }
    } else if (query_param.indexOf(F("Wake_On_Lan")) != -1) {
      Serial.println(F("DEBUG: Requested Wake_On_Lan Page"));
      param.replace("%3A", ":");
      WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
      //WOL.sendMagicPacket(param);
      if (WOL.sendMagicPacket(param) == true) {
        Web_server_client.print(param + F("<br>WOL Sent"));
      } else {
        Web_server_client.print(param + F("<br>WOL Failed"));
        WOL_Err_Counter++;
      }
    } else if (query_param.indexOf(F("External_IP")) != -1) {
      Serial.println(F("DEBUG: Requested External_IP Page"));
      param.replace("%3A", ":");
      WiFiClient tcp_ext_ip_client;
      if (tcp_ext_ip_client.connect(param, 80)) {
        Web_server_client.println(F("Connected to ") + param + F("<br>"));
        String External_ip_request_payload = F("GET / HTTP/1.1\r\nHost: ") + param + F("\r\nUser-Agent: ESP8266\r\nConnection: close\r\n\r\n");
        Serial.println(F("DEBUG: Data Stream read timeout_ms : ") + String(tcp_ext_ip_client.getTimeout()));
        Serial.println(F("DEBUG: Payload : ") + External_ip_request_payload);
        tcp_ext_ip_client.print(External_ip_request_payload);
        Web_server_client.println(F("Query sent<br>"));
        Serial.println(F("DEBUG: Response : "));
        while (tcp_ext_ip_client.connected() || tcp_ext_ip_client.available()) {
          if (tcp_ext_ip_client.available()) {
            while (tcp_ext_ip_client.available()) {
              String line = tcp_ext_ip_client.readStringUntil('\r\n');
              Serial.print(line);
              Web_server_client.println(line + F("<br>"));
            }
          }
        }
        Web_server_client.println(F("External_IP Check OK"));
      } else {
        Web_server_client.println(F("External_IP Check failed"));
        External_IP_Err_Counter++;
      }
    } else if (query_param.indexOf(F("MongoDB_Query")) != -1) {
      Serial.println(F("DEBUG: Requested MongoDB_Query Page"));
      Serial.println(FPSTR(Separator));
      Serial.println(F("DEBUG: Starting MongoDB connection"));
      WiFiClient tcp_mongo_client;
      if (tcp_mongo_client.connect(MongoDB_Server, MongoDB_Server_port)) {
        Serial.println(F("DEBUG: TCP connection OK to remote host ") + String(MongoDB_Server) + ":" + String(MongoDB_Server_port));
        Web_server_client.println(F("Connected to ") + MongoDB_Server + F("<br>"));
        Mongo_request_full_payload = F("GET /iot/audit/?filter_ResetReason=") + param + F("&limit=-2000 HTTP/1.1\r\n\r\n");  //skip=5 can be an option
        Serial.println(F("DEBUG: Data Stream read timeout_ms : ") + String(tcp_mongo_client.getTimeout()));
        Serial.println(F("DEBUG: Payload : ") + Mongo_request_full_payload);
        tcp_mongo_client.print(Mongo_request_full_payload);
        Web_server_client.println(F("Query sent<br>"));
        Serial.println(F("DEBUG: Response : "));
        while (tcp_mongo_client.connected() || tcp_mongo_client.available()) {
          if (tcp_mongo_client.available()) {
            while (tcp_mongo_client.available()) {
              String line = tcp_mongo_client.readStringUntil('\r\n');
              Serial.print(line);
              Web_server_client.println(line + F("<br>"));
            }
          }
        }
        Web_server_client.println(F("MongoDB_Query Check OK"));
      } else {
        Web_server_client.println(F("MongoDB_Query Check failed"));
      }
    } else if (query_param.indexOf(F("Reverse_DNS")) != -1) {
      Serial.println(F("DEBUG: Requested Reverse_DNS Page"));
      Web_server_client.print(performReverseDNS(param.c_str(), WiFi.gatewayIP(), 53));
    } else {
      Serial.println(F("DEBUG: No matching variable name"));
      Web_server_client.print(F("No matching variable name"));
    }
    Web_server_client.print(F("<br><br>"));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "200";
  }



  else if (http_request_url.indexOf(FPSTR(API_Restart_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Restart Page through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print(F("{\"status\":\"restarting\"}"));
    delay(1);                   //1ms instructed
    Web_server_client.flush();  //Need to test this
    delay(1);
    Web_server_client.stop();  //To be used only in conjunction with Header "Content-Length: <value>"
    notify_restart();
    ESP.restart();
  }

  else if (http_request_url.indexOf(FPSTR(API_Update_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Update Page through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print(F("{\"status\":\"updating\"}"));
    delay(1);  //1ms instructed
    ESP_FW_Update();
  }

  else if (http_request_url.indexOf(FPSTR(API_Refresh_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Refresh through API"));
    Central_Heating_State = digitalRead(Relay_State_Pin);  //Read Confirmation Relay's State
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.println("{\"temperature\":\"" + String(temperature) + "\",");
    Web_server_client.println("\"relay_state\":\"" + String(Central_Heating_State) + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_Status_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Status through API"));
    Central_Heating_State = digitalRead(Relay_State_Pin);  //Read Confirmation Relay's State
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    if (Relay_Failure_counter == 0) {
      Web_server_client.println(F("{\"status\":\"OK\","));
    } else {
      Web_server_client.println(F("{\"status\":\"Unhealthy\","));
    }
    Web_server_client.println("\"arduino_ide_version\":\"" + String(ARDUINO, DEC) + "\",");
    Web_server_client.println("\"arduino_cpp_compiler\":\"" + String(__cplusplus) + "\",");
    Web_server_client.println("\"esp_version\":\"" + ESP.getFullVersion() + "\",");
    Web_server_client.println("\"app_version\":\"" + String(FPSTR(Version_String)) + "\",");
    Web_server_client.println(F("\"compiled\":\""__DATE__
                                ", "__TIME__
                                ", "__VERSION__
                                "\","));
    Web_server_client.println("\"uptime_hours\":\"" + String(millis() / 1000 / 3600) + "\",");
    Web_server_client.println("\"ntp_time\":\"" + String(timeClient.getFormattedDate()) + "\",");
    Web_server_client.println("\"capacity_utilization\":\"" + String(last_processing_time / 1000) + "s+" + String(last_recurring_tasks_processing_time / 1000) + "s/" + String(processing_interval / 1000) + "s\",");
    Web_server_client.println("\"free_heap_bytes\":\"" + String(Free_Heap) + " / 81920\",");
    Web_server_client.println("\"relay_failures\":\"" + String(Relay_Failure_counter) + "\",");
    Web_server_client.println("\"alarms\":\"" + String(Alarms_counter) + "\",");
    Web_server_client.println("\"temperature\":\"" + String(temperature) + "\",");
    Web_server_client.println("\"relay_state\":\"" + String(Central_Heating_State) + "\",");
    Web_server_client.println("\"operation_mode\":\"" + String(heating_auto_switch) + "\",");
    Web_server_client.println("\"latest_heating_start\":\"" + heating_start_NTP_timestamp + "\",");
    Web_server_client.print("\"latest_heating_auto_start\":\"" + heating_auto_start_NTP_timestamp + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_External_Components_Status_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested External Components Check through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    ext_components_dns_ping_check();
    Web_server_client.print("{\"Gateway_Online\":\"" + String(Gateway_Online) + "\",");
    Web_server_client.print("\"DNS_Online\":\"" + String(DNS_Online) + "\",");
    Web_server_client.print("\"SMTP_Online\":\"" + String(SMTP_Online) + "\",");
    Web_server_client.print("\"NTP_Online\":\"" + String(NTP_Online) + "\",");
    Web_server_client.print("\"MongoDB_Online\":\"" + String(MongoDB_Online) + "\",");
    Web_server_client.print("\"ESP_FW_Filestore_Online\":\"" + String(ESP_FW_Filestore_Online) + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_History_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Requests History through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.println("{\"inbound_history\":\"" + Inbound_History + "\",");
    Web_server_client.println("\"outbound_history\":\"" + Outbound_History + "\",");
    Web_server_client.println("\"alarms\":\"" + Alarms_History + "\",");
    Web_server_client.print("\"relay_commands\":\"" + Relay_History + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(API_Historical_Temp_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Temp History through API"));
    Web_server_client.print(FPSTR(HTML_200_API_Response_Code_Headers));
    Web_server_client.print("{\"hist_temp\":\"");
    for (byte i = 0; i < 59; i = i + 1) {
      Web_server_client.print(String(Temp_values_circular_buf[i]) + ",");
    }
    Web_server_client.print(String(Temp_values_circular_buf[59]) + "\"}");
    HTTP_Status_Code = "200";
  }

  else if (http_request_url.indexOf(FPSTR(GUI_Read_Memory_Endpoint)) != -1) {
    Serial.println(F("DEBUG: Requested Memory Page"));
    Web_server_client.println(FPSTR(HTML_200_GUI_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Read_Memory_Response_Content));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    read_and_list_memory();
  }

  else {
    Serial.println(F("WARN: Requested Page was not found"));
    Web_server_client.println(FPSTR(HTML_404_Response_Code_Headers));
    Web_server_client.println(FPSTR(HTML_Menu_Response_Head));
    Web_server_client.print(FPSTR(HTML_Not_Found_Response_Content));
    Web_server_client.print(FPSTR(HTML_Response_Bottom_Page));
    HTTP_Status_Code = "404";
  }

  request_response_end_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Response to TCP Client sent"));
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Performance: Total TCP Connection duration : ") + String(request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms");
  Serial.println(F("DEBUG: Performance: Data Communication duration : ") + String(request_response_end_timestamp - tcp_data_stream_arrival_timestamp) + "ms");
  Serial.println(F("DEBUG: Performance: Request Processing duration : ") + String(request_response_end_timestamp - request_process_start_timestamp) + "ms");

  if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
    Serial.println(F("WARN: Slow Response"));
  }

  if (Inbound_History.length() > Inbound_History_max_bytes) {  //limit String size
    Inbound_History.remove(0);                                 // Remove from from index through the end of the string
    Serial.println(F("DEBUG: ") + String(FPSTR(Inbound_purge)));
  }
  Inbound_History = Inbound_History + String(Inbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client.remoteIP().toString() + ":" + Web_server_client.remotePort() + " | " + http_request_url + " | " + HTTP_Status_Code + " | " + (request_response_end_timestamp - tcp_connection_request_arrival_timestamp) + "ms | " + (request_response_end_timestamp - tcp_data_stream_arrival_timestamp) + "ms | " + (request_response_end_timestamp - request_process_start_timestamp) + "ms | ";
  if ((request_response_end_timestamp - tcp_connection_request_arrival_timestamp) > 200) {
    Inbound_History = Inbound_History + F("Slow Response");
  }
  Inbound_History = Inbound_History + "\r\n";
  delay(1);                   //1ms instructed
  Web_server_client.flush();  //Need to test this
  delay(1);
  //Web_server_client.stop();   //To be used only in conjunction with Header "Content-Length: <value>"

  //list_general_system_params();
  processing_time = processing_time + (millis() - tcp_connection_request_arrival_timestamp);
}



void initial_esp_board_setup() {
  Serial.println(FPSTR(Version_String));
  Serial.print(F("DEBUG: Source file : ") + String(__FILE__));
  Serial.print(":");
  Serial.println(String(__LINE__));
  Serial.println("");
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Board : ") + String(ARDUINO_BOARD));
  Serial.print(F("DEBUG: Arduino IDE : "));
  Serial.println(ARDUINO, DEC);
  Serial.println(F("DEBUG: Arduino C++ Compiler : ") + String(__cplusplus));
  Serial.println(F("DEBUG: ESP ") + String(ESP.getFullVersion()));
  Serial.print(F("DEBUG: Compiled: "));
  Serial.print(F(__DATE__));
  Serial.print(F(", "));
  Serial.print(F(__TIME__));
  Serial.print(F(", "));
  Serial.println(F(__VERSION__));
  Serial.print(F("DEBUG: Reset reason : "));
  Serial.println(String(ESP.getResetReason()));
  Serial.print(F("DEBUG: ResetInfo : "));
  Serial.println(String(ESP.getResetInfo()));
  Serial.print(FPSTR(System_Components));
  Serial.print(FPSTR(External_Components));
  Serial.print(FPSTR(Interfaces));
  Serial.print(FPSTR(System_pinout));
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Setting I/O pins"));
  pinMode(Relay1_pin, OUTPUT);
  digitalWrite(Relay1_pin, HIGH);
  pinMode(Relay2_pin, OUTPUT);
  digitalWrite(Relay2_pin, HIGH);
  pinMode(Relay_State_Pin, INPUT);
  Serial.println(F("DEBUG: I/O pins set"));
  flash_test();
}



void lwip_info() {
  Serial.println(F("DEBUG: Heap available for LWIP : ") + String(MEM_SIZE));
  Serial.println(F("DEBUG: LWIP Number of buffers in the pool for incoming and outgoing packets : ") + String(PBUF_POOL_SIZE));
  Serial.println(F("DEBUG: LWIP STATION_IF : ") + String(STATION_IF));
  Serial.println(F("DEBUG: LWIP SOFTAP_IF : ") + String(SOFTAP_IF));
  Serial.println(F("DEBUG: LWIP LWIP_HAVE_LOOPIF : ") + String(LWIP_HAVE_LOOPIF));
  Serial.println(F("DEBUG: LWIP IPV4 : ") + String(LWIP_IPV4));
  Serial.println(F("DEBUG: LWIP IP_FORWARD : ") + String(IP_FORWARD));
  Serial.println(F("DEBUG: LWIP NAPT : ") + String(IP_NAPT));
  Serial.println(F("DEBUG: LWIP IPV6 : ") + String(LWIP_IPV6));
  Serial.println(F("DEBUG: LWIP IPV6_FORWARD : ") + String(LWIP_IPV6_FORWARD));
  Serial.println(F("DEBUG: LWIP ARP : ") + String(LWIP_ARP));
  Serial.println(F("DEBUG: LWIP ARP_TABLE_SIZE : ") + String(ARP_TABLE_SIZE));
  Serial.println(F("DEBUG: LWIP LWIP_DHCP : ") + String(LWIP_DHCP));
  Serial.println(F("DEBUG: LWIP LWIP_AUTOIP : ") + String(LWIP_AUTOIP));
  Serial.println(F("DEBUG: LWIP LWIP_DNS : ") + String(LWIP_DNS));
  Serial.println(F("DEBUG: LWIP LWIP_ICMP(PING) : ") + String(LWIP_ICMP));
  Serial.println(F("DEBUG: LWIP LWIP_IGMP : ") + String(LWIP_IGMP));
  Serial.println(F("DEBUG: LWIP LWIP_RAW : ") + String(LWIP_RAW));
  Serial.println(F("DEBUG: LWIP LWIP_UDP : ") + String(LWIP_UDP));
  Serial.println(F("DEBUG: LWIP LWIP_TCP : ") + String(LWIP_TCP));
  Serial.println(F("DEBUG: LWIP LWIP_STATS : ") + String(LWIP_STATS));
  Serial.println(F("DEBUG: LWIP LWIP_PERF : ") + String(LWIP_PERF));
}



void scan_for_wifi_networks() {
  String Initial_Network_Scan = "";
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: WiFi Networks Scan start..."));
  Serial.println(FPSTR(Separator));
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(false, true);
  Serial.println(F("DEBUG: WiFi Networks Scan done"));
  if (n == 0) {
    Serial.println(F("WARN: Connectivity Failure. No WiFi Networks found"));
    Initial_Network_Scan = Initial_Network_Scan + F("No WiFi Networks found");
  } else {
    Serial.println(F("DEBUG: WiFi Networks found :"));
    Initial_Network_Scan = Initial_Network_Scan + F("WiFi Networks found");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Initial_Network_Scan = Initial_Network_Scan + F("<br>") + (i + 1);
      Serial.print(F(" SSID : "));
      Initial_Network_Scan = Initial_Network_Scan + F(" SSID : ");
      Serial.print(WiFi.SSID(i));
      Initial_Network_Scan = Initial_Network_Scan + WiFi.SSID(i);
      Serial.print(" (");
      Initial_Network_Scan = Initial_Network_Scan + F(", RSSI : ");
      Serial.print(WiFi.RSSI(i));
      Initial_Network_Scan = Initial_Network_Scan + WiFi.RSSI(i);
      Serial.print(F(") Channel : "));
      Serial.print(WiFi.channel(i));
      Serial.print(F(" BSSID : "));
      Serial.print(WiFi.BSSIDstr(i));
      Serial.print(F(" Hidden : "));
      Serial.print(WiFi.isHidden(i));
      Serial.print(F(" Encryption : "));
      Serial.println(WiFi.encryptionType(i));
      delay(10);
    }
    Serial.println(FPSTR(Encryption_Types));

    if (Initial_Network_Scan.indexOf(ssid_prod) != -1) {
      Serial.println(F("DEBUG: ") + String(ssid_prod) + " found");
    } else {
      Serial.println(F("CRIT: No known WiFi networks found"));
      Alarms_counter = Alarms_counter + 1;
      Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + F("s | No known WiFi networks found\r\n");
    }
    Serial.println("");
    Initial_Network_Scan.remove(0);  //clear string
  }
  WiFi.scanDelete();
}



void setup_wifi_networks() {
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Setting Soft-AP Mode..."));
  Serial.println(FPSTR(Separator));
  //WiFi.softAPConfig(SoftAP_static_IP, SoftAP_gateway, SoftAP_subnet);
  Serial.println(WiFi.softAP(SoftAP_SSID, SoftAP_password) ? "Done." : "Failed!");
  Serial.println("DEBUG: Soft-AP SSID : " + String(SoftAP_SSID));
  Serial.println("DEBUG: Soft-AP password : " + String(SoftAP_password));
  Serial.print(F("DEBUG: Soft-AP MAC address : "));
  Serial.println(WiFi.softAPmacAddress());
  Serial.print(F("DEBUG: Soft-AP IP address : "));
  Serial.println(WiFi.softAPIP());
  Serial.print(F("DEBUG: Soft-AP Stations Num : "));
  Serial.println(WiFi.softAPgetStationNum());
  Serial.println();

  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Connecting to WiFi network : "));
  Serial.println(ssid_prod);
  Serial.println(FPSTR(Separator));


  Serial.println(F("DEBUG: Setting Prod DHCP IP, DNS, GW"));
  //WiFi.config(Station_static_ip_prod, Station_dns_prod, Station_gateway_prod);
  WiFi.begin(ssid_prod, password_prod);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\r\nDEBUG: WiFi connected"));
  Serial.println("");
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Wifi Interface Details"));
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Wifi Interface MAC : "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("DEBUG: WiFi Interface IP : "));
  Serial.println(WiFi.localIP());
  Serial.print(F("DEBUG: Hostname : "));
  Serial.println(WiFi.hostname());
  WiFi.printDiag(Serial);
}



void connectivity_tests() {
  remote_db_test();
  Serial.println("");
}



bool ext_components_dns_ping_check() {
  Gateway_Online = false;
  DNS_Online = false;
  SMTP_Online = false;
  NTP_Online = false;
  MongoDB_Online = false;
  ESP_FW_Filestore_Online = false;

  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Checking External System Components"));

  if (remote_host_ping_check(WiFi.gatewayIP().toString().c_str()) == true) {
    Gateway_Online = true;
  }
  if (remote_host_ping_check(WiFi.dnsIP().toString().c_str()) == true) {
    DNS_Online = true;
  }
  if (remote_host_ping_check(SMTP_Server) == true) {
    SMTP_Online = true;
  }
  if (remote_host_ping_check(NTP_Server) == true) {
    NTP_Online = true;
  }
  if (remote_host_ping_check(MongoDB_Server.c_str()) == true) {
    MongoDB_Online = true;
  }
  if (remote_host_ping_check(ESP_FW_Filestore_Server.c_str()) == true) {
    ESP_FW_Filestore_Online = true;
  }

  if ((Gateway_Online == true) && (DNS_Online == true) && (SMTP_Online == true) && (NTP_Online == true) && (MongoDB_Online == true) && (ESP_FW_Filestore_Online == true)) {
    return 0;
  } else {
    return 1;
  }
}



void remote_db_test() {
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Remote Data Base Read Test"));

  Mongo_request_full_payload = F("GET /admin/$cmd/?filter_listDatabases=1&limit=1 HTTP/1.1\r\n\r\n");
  send_mongodb_operation();
  Serial.println("");

  Mongo_request_full_payload = F("GET /iot/$cmd/?filter_count=audit&limit=1 HTTP/1.1\r\n\r\n");
  send_mongodb_operation();
  Serial.println("");

  Mongo_request_full_payload = F("GET /iot/$cmd/?filter_count=historical_temp&limit=1 HTTP/1.1\r\n\r\n");
  send_mongodb_operation();
  Serial.println("");

  //curl --request GET 'http://10.8.0.1:28017/iot/$cmd/?filter_count=audit&limit=1'
  //db.audit.find({"ResetReason":{$nin : ["Exception","Software Watchdog","Hardware Watchdog","External System","Software/System restart","Power On"]}}).sort({"Timestamp_Zulu_GMT":-1})

  Mongo_request_full_payload = F("GET /iot/audit/?filter_ResetReason=Hardware%20Watchdog&limit=-5 HTTP/1.1\r\n\r\n");  //skip=5 can be an option
  send_mongodb_operation();
  Serial.println("");

  Mongo_request_full_payload = F("GET /iot/audit/?filter_ResetReason=Software%20Watchdog&limit=-5 HTTP/1.1\r\n\r\n");  //skip=5 can be an option
  send_mongodb_operation();
  Serial.println("");

  Mongo_request_full_payload = F("GET /iot/audit/?filter_ResetReason=Exception&limit=-5 HTTP/1.1\r\n\r\n");  //skip=5 can be an option
  send_mongodb_operation();
  Serial.println("");

  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Remote Data Base Insert Test"));
  Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"ESP_Started\",\"ResetReason\":\"" + String(ESP.getResetReason()) + "\",\"ResetInfo\":\"" + String(ESP.getResetInfo()) + "\"}";
  Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
  send_mongodb_operation();
}



void mail_startup_message() {
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Sending Startup Message"));
  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + F(". Hostname : ") + WiFi.hostname() + F(". App Version : ") + FPSTR(Version_String) + F(". Arduino C++ Compiler : ") + String(__cplusplus) + F(". ESP") + " " + String(ESP.getFullVersion()) + F(" ESP_Started. ResetReason : ") + String(ESP.getResetReason()) + F(". ResetInfo : ") + String(ESP.getResetInfo()) + F(". Local IP : ") + WiFi.localIP().toString();
  send_mail_now();
}



void start_wifi_tcp_server() {
  Web_server.begin();
  Serial.println(F("DEBUG: TCP Server started LISTENING for Web HTTP requests"));
  Serial.print(F("INFO: Use this local URL : http://"));
  Serial.println(WiFi.localIP());
  Serial.print(F("INFO: Or this softAP URL : http://"));
  Serial.println(WiFi.softAPIP());
}



void list_general_system_params() {
  Free_Heap = ESP.getFreeHeap();
  Central_Heating_State = digitalRead(Relay_State_Pin);
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: General_system_params"));
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Status : "));
  if (Relay_Failure_counter == 0) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("Unhealthy"));
  }
  Serial.print(F("DEBUG: Uptime : "));
  Serial.println(String(millis() / 1000) + " s");
  Serial.println("DEBUG: Time : " + String(timeClient.getFormattedDate()));
  Serial.print(F("DEBUG: Free Heap Mem : "));
  Serial.println(String(Free_Heap) + " / 81920 Bytes");
  Serial.print(F("DEBUG: HeapFragmentation : "));
  Serial.println(String(ESP.getHeapFragmentation()) + " %");
  Serial.print(F("DEBUG: MaxFreeBlockSize : "));
  Serial.println(String(ESP.getMaxFreeBlockSize()));
  Serial.println("DEBUG: Capacity Utilization (us+sy/time) : " + String(last_processing_time / 1000) + "s+" + String(last_recurring_tasks_processing_time / 1000) + "s/" + String(processing_interval / 1000) + "s");
  Serial.println("DEBUG: TPS : " + String(last_minute_http_requests_counter) + " / " + String(processing_interval / 1000) + "s");
  Serial.print(F("DEBUG: Network received bytes : "));
  Serial.println(String(network_incoming_bytes_counter));
  Serial.print(F("DEBUG: Inbound history counter: "));
  Serial.println(Inbound_History_counter);
  Serial.print(F("DEBUG: Inbound_History String length : "));
  Serial.println(String(Inbound_History.length()) + " / " + Inbound_History_max_bytes);
  Serial.print(F("DEBUG: Outbound history counter: "));
  Serial.println(Outbound_History_counter);
  Serial.print(F("DEBUG: Outbound_History String length : "));
  Serial.println(String(Outbound_History.length()) + " / " + Outbound_History_max_bytes);
  Serial.print(F("DEBUG: Relay history counter: "));
  Serial.println(Relay_History_counter);
  Serial.print(F("DEBUG: Relay_History String length : "));
  Serial.println(String(Relay_History.length()) + " / " + Relay_History_max_bytes);
  Serial.print(F("DEBUG: Temp history counter: "));
  Serial.println(read_temp_sensor_counter);
  Serial.print(F("DEBUG: Temp Array length : "));
  Serial.println(String(sizeof(Temp_values_circular_buf)));
  Serial.print(F("DEBUG: Relay failures: "));
  Serial.println(String(Relay_Failure_counter));
  Serial.print(F("DEBUG: Alarms counter: "));
  Serial.println(String(Alarms_counter));
  Serial.print(F("DEBUG: Alarms String length : "));
  Serial.println(String(Alarms_History.length()) + " / " + Alarms_max_bytes);
  Serial.print(F("DEBUG: Sent Mails : "));
  Serial.println(sent_mails_counter);
  Serial.print(F("DEBUG: Inbound history : "));
  Serial.println(Inbound_History);
  Serial.print(F("DEBUG: Outbound history : "));
  Serial.print(Outbound_History);
  Serial.print(F("DEBUG: Relay history : "));
  Serial.print(Relay_History);
  Serial.print(F("DEBUG: Alarms history : "));
  Serial.print(Alarms_History);
  Serial.print(F("INFO: Temp history : "));
  for (byte i = 0; i < 60; i = i + 1) {
    Serial.print(String(Temp_values_circular_buf[i]) + " | ");
  }
  Serial.println("");
  Serial.print(F("INFO: Ambient temperature : "));
  Serial.println(String(temperature) + " C");
  Serial.println("INFO: Relay State : " + String(Central_Heating_State));
  Serial.println("INFO: Operation Mode : " + String(heating_auto_switch));
  Serial.println("INFO: Latest Heating Start Command : " + heating_start_NTP_timestamp);
  Serial.println("INFO: Latest Heating Auto Start Command : " + heating_auto_start_NTP_timestamp);
}



bool relay_start_command() {
  unsigned long relay_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Executing Relay START command"));
  Relay_History_counter++;
  digitalWrite(Relay1_pin, LOW);
  digitalWrite(Relay2_pin, LOW);
  heating_auto_switch = 0;
  heating_auto_start_timestamp_ms = 0;
  heating_start_timestamp = millis();
  heating_start_NTP_timestamp = String(timeClient.getFormattedDate());
  delay(relay_command_delay_ms);
  Central_Heating_State = digitalRead(Relay_State_Pin);  //CITIREA STARII RELEULUI DE CONFIRMARE
  if (Central_Heating_State == HIGH) {
    Serial.println(F("DEBUG: Success"));
    Relay_Status_Code = "OK";
    Serial.println(FPSTR(Email_prep));
    notify_relay_operation("on");
  } else if (Central_Heating_State == LOW) {
    Serial.println(F("CRIT: Relay Failure"));
    Relay_Status_Code = "Fail";
    Relay_Failure_counter++;
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + "Relay Start Fail" + "\r\n";
  }
  unsigned long relay_response_timestamp = millis();
  Serial.println("DEBUG: Performance: Relay command duration : " + String(relay_response_timestamp - relay_request_arrival_timestamp) + "ms");
  Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Start | " + Relay_Status_Code + "\r\n";
  if (Relay_Status_Code == "OK") {
    return 0;
  } else {
    return 1;
  }
}



bool relay_stop_command() {
  unsigned long relay_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Executing Relay STOP command"));
  Relay_History_counter++;
  digitalWrite(Relay1_pin, HIGH);
  digitalWrite(Relay2_pin, HIGH);
  heating_auto_switch = 0;
  delay(relay_command_delay_ms);
  Central_Heating_State = digitalRead(Relay_State_Pin);  //CITIREA STARII RELEULUI DE CONFIRMARE
  if (Central_Heating_State == HIGH) {
    Serial.println(F("CRIT: Relay Failure"));
    Relay_Status_Code = "Fail";
    Relay_Failure_counter++;
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + "Relay Stop Failed" + "\r\n";
  } else if (Central_Heating_State == LOW) {
    Serial.println(F("DEBUG: Success"));
    Relay_Status_Code = "OK";
    Serial.println(FPSTR(Email_prep));
    notify_relay_operation("off");
  }
  unsigned long relay_response_timestamp = millis();
  Serial.println("DEBUG: Performance: Relay command duration : " + String(relay_response_timestamp - relay_request_arrival_timestamp) + "ms");
  Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Stop | " + Relay_Status_Code + "\r\n";
  if (Relay_Status_Code == "OK") {
    return 0;
  } else {
    return 1;
  }
}



bool relay_auto_stop_command() {
  unsigned long relay_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Executing Relay Auto STOP command"));
  Relay_History_counter++;
  digitalWrite(Relay1_pin, HIGH);
  digitalWrite(Relay2_pin, HIGH);
  delay(relay_command_delay_ms);
  Central_Heating_State = digitalRead(Relay_State_Pin);  //CITIREA STARII RELEULUI DE CONFIRMARE
  if (Central_Heating_State == HIGH) {
    Serial.println(F("CRIT: Relay Failure"));
    Relay_Status_Code = "Fail";
    Relay_Failure_counter++;
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + "Relay Auto Stop Failed" + "\r\n";
  } else if (Central_Heating_State == LOW) {
    Serial.println(F("DEBUG: Success"));
    Relay_Status_Code = "OK";
    Serial.println(FPSTR(Email_prep));
    notify_relay_operation("auto_stop");
  }
  unsigned long relay_response_timestamp = millis();
  Serial.println("DEBUG: Performance: Relay command duration : " + String(relay_response_timestamp - relay_request_arrival_timestamp) + "ms");
  Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Auto Stop | " + Relay_Status_Code + "\r\n";
  if (Relay_Status_Code == "OK") {
    return 0;
  } else {
    return 1;
  }
}



bool relay_auto_start_command() {
  unsigned long relay_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Executing Relay Auto START command"));
  Relay_History_counter++;
  digitalWrite(Relay1_pin, LOW);
  digitalWrite(Relay2_pin, LOW);
  delay(relay_command_delay_ms);
  Central_Heating_State = digitalRead(Relay_State_Pin);  //CITIREA STARII RELEULUI DE CONFIRMARE
  if (Central_Heating_State == HIGH) {
    if (heating_auto_start_timestamp_ms == 0) {
      heating_auto_start_timestamp_ms = millis();
      heating_auto_start_NTP_timestamp = String(timeClient.getFormattedDate());
    }
    Serial.println(F("DEBUG: Success"));
    Relay_Status_Code = "OK";
    Serial.println(FPSTR(Email_prep));
    notify_relay_operation("auto_start");
  } else if (Central_Heating_State == LOW) {
    Serial.println(F("CRIT: Relay Failure"));
    Relay_Status_Code = "Fail";
    Relay_Failure_counter++;
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + "Relay Auto Start Failed" + "\r\n";
  }
  unsigned long relay_response_timestamp = millis();
  Serial.println("DEBUG: Performance: Relay command duration : " + String(relay_response_timestamp - relay_request_arrival_timestamp) + "ms");
  Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Auto Start | " + Relay_Status_Code + "\r\n";
  if (Relay_Status_Code == "OK") {
    return 0;
  } else {
    return 1;
  }
}



void read_temp_sensor() {
  read_temp_sensor_counter++;
  sensorValue = analogRead(A0);  //read LM35 sensor
  temperature = (sensorValue / 1023) * powervoltage * 100 - lm_35_temp_calibration;
  if (temperature < temp_warn_value) {
    Serial.println(F("ERROR: Temperature Sensor possible malfunction"));
    Temp_Sens_Err_Counter++;
  }
}



void flash_test() {
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: ESP Flash mem test"));
  Serial.println(FPSTR(Separator));
  unsigned long flash_arrival_timestamp = millis();
  if (ESP.checkFlashCRC() == 1) {
    Serial.println(F("DEBUG: Flash CRC OK"));
  } else {
    Serial.println(F("CRIT: Flash CRC BAD"));
    Flash_CRC_Failure_counter++;
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | Flash CRC BAD\r\n";
  }
  unsigned long flash_response_timestamp = millis();
  Serial.println(F("DEBUG: Performance: Flash command duration : ") + String(flash_response_timestamp - flash_arrival_timestamp) + "ms\r\n");
}



void notify_restart() {

  Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "ESP_Restart" + "\"}";
  Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
  send_mongodb_operation();

  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Requested ESP Restart";
  send_mail_now();
}



void notify_relay_operation(String operation) {
  //on / off / auto_start / auto_stop
  if (operation.indexOf(F("on")) != -1) {
    Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "Relay_Start" + "\"}";
    Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
    send_mongodb_operation();

    mail_to_send_w_delay = true;
    mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Relay Start Success";
  } else if (operation.indexOf(F("off")) != -1) {
    Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "Relay_Stop" + "\"}";
    Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
    send_mongodb_operation();

    mail_to_send_w_delay = true;
    mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Relay Stop Success";
  } else if (operation.indexOf(F("auto_start")) != -1) {
    Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "Relay_Auto_Start" + "\"}";
    Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
    send_mongodb_operation();

    mail_to_send_w_delay = true;
    mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Requested Relay Auto Start command";
  } else if (operation.indexOf(F("auto_stop")) != -1) {
    Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "Relay_Auto_Stop" + "\"}";
    Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
    send_mongodb_operation();

    mail_to_send_w_delay = true;
    mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Requested Relay Auto Stop command";
  } else {
    Serial.println("DEBUG: Invalid operation for notify_relay");
  }
}



void start_auto_heating() {
  Relay_History_counter++;
  Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Start Auto Mode" + "\r\n";

  Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "Auto_Mode_Start" + "\"}";
  Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
  send_mongodb_operation();

  relay_auto_start_command();

  mail_to_send_w_delay = true;
  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " Requested Start Auto Mode command";
}


void temp_sensor_test() {
  Serial.println("");
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Temperature Sensor Test"));
  Serial.println(FPSTR(Separator));
  read_temp_sensor();
  Serial.println(String(temperature) + " C");
}



void free_mem_check() {
  Free_Heap = ESP.getFreeHeap();
  if (Free_Heap < 30000) {
    Serial.println(F("WARN: Low Memory Warning under 30KB out of 80KB"));
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + "Low Memory Warning " + String(Free_Heap / 1024) + "KB\r\n";
  }
}



void strings_expansion_check() {
  if (Alarms_History.length() > Alarms_max_bytes) {  //limit String size
    Alarms_History.remove(0);                        // Remove from from index through the end of the string
    Serial.println("DEBUG: " + String(FPSTR(Alarms_purge)));
  }

  if (Outbound_History.length() > Outbound_History_max_bytes) {  //limit String size
    Outbound_History.remove(0);                                  // Remove from from index through the end of the string
    Serial.println("DEBUG: " + String(FPSTR(Outbound_purge)));
  }

  if (Relay_History.length() > Relay_History_max_bytes) {  //limit String size
    Relay_History.remove(0);                               // Remove from from index through the end of the string
    Serial.println("DEBUG: " + String(FPSTR(Relay_purge)));
  }
}



void ntp_update() {
  if (timeClient.update() == 0) {
    Serial.println(F("WARN: NTP Update Failure"));
    NTP_Err_Counter++;
  } else {
    Serial.println(F("DEBUG: NTP Update OK!"));
  }
}



void ESP_FW_Update() {
  Serial.println(F("DEBUG: Starting ESP FW Update"));
  unsigned long update_arrival_timestamp = millis();
  WiFiClient updateclient_f_server;
  //C:\Users\<user>\AppData\Local\Temp\arduino\sketches\7095592A2D49FD0DC19C715F54AC6A6C\ESP_Wemos_AC_Controller.ino.bin
  t_httpUpdate_return update_result;
  update_result = ESPhttpUpdate.update(updateclient_f_server, ESP_FW_Filestore_Server, ESP_FW_Filestore_Port, Prod_ESP_FW_Filestore_file);

  Serial.println("ESP FW Update result : " + String(ESPhttpUpdate.getLastError()) + " / " + ESPhttpUpdate.getLastErrorString());

  unsigned long update_response_end_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println("DEBUG: Performance: FW Update command duration : " + String(update_response_end_timestamp - update_arrival_timestamp) + "ms\r\n");

  if ((update_response_end_timestamp - update_arrival_timestamp) > 25) {
    Serial.println(F("WARN: Slow Response"));
  }

  if (ESPhttpUpdate.getLastError() != 0) {
    FW_Update_Err_Counter++;
    Outbound_History_counter = Outbound_History_counter + 1;
    Outbound_History = Outbound_History + String(Outbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + String(ESP_FW_Filestore_Server) + ":" + String(ESP_FW_Filestore_Port) + " | FW Update op | " + ESPhttpUpdate.getLastErrorString() + " | " + String(update_response_end_timestamp - update_arrival_timestamp) + "ms | ";
    if ((update_response_end_timestamp - update_arrival_timestamp) > 25) {
      Outbound_History = Outbound_History + F("Slow Response");
    }
    Outbound_History = Outbound_History + "\r\n";
  }

  Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Operation\":\"" + "ESP_FW_Update\",\"Result\":\"" + ESPhttpUpdate.getLastErrorString() + "\"}";
  Mongo_request_full_payload = F("POST /iot/audit/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
  send_mongodb_operation();

  mail_to_send_w_delay = true;
  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " ESP FW Update result : " + ESPhttpUpdate.getLastErrorString();
}



void send_mongodb_operation() {
  if (MongoDB_Operations == false) {
    Serial.println(F("DEBUG: MongoDB Operations deactivated"));
    return;
  }
  unsigned long mongodb_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Starting MongoDB connection"));
  WiFiClient tcp_mongo_client;
  if (tcp_mongo_client.connect(MongoDB_Server, MongoDB_Server_port)) {
    Serial.println(F("DEBUG: TCP connection OK to remote host ") + String(MongoDB_Server) + ":" + String(MongoDB_Server_port));
    tcp_mongo_client.setTimeout(mongo_tcp_stream_read_timeout_ms);  // sets the maximum milliseconds to wait for stream data
    Serial.println(F("DEBUG: Data Stream read timeout_ms : ") + String(tcp_mongo_client.getTimeout()));
    Serial.println(F("DEBUG: Payload : "));
    Serial.println(Mongo_request_full_payload);
    tcp_mongo_client.print(Mongo_request_full_payload);
    Serial.println(F("DEBUG: Response : "));
    while (tcp_mongo_client.connected() || tcp_mongo_client.available()) {
      if (tcp_mongo_client.available()) {
        String line = tcp_mongo_client.readStringUntil('\r');
        Serial.print(line);
        if (line.indexOf(FPSTR(Mongo_Status_Code_Header)) != -1) {
          Mongo_Status_Code = line;
          if (line.indexOf(FPSTR("200")) != -1 || line.indexOf(FPSTR("201")) != -1) {
            //
          } else {
            Mongo_Err_Counter++;
          }
        }
      }
    }
    Serial.println("");
  } else {
    Serial.println(F("WARN: TCP connection Failure to remote MongoDB Server ") + String(MongoDB_Server) + ":" + String(MongoDB_Server_port));
    Mongo_Status_Code = F("TCP Failure");
    Mongo_Err_Counter++;
  }
  delay(1);
  tcp_mongo_client.stop();
  unsigned long mongodb_response_end_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Performance: MongoDB Request duration : ") + String(mongodb_response_end_timestamp - mongodb_request_arrival_timestamp) + "ms");
  if ((mongodb_response_end_timestamp - mongodb_request_arrival_timestamp) > 100) {
    Serial.println(F("WARN: Slow Response"));
  }
  Serial.println(FPSTR(Separator));
  String Mongo_CRUD = Mongo_request_full_payload.substring(0, Mongo_request_full_payload.indexOf(" "));
  Outbound_History_counter = Outbound_History_counter + 1;
  Outbound_History = Outbound_History + String(Outbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + String(MongoDB_Server) + ":" + String(MongoDB_Server_port) + " | MongoDB op " + Mongo_CRUD + " | " + Mongo_Status_Code + " | " + String(mongodb_response_end_timestamp - mongodb_request_arrival_timestamp) + "ms | ";
  if ((mongodb_response_end_timestamp - mongodb_request_arrival_timestamp) > 100) {
    Outbound_History = Outbound_History + F("Slow Response");
  }
  Outbound_History = Outbound_History + "\r\n";
}



void send_mail_now() {  
  if (SMTP_Notifications == false) {
    Serial.println(F("DEBUG: Mail Notifications deactivated"));
    mail_to_send_w_delay = false;
    return;
  }
  unsigned long smtp_request_arrival_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Starting Mail connection"));
  Serial.println(F("DEBUG: Mail content : "));
  Serial.println(mail_content);

  EMailSender::EMailMessage message;
  message.subject = String(FPSTR(email_subject)) + WiFi.hostname();
  message.message = mail_content;

  EMailSender::Response resp = emailSend.send(Send_Mail_to, message);

  Serial.println(String(F("Sending status: ")) + String(resp.status));
  Serial.println(String(F("Sending code: ")) + String(resp.code));
  Serial.println(String(F("Sending desc: ")) + String(resp.desc));

  if (resp.status == true) {
    sent_mails_counter++;
    Serial.println(F("DEBUG: Message number ") + String(sent_mails_counter) + " sent");
    SMTP_Status_Code = F("OK");
  } else {
    Serial.print(F("WARN: Mail message sending Failure: "));
    Serial.println(resp.desc);
    SMTP_Status_Code = resp.desc;
    SMTP_Err_Counter++;
  }

  mail_to_send_w_delay = false;
  unsigned long smtp_response_end_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.println("DEBUG: Performance: SMTP Request duration : " + String(smtp_response_end_timestamp - smtp_request_arrival_timestamp) + "ms");
  if ((smtp_response_end_timestamp - smtp_request_arrival_timestamp) > SMTP_Slow_Resp_ms) {
    Serial.println(F("WARN: Slow Response"));
  }
  Outbound_History_counter = Outbound_History_counter + 1;
  Outbound_History = Outbound_History + String(Outbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + String(SMTP_Server) + ":" + String(SMTP_PORT) + " | SMTP sendmail | " + SMTP_Status_Code + " | " + String(smtp_response_end_timestamp - smtp_request_arrival_timestamp) + "ms | ";
  if ((smtp_response_end_timestamp - smtp_request_arrival_timestamp) > SMTP_Slow_Resp_ms) {
      Outbound_History+= F("Slow Response");
  }
  Outbound_History = Outbound_History + "\r\n";
}



IPAddress remote_host_dns_lookup(const char* remote_host) {
  bool Host_Lookup = false;
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: DNS Lookup"));
  Serial.println(FPSTR(Separator));

  unsigned long measured_arrival_timestamp = millis();
  IPAddress remote_addr;
  if (WiFi.hostByName(remote_host, remote_addr)) {
    Host_Lookup = true;
    Serial.println("DEBUG: DNS Resolution : " + String(remote_host) + " : " + String(remote_addr[0]) + "." + String(remote_addr[1]) + "." + String(remote_addr[2]) + "." + String(remote_addr[3]));
  } else {
    Serial.println(F("WARN: DNS Resolution Failed"));
    Dns_Query_Err_Counter++;
    Outbound_History_counter = Outbound_History_counter + 1;
    Outbound_History = Outbound_History + String(Outbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + remote_addr.toString() + " | DNS Resolution Failed | |\r\n";
  }
  unsigned long measured_response_timestamp = millis();
  Serial.println("DEBUG: Performance: DNS resolution duration : " + String(measured_response_timestamp - measured_arrival_timestamp) + "ms");
  if ((measured_response_timestamp - measured_arrival_timestamp) > 10) {
    Serial.println(F("WARN: Slow Response"));
  }
  if (Host_Lookup == true) {
    return remote_addr;
  } else {
    return false;
  }
}



bool remote_host_ping_check(const char* remote_host) {
  remote_host_dns_lookup(remote_host);

  bool Host_Ping = false;
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: Host Ping"));
  Serial.println(FPSTR(Separator));

  unsigned long measured_arrival_timestamp = millis();
  if (Ping.ping(remote_host, 1)) {
    Serial.println("DEBUG: " + String(remote_host) + " Ping OK!");
    Host_Ping = true;
  } else {
    Serial.print(F("WARN: Ping Failure for Host : "));
    Serial.println(String(remote_host));
    Ping_Err_Counter++;
    Outbound_History_counter = Outbound_History_counter + 1;
    Outbound_History = Outbound_History + String(Outbound_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + String(remote_host) + " | Host Ping Failure | |\r\n";
  }
  unsigned long measured_response_timestamp = millis();
  Serial.println("DEBUG: Performance: Ping duration : " + String(measured_response_timestamp - measured_arrival_timestamp) + "ms");
  if ((measured_response_timestamp - measured_arrival_timestamp) > 20) {
    Serial.println(F("WARN: Slow Response"));
  }
  if (Host_Ping == true) {
    return true;
  } else {
    return false;
  }
}



void recurring_task_send_mail() {
  recurring_tasks_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Handling recurring Task Send Mail w delay at "));
  Serial.println(String(millis() / 1000) + "s");
  if (Web_server.hasClient()) {
    Serial.println(F("WARN: Another TCP client is waiting in the background while handling mail sending"));
    //WiFiClient Web_server_client_background = Web_server.available();
    //String http_request_url_background = Web_server_client_background.readStringUntil('\r');
    //Serial.print(F("DEBUG: http_request_url String length : "));
    //Serial.println(http_request_url_background.length());
    //Serial.println(http_request_url_background);
    //Web_server_client_background.print(FPSTR(HTML_503_Response_Code_Headers));
    //String HTTP_Status_Code = "503";
    //Inbound_History_counter =  Inbound_History_counter + 1;
    //Inbound_History = Inbound_History + String(Inbound_History_counter,DEC) + " | " + millis()/1000 + "s | " + String(timeClient.getFormattedDate()) + " | " + Web_server_client_background.remoteIP().toString() + ":" + Web_server_client_background.remotePort() + " | " + http_request_url_background + " | " + HTTP_Status_Code + " | " + "ms | " + "ms | " + "ms" + "\r\n";
  }
  send_mail_now();
  recurring_tasks_processing_time = recurring_tasks_processing_time + (millis() - recurring_tasks_timestamp);
}



void recurring_task_1() {
  recurring_tasks_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Handling recurring Task-1 at "));
  Serial.println(String(millis() / 1000) + "s");

  last_processing_start_interval = processing_start_interval;
  processing_start_interval = millis();
  processing_interval = processing_start_interval - last_processing_start_interval;
  last_processing_time = processing_time;
  processing_time = 0;
  last_minute_http_requests_counter = minute_http_requests_counter;
  minute_http_requests_counter = 0;
  last_recurring_tasks_processing_time = recurring_tasks_processing_time;
  recurring_tasks_processing_time = 0;
  Serial.print(F("DEBUG: Capacity Utilization (us+sy/time) : "));
  Serial.println(String(last_processing_time / 1000) + "s+" + String(last_recurring_tasks_processing_time / 1000) + "s/" + String(processing_interval / 1000) + "s");
  Serial.println(F("DEBUG: TPS : ") + String(last_minute_http_requests_counter) + " / " + String(processing_interval / 1000) + "s");

  free_mem_check();
  strings_expansion_check();
  ntp_update();
  read_temp_sensor();

  Central_Heating_State = digitalRead(Relay_State_Pin);  //CITIREA STARII RELEULUI DE CONFIRMARE

  if (Central_Heating_State == HIGH && temperature < temp_warn_value) {
    Serial.println(F("ERROR: Temperature Sensor possible malfunction. Stopping Heating System"));
    Alarms_counter = Alarms_counter + 1;
    Alarms_History = Alarms_History + String(Alarms_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " | Temperature Sensor malfunction. Stopping Heating System.\r\n";
    relay_stop_command();
  }

  if (heating_auto_switch == 1) {
    Serial.println(FPSTR(Separator));
    Serial.println(F("DEBUG: Handling Heating Auto Switch is On"));

    if ((millis() - heating_auto_start_timestamp_ms) > heating_auto_timeout_ms) {
      Serial.println("DEBUG: " + String(FPSTR(Heating_auto_timeout_reached)));
      Relay_History_counter++;
      Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " |  " + String(FPSTR(Heating_auto_timeout_reached)) + "\r\n";
      heating_auto_switch = 0;
      heating_auto_start_timestamp_ms = 0;
      relay_auto_stop_command();
      return;
    }

    if (Central_Heating_State == LOW && temperature < heating_auto_min_temp) {
      Serial.println("DEBUG: " + String(FPSTR(Heating_auto_min_temp_reached)));
      Relay_History_counter++;
      Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " |  " + String(FPSTR(Heating_auto_min_temp_reached)) + "\r\n";
      relay_auto_start_command();
    }

    if (Central_Heating_State == HIGH && temperature > heating_auto_max_temp) {
      Serial.println("DEBUG: " + String(FPSTR(Heating_auto_max_temp_reached)));
      Relay_History_counter++;
      Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " |  " + String(FPSTR(Heating_auto_max_temp_reached)) + "\r\n";
      relay_auto_stop_command();
    }

  } else if (heating_auto_switch == 0 && Central_Heating_State == HIGH) {
    if ((millis() - heating_start_timestamp) > heating_timeout_ms) {
      Serial.println("DEBUG: " + String(FPSTR(Heating_timeout_reached)));
      Relay_History_counter++;
      Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " |  " + String(FPSTR(Heating_timeout_reached)) + "\r\n";
      relay_stop_command();
    }
    if (temperature > heating_max_temp) {
      Serial.println("DEBUG: " + String(FPSTR(Heating_max_temp_reached)));
      Relay_History_counter++;
      Relay_History = Relay_History + String(Relay_History_counter, DEC) + " | " + millis() / 1000 + "s | " + String(timeClient.getFormattedDate()) + " |  " + String(FPSTR(Heating_max_temp_reached)) + "\r\n";
      relay_stop_command();
    }
  }
  delay(1);  //to avoid executing multiple times this loop
  Serial.println("DEBUG: Performance: Task-1 duration : " + String(millis() - recurring_tasks_timestamp) + "ms");
  recurring_tasks_processing_time = recurring_tasks_processing_time + (millis() - recurring_tasks_timestamp);
}



void recurring_task_2() {
  recurring_tasks_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Handling recurring Task-2 at "));
  Serial.println(String(millis() / 1000) + "s");

  read_temp_sensor();
  for (byte i = 0; i < 59; i = i + 1) {
    Temp_values_circular_buf[i] = Temp_values_circular_buf[i + 1];
  }
  Temp_values_circular_buf[59] = temperature;

  Mongo_request_body_Json = F("{\"Timestamp_Zulu_GMT\":\"") + String(timeClient.getFormattedDate()) + "\",\"Hostname\":\"" + WiFi.hostname() + "\",\"Temp_Celsius\":\"" + String(temperature) + "\"}";
  Mongo_request_full_payload = F("POST /iot/historical_temp/ HTTP/1.1\r\nContent-Length: ") + String(Mongo_request_body_Json.length()) + "\r\n\r\n" + Mongo_request_body_Json;
  send_mongodb_operation();

  delay(1);  //to avoid executing multiple times this loop
  Serial.println("DEBUG: Performance: Task-2 duration : " + String(millis() - recurring_tasks_timestamp) + "ms");
  recurring_tasks_processing_time = recurring_tasks_processing_time + (millis() - recurring_tasks_timestamp);
}


void recurring_task_3() {
  recurring_tasks_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Handling recurring Task-3 at "));
  Serial.println(String(millis() / 1000) + "s");

  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + F(" Hostname : ") + WiFi.hostname() + F(" Aliveness Notification. Uptime : ") + String(millis() / 1000 / 3600) + F("h");
  send_mail_now();

  delay(1);  //to avoid executing multiple times this loop
  Serial.println("DEBUG: Performance: Task-3 duration : " + String(millis() - recurring_tasks_timestamp) + "ms");
  recurring_tasks_processing_time = recurring_tasks_processing_time + (millis() - recurring_tasks_timestamp);
}


void recurring_task_4() {
  recurring_tasks_timestamp = millis();
  Serial.println(FPSTR(Separator));
  Serial.print(F("DEBUG: Handling recurring Task-4 at "));
  Serial.println(String(millis() / 1000) + "s");

  mail_content = F("Timestamp : ") + String(timeClient.getFormattedDate()) + " Hostname : " + WiFi.hostname() + " ESP Auo Update Initiated";
  send_mail_now();
  ESP_FW_Update();

  delay(1);  //to avoid executing multiple times this loop
  Serial.println("DEBUG: Performance: Task-4 duration : " + String(millis() - recurring_tasks_timestamp) + "ms");
  recurring_tasks_processing_time = recurring_tasks_processing_time + (millis() - recurring_tasks_timestamp);
}



String render_html_form_server_config(String form_action, String form_method, String form_label_name, String server, int port) {
  String rendered_form = "";
  rendered_form = F("<form action=\"") + String(form_action) + F("\" method=\"") + String(form_method) + F("\"><label>") + String(form_label_name) + F(": </label>");
  rendered_form = rendered_form + F("<input type=\"text\" name=\"") + form_label_name + F("\" value=\"") + String(server) + F(":") + String(port) + F("\" required>");
  rendered_form = rendered_form + F("<button type=\"submit\">Save</button>");
  rendered_form = rendered_form + F("</form>");
  return rendered_form;
}



String render_html_simple_form_bool(String form_action, String form_method, String form_label_name, bool form_parameter) {
  String rendered_form = "";
  rendered_form = F("<form action=\"") + String(form_action) + F("\" method=\"") + String(form_method) + F("\"><label>") + String(form_label_name) + F(": </label>");
  rendered_form = rendered_form + F("<input type=\"text\" name=\"") + form_label_name + F("\" value=\"") + String(form_parameter) + F("\" required>");
  rendered_form = rendered_form + F("<button type=\"submit\">Save</button>");
  rendered_form = rendered_form + F("</form>");
  return rendered_form;
}



String render_html_simple_form_string(String form_action, String form_method, String form_label_name, String form_parameter, String button_text) {
  String rendered_form = "";
  rendered_form = F("<form action=\"") + String(form_action) + F("\" method=\"") + String(form_method) + F("\"><label>") + String(form_label_name) + F(": </label>");
  rendered_form = rendered_form + F("<input type=\"text\" name=\"") + form_label_name + F("\" value=\"") + String(form_parameter) + F("\" required>");
  rendered_form = rendered_form + F("<button type=\"submit\">") + button_text + F("</button>");
  rendered_form = rendered_form + F("</form>");
  return rendered_form;
}



String render_html_simple_6_select_string(String form_action, String form_method, String form_label_name, String form_parameter_1, String form_parameter_2, String form_parameter_3, String form_parameter_4, String form_parameter_5, String form_parameter_6, String button_text) {
  String rendered_form = "";
  rendered_form = F("<form action=\"") + String(form_action) + F("\" method=\"") + String(form_method) + F("\"><label>") + String(form_label_name) + F("</label>");
  rendered_form = rendered_form + F("<select name=\"") + String(form_label_name) + F("\" id=\"") + String(form_label_name) + F("\">");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_1) + F("\">") + String(form_parameter_1) + F("</option>");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_2) + F("\">") + String(form_parameter_2) + F("</option>");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_3) + F("\">") + String(form_parameter_3) + F("</option>");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_4) + F("\">") + String(form_parameter_4) + F("</option>");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_5) + F("\">") + String(form_parameter_5) + F("</option>");
  rendered_form = rendered_form + F("<option value=\"") + String(form_parameter_6) + F("\">") + String(form_parameter_6) + F("</option>");
  rendered_form = rendered_form + F("</select>");
  rendered_form = rendered_form + F("<button type=\"submit\">") + button_text + F("</button>");
  rendered_form = rendered_form + F("</form>");
  return rendered_form;
}



bool connection_test(const char* hostname, int port, bool connection) {
  unsigned long test_tcp_client_request_arrival_timestamp = 0;
  unsigned long test_tcp_client_response_timestamp = 0;
  WiFiClient tcp_test_client;
  Serial.println(FPSTR(Separator));
  test_tcp_client_request_arrival_timestamp = millis();
  if (tcp_test_client.connect(hostname, port)) {
    Serial.println(F("DEBUG: TCP connection test OK to ") + String(hostname) + ":" + String(port));
    connection = true;
  } else {
    Serial.println(F("WARN: TCP connection test Failure to ") + String(hostname) + ":" + String(port));
    connection = false;
  }
  delay(1);
  tcp_test_client.stop();
  test_tcp_client_response_timestamp = millis();
  Serial.println(F("DEBUG: Performance: Connection duration : ") + String(test_tcp_client_response_timestamp - test_tcp_client_request_arrival_timestamp) + "ms");
  if (connection == false) {
    TCP_Connection_Err_Counter++;
    return false;
  } else {
    return true;
  }
}



String IP_String_to_IPAddress_w_arp_ping(String ipstr) {
  IPAddress ip_request_addr;
  struct eth_addr* returned_mac;
  const ip4_addr_t* returned_ipaddr;  //ip4_addr_t or ip_addr_t
  String Results_buffer = "";
  if (ip_request_addr.fromString(ipstr)) {
    Results_buffer = String(ip_request_addr[0]) + "." + String(ip_request_addr[1]) + "." + String(ip_request_addr[2]) + "." + String(ip_request_addr[3]);
    if (etharp_find_addr(NULL, ip_request_addr, &returned_mac, &returned_ipaddr) != -1) {
      Results_buffer = Results_buffer + F(" ") + String(returned_mac->addr[0], HEX) + ":" + String(returned_mac->addr[1], HEX) + ":" + String(returned_mac->addr[2], HEX) + ":" + String(returned_mac->addr[3], HEX) + ":" + String(returned_mac->addr[4], HEX) + ":" + String(returned_mac->addr[5], HEX) + "<br>ARP_Ping OK";
    } else {
      Results_buffer = Results_buffer + F("<br>No MAC returned");
      ARP_Ping_Err_Counter++;
    }
  } else {
    Results_buffer = ipstr + F("<br>Unparsable IP");
  }
  return Results_buffer;
}


String performReverseDNS(String ipstr, IPAddress rdns_Server, int rdns_Port) {
  Serial.println(FPSTR(Separator));
  Serial.println(F("DEBUG: ReverseDNS"));
  Serial.println(FPSTR(Separator));

  byte dnsPacket[512];  // DNS packet buffer
  IPAddress ip;
  ip.fromString(ipstr);
  int packetSize = buildRDNSQuery(dnsPacket, ip);

  // Send the UDP packet to the DNS server
  if (!rDNSUDP.begin(rdns_Port)) {
    Serial.println("Failed to start rDNS UDP");
  }
  rDNSUDP.beginPacket(rdns_Server, rdns_Port);
  rDNSUDP.write(dnsPacket, packetSize);
  rDNSUDP.endPacket();

  // Print sent DNS packet for debugging
  Serial.println(F("Sent DNS Packet:"));
  for (int i = 0; i < packetSize; i++) {
    Serial.print(dnsPacket[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Wait for a response
  delay(1000);

  // Read response
  int size = rDNSUDP.parsePacket();
  if (size > 0) {
    rDNSUDP.read(dnsPacket, 512);

    // Print received DNS packet for debugging
    Serial.println(F("Received DNS Response:"));
    for (int i = 0; i < size; i++) {
      Serial.print(dnsPacket[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    String domainName = parseDNSResponse(dnsPacket, size);
    Serial.println(F("Resolved Domain Name: ") + domainName);
    return domainName;
  } else {
    Serial.println(F("No response received."));
    return "No response received.";
  }
}

// Function to build a reverse DNS query (PTR query)
int buildRDNSQuery(byte* buffer, IPAddress ip) {
  buffer[0] = 0x00;  // Transaction ID (random)
  buffer[1] = 0x01;
  buffer[2] = 0x01;  // Flags: recursion desired
  buffer[3] = 0x00;
  buffer[4] = 0x00;  // Questions count
  buffer[5] = 0x01;
  buffer[6] = 0x00;  // Answer RRs
  buffer[7] = 0x00;
  buffer[8] = 0x00;  // Authority RRs
  buffer[9] = 0x00;
  buffer[10] = 0x00;  // Additional RRs
  buffer[11] = 0x00;

  // Reverse IP address and append ".in-addr.arpa"
  int offset = 12;
  offset = appendReversedIP(buffer, offset, ip);
  buffer[offset++] = 7;  // Length of "in-addr"
  buffer[offset++] = 'i';
  buffer[offset++] = 'n';
  buffer[offset++] = '-';
  buffer[offset++] = 'a';
  buffer[offset++] = 'd';
  buffer[offset++] = 'd';
  buffer[offset++] = 'r';
  buffer[offset++] = 4;  // Length of "arpa"
  buffer[offset++] = 'a';
  buffer[offset++] = 'r';
  buffer[offset++] = 'p';
  buffer[offset++] = 'a';
  buffer[offset++] = 0;  // Null terminator for the domain

  // Set the query type to PTR (0x000C) and class to IN (0x0001)
  buffer[offset++] = 0x00;
  buffer[offset++] = 0x0C;  // PTR type
  buffer[offset++] = 0x00;
  buffer[offset++] = 0x01;  // Class IN

  return offset;
}

// Helper function to reverse the IP address and append to DNS query
int appendReversedIP(byte* buffer, int offset, IPAddress ip) {
  buffer[offset++] = String(ip[3]).length();
  for (int i = 0; i < String(ip[3]).length(); i++) {
    buffer[offset++] = String(ip[3])[i];
  }
  buffer[offset++] = String(ip[2]).length();
  for (int i = 0; i < String(ip[2]).length(); i++) {
    buffer[offset++] = String(ip[2])[i];
  }
  buffer[offset++] = String(ip[1]).length();
  for (int i = 0; i < String(ip[1]).length(); i++) {
    buffer[offset++] = String(ip[1])[i];
  }
  buffer[offset++] = String(ip[0]).length();
  for (int i = 0; i < String(ip[0]).length(); i++) {
    buffer[offset++] = String(ip[0])[i];
  }
  return offset;
}

// Parse the DNS response and extract the PTR record (domain name)
String parseDNSResponse(byte* buffer, int size) {
  int answerStart = 12;  // Skip the DNS header and question section
  while (buffer[answerStart] != 0) {
    answerStart++;
  }
  answerStart += 5;  // Skip null byte and question type & class (2 bytes each)

  // Check if it's a PTR record (type 0x000C)
  if (buffer[answerStart + 2] == 0x00 && buffer[answerStart + 3] == 0x0C) {
    int domainStart = answerStart + 12;  // Skip the answer header (12 bytes)
    String domainName = "";

    int length = buffer[domainStart];
    while (length > 0 && domainStart < size) {
      for (int i = 1; i <= length; i++) {
        domainName += char(buffer[domainStart + i]);
      }
      domainStart += length + 1;
      length = buffer[domainStart];
      if (length > 0) domainName += ".";
    }
    return domainName;
  }
  rDNS_Err_Counter++;
  return "Invalid PTR record or not found.";
}


void read_and_list_memory() {
  char mem = 'A';
  char* mem_ptr;
  mem_ptr = &mem;
  Serial.println("+++++++++++++++++");

  while ((uint32)mem_ptr != 0x3FFC0000) {
    mem_ptr++;
  }

  Serial.println((uint32)mem_ptr, HEX);  // address of variable

  while (1 < 2) {
    if ((uint32)mem_ptr % 0x1000 == 0) {
      Serial.println("");
      Serial.println("+++++++++++++++++");
      Serial.println((uint32)mem_ptr, HEX);  // address of variable
    }

    mem_ptr++;

    // Check if pointer is within accessible memory range
    if ((uint32)mem_ptr > 0x3FFFFFFF) {
      Serial.println("");
      Serial.println("Reached an invalid memory region.");
      break;
    }

    Serial.print((char)*mem_ptr);  // value contained at address
    yield();                       // Allow system tasks to run, preventing watchdog reset
  }
}