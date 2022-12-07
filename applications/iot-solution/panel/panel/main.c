#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <cli.h>
#include <ble_cli_cmds.h>

#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <lwip/err.h>
#include <http_client.h>
#include <netutils/netutils.h>

#include <bl602_glb.h>
#include <bl602_hbn.h>
#include <easyflash.h>
#include <bl_sys.h>
#include <bl_uart.h>
#include <bl_chip.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <bl_sec.h>
#include <bl_cks.h>
#include <bl_irq.h>
#include <bl_dma.h>
#include <bl_gpio_cli.h>
#include <bl_wdt_cli.h>
#include <hal_sys.h>
#include <hal_gpio.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <looprt.h>
#include <loopset.h>
#include <sntp.h>
#include <bl_sys_time.h>
#include <bl_sys_ota.h>
#include <bl_romfs.h>
#include <fdt.h>
#include <bl60x_fw_api.h>
#include <wifi_mgmr_ext.h>
#include <utils_log.h>
#include <libfdt.h>
#include <blog.h>
#include <event_groups.h>

#include "lvgl.h"

#include "wifi_interface.h"
#include "blufi.h"
#include "blufi_api.h"
#include "blufi_hal.h"
#include "blufi_init.h"
#include "axk_blufi.h"
#include "ble_interface.h"

extern TaskHandle_t ui_task_handle;

blufi_config_t g_blufi_config = { 0 };

extern lv_obj_t * humidity_value_label;
extern lv_obj_t * temperature_icon_label;
extern lv_obj_t * temperature_value_label;
extern lv_obj_t * room1_button;
extern lv_obj_t * room1_icon;
extern lv_obj_t * room2_button;
extern lv_obj_t * room2_icon;
extern lv_obj_t * people_icon;
extern lv_obj_t * status_value;
extern lv_obj_t * ip_address_value;
extern lv_obj_t * netmask_value;
extern lv_obj_t * gateway_value;
extern lv_obj_t * reset_button;

extern void start_ui_task();

static wifi_conf_t conf =
{
    .country_code = "CN",
};

static char ssid[33];
static char psk[65];

static void _connect_wifi()
{
    if (ef_get_env_blob("SSID", ssid, sizeof ssid, NULL)) {
        ef_get_env_blob("PSK", psk, sizeof psk, NULL);

        wifi_interface_t wifi_interface;
        wifi_interface = wifi_mgmr_sta_enable();
        wifi_mgmr_sta_connect(wifi_interface, ssid, psk, NULL, NULL, 0, 0);
    }
}

