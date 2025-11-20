# Project Title
ESP8266 Smart AC Controller

## Test:
TARGET_IP="192.168.1.4"
watch -n 0 "curl $TARGET_IP/; curl $TARGET_IP/alarms; curl $TARGET_IP/historical_temp; curl $TARGET_IP/history; curl $TARGET_IP/admin; curl $TARGET_IP/config; curl $TARGET_IP/sys_params; curl $TARGET_IP/api; curl $TARGET_IP/api/status; curl $TARGET_IP/api/ext_comp_status; curl $TARGET_IP/api/refresh; curl $TARGET_IP/api/alarms; curl $TARGET_IP/api/historical_temp; curl $TARGET_IP/api/refresh; curl $TARGET_IP/api/history;"
