
#ifndef ACCOUNT_STRUCT
#define ACCOUNT_STRUCT

typedef struct account_struct {
  int  sock;
  int  ip4; 
} account_struct;

typedef struct mqtt_response_struct {
  char data[32];
  MQTTQoS_t mqttQos; 
} mqtt_response_struct;

extern account_struct account_struct_list[100];
extern mqtt_response_struct  mqtt_response_list[1024];
#endif

extern int   account_list_size;
extern int   response_id;

extern int   mqtt_response_counter; 
extern bool  send_mqtt_message;

void update_response_id();
void remove_account(int sock);
void add_account(int sock , int temp_ip4);
void send_data_to_clients(int sock , char *data , int data_size , MQTTQoS_t mqttQos , bool send_local , bool send_remote );