#include "context.h"

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t         adc_cali_handle = NULL;
bool                      do_calibration  = false;

int  new_measured_voltage  = 0;
int  last_measured_voltage = 0;
bool adc_value_changed     = false;

int  adc_data_list[ADC_BUFFER_SIZE];
int  adc_data_counter = 0; 
int  adc_remote_send_counter = 0;

SemaphoreHandle_t adc_data_mutex;

void init_analog_measurement_handler(){

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_MEASUREMENT_UNIT,
        // .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_MEASUREMENT_CH, &config));

    adc_cali_handle = NULL;
    do_calibration  = adc_calibration_init(ADC_MEASUREMENT_UNIT, ADC_ATTEN_DB_11, &adc_cali_handle);

}

void start_analog_measuring(void *pvParameter){
    
    init_analog_measurement_handler();
    char temp_measured_string[ADC_DATA_MAX_SIZE];
    while (1) {
        
        measure_analog_input(ADC_MEASUREMENT_CH , &new_measured_voltage , &last_measured_voltage , &adc_value_changed, NUMBER_OF_SAMPLES);
        if(adc_value_changed){
            int len = sprintf(temp_measured_string , "%s%d%s" , ADC_DATA_PREFIX , new_measured_voltage , ADC_DATA_SUFFIX);
            if(xSemaphoreTake(adc_data_mutex, 0) == pdTRUE){
#if IS_ADC_BUFFER_ENABLE == 1                
                for(int i = 0 ; i < adc_data_counter ; i++){
                    char temp_measured_string2[ADC_DATA_MAX_SIZE];
                    int len2 = sprintf(temp_measured_string2 , "%s%d%s" , ADC_DATA_PREFIX , adc_data_list[i] , ADC_DATA_SUFFIX);
                    adc_remote_send_counter++;
                    if(adc_remote_send_counter == 10){
                        send_data_to_clients(SEND_TO_ALL, temp_measured_string, len, MQTTQoS0 , true , true);
                        adc_remote_send_counter = 0;
                    }else{
                        send_data_to_clients(SEND_TO_ALL, temp_measured_string, len, MQTTQoS0 , true , false);
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                adc_data_counter = 0;
#endif                
                adc_remote_send_counter++;
                if(adc_remote_send_counter == 10){
                    send_data_to_clients(SEND_TO_ALL, temp_measured_string, len, MQTTQoS0 , true , true);
                    adc_remote_send_counter = 0;
                }else{
                    send_data_to_clients(SEND_TO_ALL, temp_measured_string, len, MQTTQoS0 , true , false);
                }
                
                xSemaphoreGive(adc_data_mutex);
            }
#if IS_ADC_BUFFER_ENABLE == 1                
            else{
                if(adc_data_counter < ADC_BUFFER_SIZE - 1){
                    adc_data_list[adc_data_counter] = new_measured_voltage;
                    adc_data_counter++;
                }
            }
#endif            
        }
        vTaskDelay(pdMS_TO_TICKS(100));

    }

    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
    if (do_calibration) {
        adc_calibration_deinit(adc_cali_handle);
    }

    vTaskDelete(NULL);

}

void measure_analog_input(adc_channel_t adc_channel , int *new_value , int *last_value , bool *value_changed , int number_of_sample){

    int temp_measured_value = 0;
    int temp_raw_measured_value = 0;
    long int temp_measured_average = 0;
    
    if (do_calibration) {
        for (int i = 0; i < number_of_sample; i++) {

            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, &temp_raw_measured_value));             
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, temp_raw_measured_value, &temp_measured_value));
            temp_measured_average += temp_measured_value;

        }
        temp_measured_average /= number_of_sample;
        // printf("1: %d   %ld \n" , temp_raw_measured_value , temp_measured_average);
    }else{
        for (int i = 0; i < number_of_sample; i++) {

            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, &temp_raw_measured_value));
            temp_measured_average += temp_raw_measured_value;

        }            
        temp_measured_average = ( (temp_measured_average * 3140) / 4096) / number_of_sample;
        // printf("2: %d   %ld \n" , temp_raw_measured_value , temp_measured_average);
    }

    *new_value = (int) temp_measured_average;       
    if(!*value_changed){
        if(*last_value != *new_value){
            *value_changed = true;
        }
    }else{
        if(*last_value == *new_value){
            *value_changed = false;
        }        
    }
    *last_value = *new_value;

}

bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle){

    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
    
}

void adc_calibration_deinit(adc_cali_handle_t handle){

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif

}