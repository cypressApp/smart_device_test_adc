#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "driver/gpio.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_event.h"
#include "driver/touch_pad.h"
#include <stdlib.h>
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_check.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"

#include "lwip/sockets.h"
#include "esp_netif.h"

#include "aws_iot_handler.h"
#include "AccessPointMode.h"
#include "StationMode.h"
#include "wifi_info_handler.h"
#include "TcpServer.h"
#include "UdpServer.h"
#include "udp_response_handler.h"
#include "constants.h"
#include "GPIO.h"
#include "ota_lib/tcp_ota_handler.h"
#include "ota_lib/ota_handler.h"
#include "ota_lib/flash_boot_handler.h"
#include "checksum_handler.h"
#include "memory_handler/flash_wifi_info_handler.h"
#include "memory_handler/flash_reset_handler.h"
#include "partitions.h"
#include "reset_handler.h"
#include "account_handler.h"
#include "adc_handler.h"
