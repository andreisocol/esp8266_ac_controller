#define VERSION "1.53"
static const char Version_String[] PROGMEM = "Smart Heating System Controller v" VERSION " " __DATE__ " " __TIME__;
#include "Config_Secrets.h"

bool serial_DebugOutput = false;

int Relay1_pin = D6;       //Relay1
int Relay2_pin = D5;       //Relay2
int Relay_State_Pin = D7;  //Relay state
float powervoltage = 3.3;  //define the power supply voltage

const char* SoftAP_SSID = "ESP_SoftAP";
const char* SoftAP_password = "12345678";

IPAddress Station_static_ip_prod(192, 168, 1, 4);
IPAddress Station_dns_prod(192, 168, 1, 1);
IPAddress Station_gateway_prod(192, 168, 1, 1);

bool SMTP_Notifications = true;
bool MongoDB_Operations = true;

int recurring_task_1_ms = 60001;
unsigned long recurring_task_2_ms = 21600002;
uint32_t recurring_task_3_ms = 604800005;
uint32_t recurring_task_4_ms = 2592000003;

int relay_command_delay_ms = 200;
unsigned long heating_timeout_ms = 28800000;       ///PROD - 8h, 21600000ms / TEST - 3min, 180000ms
unsigned long heating_auto_timeout_ms = 28800000;  ///PROD - 8h, 28800000ms / TEST - 3min, 180000ms
int heating_max_temp = 25.5;                       //Value in Celsius degrees
int heating_auto_min_temp = 22;
int heating_auto_max_temp = 25.5;
int temp_warn_value = 0;
float lm_35_temp_calibration = 3.5;  // Celsius degrees to adjust for calibration of lm35 sensor : decrease x degrees

int Inbound_History_max_bytes = 2200;
int Outbound_History_max_bytes = 1100;
int Relay_History_max_bytes = 1000;
int Alarms_max_bytes = 1000;

int inbound_tcp_data_stream_timeout_ms = 1000;  ///5000 used value
int mongo_tcp_stream_read_timeout_ms = 10;            // max milliseconds to wait for stream data ; for Mongo it was 100ms
int tcp_buffer_overflow_threshold = 4096;

uint32_t Free_Heap = 0;
int Central_Heating_State;
int heating_auto_switch = 0;
float sensorValue;
float temperature;
float Temp_values_circular_buf[60];

long Relay_History_counter = 0;
String Relay_History = "";
long Flash_CRC_Failure_counter = 0;
long Relay_Failure_counter = 0;
long Inbound_History_counter = 0;
long Inbound_History_dropped_req_counter = 0;
String Inbound_History = "";
long Outbound_History_counter = 0;
String Outbound_History = "";
long Alarms_counter = 0;
String Alarms_History = "";
long read_temp_sensor_counter = 0;
long network_incoming_bytes_counter = 0;

bool Gateway_Online = false;
bool DNS_Online = false;
bool SMTP_Online = false;
bool NTP_Online = false;
bool MongoDB_Online = false;
bool ESP_FW_Filestore_Online = false;

long Dns_Query_Err_Counter = 0;
long Ping_Err_Counter = 0;
long ARP_Ping_Err_Counter = 0;
long External_IP_Err_Counter = 0;
long WOL_Err_Counter = 0;
long rDNS_Err_Counter = 0;
long TCP_Connection_Err_Counter = 0;
long SMTP_Err_Counter = 0;
long Mongo_Err_Counter = 0;
long FW_Update_Err_Counter = 0;
long NTP_Err_Counter = 0;
long Temp_Sens_Err_Counter = 0;

unsigned long processing_start_interval = 0;
unsigned long last_processing_start_interval = 0;
unsigned long processing_interval = 0;
unsigned long processing_time = 0;
unsigned long last_processing_time = 0;
int minute_http_requests_counter = 0;
int last_minute_http_requests_counter = 0;
unsigned long recurring_tasks_timestamp = 0;
unsigned long recurring_tasks_processing_time = 0;
unsigned long last_recurring_tasks_processing_time = 0;
unsigned long tcp_connection_request_arrival_timestamp;
unsigned long tcp_data_stream_arrival_timestamp;
unsigned long request_response_end_timestamp;
unsigned long request_process_start_timestamp;
unsigned long heating_start_timestamp;
unsigned long heating_auto_start_timestamp_ms;
String heating_start_NTP_timestamp;
String heating_auto_start_NTP_timestamp;
String Relay_Status_Code = "";

String MongoDB_Server = "10.8.0.1";  //IP or hostname
int MongoDB_Server_port = 28017;
String Mongo_request_full_payload;
String Mongo_request_body_Json;
String Mongo_Status_Code;
static const char Mongo_Status_Code_Header[] PROGMEM = "HTTP/";

String ESP_FW_Filestore_Server = "10.8.0.1";
int ESP_FW_Filestore_Port = 8080;
String Prod_ESP_FW_Filestore_file = "/ESP_Wemos_AC_Controller.ino.bin";

const char* SMTP_Server = "smtp.gmail.com";  //Value coming from EMailSender
const int SMTP_PORT = 465;                   //Value coming from EMailSender
static const char email_subject[] PROGMEM = "AC Controller message ";
String SMTP_Status_Code;
int SMTP_Slow_Resp_ms = 5000;

int TimeOffset = 7200;
const char* NTP_Server = "pool.ntp.org";

String mail_content;
bool mail_to_send_w_delay = false;
int sent_mails_counter = 0;