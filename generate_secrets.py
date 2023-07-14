import os

with open('.env', 'r') as f:
    env_lines = f.readlines()

env_vars = {}
for line in env_lines:
    key, value = line.strip().split('=')
    env_vars[key] = value

ssid = env_vars.get('SSID', '')
wifi_password = env_vars.get('WIFI_PASSWORD', '')
keypad_password = env_vars.get('KEYPAD_PASSWORD', '')
sinric_app_key = env_vars.get('SINRIC_APP_KEY', '')
sinric_app_secret = env_vars.get('SINRIC_APP_SECRET', '')
sinric_garagedoor_id = env_vars.get('SINRIC_GARAGEDOOR_ID', '')
remote_syslog = env_vars.get('REMOTE_SYSLOG')

with open('src\\Secrets.h', 'w') as f:
    f.write(f'#ifndef SECRETS_H\n')
    f.write(f'#define SECRETS_H\n\n')
    f.write(f'\n')
    f.write(f'const char* REMOTE_SYSLOG = "{remote_syslog}";\n')
    f.write(f'\n')
    f.write(f'const char* ssid = "{ssid}";\n')
    f.write(f'const char* keypadPassword = "{keypad_password}";\n')    
    f.write(f'const char* wifiPassword = "{wifi_password}";\n')    
    f.write(f'const char* sinricAppKey = "{sinric_app_key}";\n')
    f.write(f'const char* sinricAppSecret = "{sinric_app_secret}";\n')
    f.write(f'const char* sinricDeviceID = "{sinric_garagedoor_id}";\n')

    f.write(f'\n')
    f.write(f'#endif // SECRETS_H\n')