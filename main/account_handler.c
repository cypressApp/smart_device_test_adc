#include "context.h"

struct account_struct account_struct_list[100];
struct mqtt_response_struct mqtt_response_list[1024];

int   account_list_size = 0;
int   response_id = 10;
int   mqtt_response_counter = 0; 
bool  send_mqtt_message = false;

void update_response_id(){
    response_id++;
    if(response_id >= 255){
        response_id = 10;
    }
}

void remove_account(int sock) {
    
    int temp_index = -1;

    for(int i = 0 ; i < account_list_size ; i++){
        if(account_struct_list[i].sock == sock){
            temp_index = i;
            break;
        }
    }

    if(temp_index >= 0){
        for (int i = temp_index; i < account_list_size - 1; i++) {
            account_struct_list[i] = account_struct_list[i + 1];
        }
        account_list_size--;
    }

    
}

void add_account(int sock , int temp_ip4){
    
    if(account_list_size >= 100) return;

    account_struct_list[account_list_size].sock = sock;
    account_struct_list[account_list_size].ip4 = temp_ip4;
    account_list_size++;
}

void send_data_to_clients(int sock , char *data , int data_size , MQTTQoS_t mqttQos , bool send_local , bool send_remote ){

    update_response_id();
    int response_size = sprintf(mqtt_response_list[mqtt_response_counter].data, "%d:%s", response_id , data);
    mqtt_response_list[mqtt_response_counter].mqttQos = mqttQos;

    if(sock == SEND_TO_ALL){
        if(send_local){
            for(int i = 0 ; i < account_list_size ; i++){
                send(account_struct_list[i].sock, mqtt_response_list[mqtt_response_counter].data, response_size, 0);
            }            
        }
#if IS_REMOTE_CON_ENABLE == 1   
        if(send_remote){
            send_mqtt_message = true;
        }     
#endif        
    }else{
        if(send_local){
            send(sock, mqtt_response_list[mqtt_response_counter].data, response_size, 0);
        }
        
    }
#if IS_REMOTE_CON_ENABLE == 1      
    if(send_remote){
        mqtt_response_counter++;
        if(mqtt_response_counter >= 1023){
            mqtt_response_counter = 0;
        }
    }
#endif

}