static void example_event_callback(_blufi_cb_event_t event, _blufi_cb_param_t* param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    switch (event)
    {
        case AXK_BLUFI_EVENT_INIT_FINISH:
            blog_info("BLUFI init finish");
            axk_blufi_adv_start();
            break;
        case AXK_BLUFI_EVENT_DEINIT_FINISH:
            blog_info("BLUFI deinit finish");
            break;
        case AXK_BLUFI_EVENT_BLE_CONNECT:
            blog_info("BLUFI ble connect");
            axk_blufi_adv_stop();
            // blufi_security_init();
            break;
        case AXK_BLUFI_EVENT_BLE_DISCONNECT:
            blog_info("BLUFI ble disconnect");
            //    blufi_security_deinit();
            // axk_blufi_adv_start();
            axk_blufi_profile_deinit();
            axk_hal_blufi_deinit();
            axk_blufi_adv_stop();

            break;
        case AXK_BLUFI_EVENT_SET_WIFI_OPMODE:
            blog_info("BLUFI Set WIFI opmode %d", param->wifi_mode.op_mode);
            // if (axk_hal_wifi_mode_set(WIFIMODE_STA, 0) != BLUFI_ERR_SUCCESS)
            // {
            //     blog_info("BLUFI axk_hal_wifi_mode_set fail");
            //     break;
            // }
            break;
        case AXK_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        {
            _connect_wifi();
            blog_info("BLUFI requset wifi connect to AP");
        }

        break;
        case AXK_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
            blog_info("BLUFI requset wifi disconnect from AP");
            
            axk_hal_disconn_ap();
            break;
        case AXK_BLUFI_EVENT_REPORT_ERROR:
            blog_info("BLUFI report error, error code %d", param->report_error.state);
            axk_blufi_send_error_info(param->report_error.state);
            break;
        case AXK_BLUFI_EVENT_GET_WIFI_STATUS:
        {
            int state = 0;
            wifi_mgmr_state_get(&state);
            wifi_mode_t mode = WIFIMODE_STA;
            
            if (state == WIFI_STATE_CONNECTED_IP_GOT || state == WIFI_STATE_WITH_AP_CONNECTED_IP_GOT)
            {
                axk_blufi_extra_info_t info = {
                    .sta_ssid = (unsigned char *)ssid,
                    .sta_ssid_len = strlen(ssid),
                };
                axk_blufi_send_wifi_conn_report(mode, _BLUFI_STA_CONN_SUCCESS, 0, &info);
            }
            else
            {
                axk_blufi_send_wifi_conn_report(mode, _BLUFI_STA_CONN_FAIL, 0, NULL);
            }
            blog_info("BLUFI get wifi status from AP");

            break;
        }
        case AXK_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
            blog_info("blufi close a gatt connection");
            axk_blufi_disconnect();
            break;
        case AXK_BLUFI_EVENT_DEAUTHENTICATE_STA:
            /* TODO */
            break;
        case AXK_BLUFI_EVENT_RECV_STA_BSSID:
            // sta_config.sta.bssid_set = 1;
            // esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            blog_info("Recv STA BSSID %s", param->sta_bssid.bssid);
            break;
        case AXK_BLUFI_EVENT_RECV_STA_SSID: {
            strncpy(ssid, (char*)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
            blog_info("Recv STA SSID %s", ssid);
            ef_set_and_save_env("SSID", ssid);
            break;
        }
        case AXK_BLUFI_EVENT_RECV_STA_PASSWD: {
            strncpy(psk, (char*)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
            blog_info("Recv STA PASSWORD %s", psk);
            ef_set_and_save_env("PSK", psk);
            break;
        }
        case AXK_BLUFI_EVENT_RECV_SOFTAP_SSID:
            break;
        case AXK_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
            break;
        case AXK_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
            break;
        case AXK_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
            break;
        case AXK_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
            break;
        case AXK_BLUFI_EVENT_GET_WIFI_LIST:
            break;
        case AXK_BLUFI_EVENT_RECV_CUSTOM_DATA:
            blog_info("Recv Custom Data len:%d", param->custom_data.data_len);
            blog_info("Custom Data:%.*s", param->custom_data.data_len, param->custom_data.data);
            //echo
            axk_blufi_send_custom_data(param->custom_data.data, param->custom_data.data_len);
            break;
        case AXK_BLUFI_EVENT_RECV_USERNAME:
            /* Not handle currently */
            break;
        case AXK_BLUFI_EVENT_RECV_CA_CERT:
            /* Not handle currently */
            break;
        case AXK_BLUFI_EVENT_RECV_CLIENT_CERT:
            /* Not handle currently */
            break;
        case AXK_BLUFI_EVENT_RECV_SERVER_CERT:
            /* Not handle currently */
            break;
        case AXK_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
            /* Not handle currently */
            break;
        case AXK_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
            /* Not handle currently */
            break;
        default:
            break;
    }
}

static _blufi_callbacks_t example_callbacks = {
    .event_cb = example_event_callback,
    // .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    // .encrypt_func = blufi_aes_encrypt,
    // .decrypt_func = blufi_aes_decrypt,
    // .checksum_func = blufi_crc_checksum,
};

static void event_cb_wifi_event(input_event_t* event, void* private_data)
{
    switch (event->code)
    {
        case CODE_WIFI_ON_INIT_DONE:
        {
            blog_info("[APP] [EVT] INIT DONE %lld", aos_now_ms());
            wifi_mgmr_start_background(&conf);
        }
        break;
        case CODE_WIFI_ON_MGMR_DONE:
        {
            blog_info("[APP] [EVT] MGMR DONE %lld", aos_now_ms());
            _connect_wifi();
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE:
        {
            blog_info("[APP] [EVT] SCAN Done %lld", aos_now_ms());
            // wifi_mgmr_cli_scanlist();
        }
        break;
        case CODE_WIFI_ON_DISCONNECT:
        {
            blog_error("[APP] [EVT] disconnect %lld", aos_now_ms());
            lv_label_set_text(status_value, "Disconnected");
        }
        break;
        case CODE_WIFI_ON_CONNECTING:
        {
            blog_info("[APP] [EVT] Connecting %lld", aos_now_ms());
            lv_label_set_text(status_value, "Connecting");
        }
        break;
        case CODE_WIFI_CMD_RECONNECT:
        {
            blog_info("[APP] [EVT] Reconnect %lld", aos_now_ms());
            lv_label_set_text(status_value, "Reconnecting");
        }
        break;
        case CODE_WIFI_ON_CONNECTED:
        {
            blog_info("[APP] [EVT] connected %lld", aos_now_ms());
            lv_label_set_text(status_value, "Connected");
        }
        break;
        case CODE_WIFI_ON_PRE_GOT_IP:
        {
            blog_info("[APP] [EVT] connected %lld", aos_now_ms());
            lv_label_set_text(status_value, "Connected");
        }
        break;
        case CODE_WIFI_ON_GOT_IP:
        {
            blog_info("[APP] [EVT] GOT IP %lld", aos_now_ms());
            blog_info("[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());

            example_event_callback(AXK_BLUFI_EVENT_GET_WIFI_STATUS, NULL);

            lv_label_set_text(status_value, "Connected");

            ip4_addr_t ip, gw, mask;
            wifi_mgmr_sta_ip_get(&ip.addr, &gw.addr, &mask.addr);
            lv_label_set_text(ip_address_value, ip4addr_ntoa(&ip));
            lv_label_set_text(gateway_value, ip4addr_ntoa(&gw));
            lv_label_set_text(netmask_value, ip4addr_ntoa(&mask));
        }
        break;
        case CODE_WIFI_ON_PROV_SSID:
        {
            blog_info("[APP] [EVT] [PROV] [SSID] %lld: %s",
                      aos_now_ms(),
                      event->value ? (const char*)event->value : "UNKNOWN");
            if (event->value)
            {
                vPortFree((void*)event->value);
            }
        }
        break;
        case CODE_WIFI_ON_PROV_BSSID:
        {
            blog_info("[APP] [EVT] [PROV] [BSSID] %lld: %s",
                      aos_now_ms(),
                      event->value ? (const char*)event->value : "UNKNOWN");
            if (event->value)
            {
                vPortFree((void*)event->value);
            }
        }
        break;
        case CODE_WIFI_ON_PROV_PASSWD:
        {
            blog_info("[APP] [EVT] [PROV] [PASSWD] %lld: %s", aos_now_ms(),
                      event->value ? (const char*)event->value : "UNKNOWN");
            if (event->value)
            {
                vPortFree((void*)event->value);
            }
        }
        break;
        case CODE_WIFI_ON_PROV_CONNECT:
        {
            blog_info("[APP] [EVT] [PROV] [CONNECT] %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_PROV_DISCONNECT:
        {
            blog_info("[APP] [EVT] [PROV] [DISCONNECT] %lld", aos_now_ms());
        }
        break;
        default:
        {
            blog_info("[APP] [EVT] Unknown code %u, %lld", event->code, aos_now_ms());
            /*nothing*/
        }
    }
}

static void _cli_init()
{
    ble_cli_register();
    wifi_mgmr_cli_init();
    easyflash_cli_init();
    network_netutils_iperf_cli_register();
    network_netutils_netstat_cli_register();
    network_netutils_ping_cli_register();
    network_netutils_tcpclinet_cli_register();
    network_netutils_tcpserver_cli_register();
}

static void on_reset_button_clicked(lv_event_t * e)
{
    (void) e;

    blog_info("WiFi Config Reset");

    ef_del_env("SSID");
    ef_del_env("PSK");
    ef_save_env();

    hal_reboot();
}

static void on_room1_button_clicked(lv_event_t * e)
{
    (void) e;

    blog_info("room 1\n");
}

static void on_room2_button_clicked(lv_event_t * e)
{
    (void) e;

    blog_info("room 2\n");
}

void main(void)
{
    bl_sys_init();
    _cli_init();
    
    easyflash_init();
    hal_wifi_start_firmware_task();
    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
    tcpip_init(NULL, NULL);

    if (!ef_get_env("SSID")) {
        axk_hal_blufi_init();
        set_blufi_name("PANEL");
        _blufi_host_and_cb_init(&example_callbacks);
    }
    
    start_ui_task();
    lv_obj_add_event_cb(reset_button, on_reset_button_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(room1_button, on_room1_button_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(room2_button, on_room2_button_clicked, LV_EVENT_CLICKED, NULL);

    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
}
