#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include <hosal_timer.h>

#include "lv_conf.h"
#include "lv_font.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "icon.h"

LV_FONT_DECLARE(font_awesome_pro_60);

LV_IMG_DECLARE(wechat_applet_qrcode);

TaskHandle_t ui_task_handle;

lv_obj_t * humidity_value_label;

lv_obj_t * temperature_icon_label;

lv_obj_t * temperature_value_label;

lv_obj_t * room1_button;

lv_obj_t * room1_icon;

lv_obj_t * room2_button;

lv_obj_t * room2_icon;

lv_obj_t * people_icon;

lv_obj_t * status_value;

lv_obj_t * ip_address_value;

lv_obj_t * netmask_value;

lv_obj_t * gateway_value;

lv_obj_t * reset_button;

static void create_ui(void)
{
    lv_obj_t * tv = lv_tileview_create(lv_scr_act());

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
        lv_obj_set_layout(tile, LV_LAYOUT_GRID);

        static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(tile, column_dsc, row_dsc);

        lv_obj_t * humidity_icon_label = lv_label_create(tile);
        lv_label_set_text(humidity_icon_label, ICON_DROPLET_PERCENT);
        lv_obj_set_style_text_font(humidity_icon_label, &font_awesome_pro_60, 0);
        lv_obj_set_style_text_align(humidity_icon_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(humidity_icon_label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        humidity_value_label = lv_label_create(tile);
        lv_label_set_text(humidity_value_label, "N/A%");
        lv_obj_set_style_text_font(humidity_value_label, &font_awesome_pro_60, 0);
        lv_obj_set_style_text_align(humidity_value_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(humidity_value_label, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        temperature_icon_label = lv_label_create(tile);
        lv_label_set_text(temperature_icon_label, ICON_TEMPERATURE_HALF);
        lv_obj_set_style_text_font(temperature_icon_label, &font_awesome_pro_60, 0);
        lv_obj_set_style_text_align(temperature_icon_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(temperature_icon_label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        temperature_value_label = lv_label_create(tile);
        lv_label_set_text(temperature_value_label, "N/A℃");
        lv_obj_set_style_text_font(temperature_value_label, &font_awesome_pro_60, 0);
        lv_obj_set_style_text_align(temperature_value_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(temperature_value_label, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    }

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP | LV_DIR_BOTTOM);
        lv_obj_set_layout(tile, LV_LAYOUT_GRID);

        static lv_coord_t column_dsc[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(tile, column_dsc, row_dsc);

        {
            static lv_coord_t column_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
            static lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

            room1_button = lv_btn_create(tile);
            lv_obj_set_layout(room1_button, LV_LAYOUT_GRID);
            lv_obj_set_grid_dsc_array(room1_button, column_dsc, row_dsc);
            lv_obj_set_grid_cell(room1_button, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

            room1_icon = lv_label_create(room1_button);
            lv_label_set_text(room1_icon, ICON_LIGHTBULB_EXCLAMATION);
            lv_obj_set_style_text_font(room1_icon, &font_awesome_pro_60, 0);
            lv_obj_set_style_text_align(room1_icon, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_grid_cell(room1_icon, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

            lv_obj_t * room1_text = lv_label_create(room1_button);
            lv_label_set_text(room1_text, "Room 1");
            lv_obj_set_style_text_font(room1_text, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_align(room1_text, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_grid_cell(room1_text, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

            room2_button = lv_btn_create(tile);
            lv_obj_set_layout(room2_button, LV_LAYOUT_GRID);
            lv_obj_set_grid_dsc_array(room2_button, column_dsc, row_dsc);
            lv_obj_set_grid_cell(room2_button, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

            room2_icon = lv_label_create(room2_button);
            lv_label_set_text(room2_icon, ICON_LIGHTBULB_EXCLAMATION);
            lv_obj_set_style_text_font(room2_icon, &font_awesome_pro_60, 0);
            lv_obj_set_style_text_align(room2_icon, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_grid_cell(room2_icon, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

            lv_obj_t * room2_text = lv_label_create(room2_button);
            lv_label_set_text(room2_text, "Room 2");
            lv_obj_set_style_text_font(room2_text, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_align(room2_text, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_grid_cell(room2_text, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
        }

        people_icon = lv_label_create(tile);
        lv_label_set_text(people_icon, ICON_USER_XMARK);
        lv_obj_set_style_text_font(people_icon, &font_awesome_pro_60, 0);
        lv_obj_set_style_text_align(people_icon, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(people_icon, LV_GRID_ALIGN_STRETCH, 1, 3, LV_GRID_ALIGN_CENTER, 1, 1);
    }

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 2, LV_DIR_TOP | LV_DIR_BOTTOM);
        lv_obj_t * img = lv_img_create(tile);
        lv_img_set_src(img, &wechat_applet_qrcode);
    }

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 3, LV_DIR_TOP);
        lv_obj_set_layout(tile, LV_LAYOUT_GRID);

        static lv_coord_t column_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(tile, column_dsc, row_dsc);

        lv_obj_t * status_label = lv_label_create(tile);
        lv_label_set_text(status_label, "Status");
        lv_obj_set_style_text_font(status_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_grid_cell(status_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        status_value = lv_label_create(tile);
        lv_label_set_text(status_value, "Disconnected");
        lv_obj_set_style_text_font(status_value, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(status_value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_grid_cell(status_value, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        lv_obj_t * ip_address_label = lv_label_create(tile);
        lv_label_set_text(ip_address_label, "Address");
        lv_obj_set_style_text_font(ip_address_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(ip_address_label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_grid_cell(ip_address_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        ip_address_value = lv_label_create(tile);
        lv_label_set_text(ip_address_value, "0.0.0.0");
        lv_obj_set_style_text_font(ip_address_value, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(ip_address_value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_grid_cell(ip_address_value, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        lv_obj_t * netmask_label = lv_label_create(tile);
        lv_label_set_text(netmask_label, "Netmask");
        lv_obj_set_style_text_font(netmask_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(netmask_label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_grid_cell(netmask_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

        netmask_value = lv_label_create(tile);
        lv_label_set_text(netmask_value, "0.0.0.0");
        lv_obj_set_style_text_font(netmask_value, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(netmask_value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_grid_cell(netmask_value, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

        lv_obj_t * gateway_label = lv_label_create(tile);
        lv_label_set_text(gateway_label, "Gateway");
        lv_obj_set_style_text_font(gateway_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(gateway_label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_grid_cell(gateway_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

        gateway_value = lv_label_create(tile);
        lv_label_set_text(gateway_value, "0.0.0.0");
        lv_obj_set_style_text_font(gateway_value, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(gateway_value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_grid_cell(gateway_value, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);

        reset_button = lv_btn_create(tile);
        lv_obj_t * reset_label = lv_label_create(reset_button);
        lv_label_set_text(reset_label, "Reset");
        lv_obj_set_style_text_font(reset_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_align(reset_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(reset_button, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 4, 1);
    }
}

static void timer_cb(void* arg)
{
    lv_tick_inc(1);
}

static void ui_routine(void *arg)
{
    (void) arg;
    
    while (1) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_timer_handler();
    }
}

void start_ui_task()
{
    static hosal_timer_dev_t lv_timer_dev = {
        .config = {
            .arg = NULL,
            .cb = timer_cb,
            .period = 1000,
            .reload_mode = TIMER_RELOAD_PERIODIC,
        },
        .port = 0,
    };
    lv_init();

    lv_port_disp_init();
    lv_port_indev_init();

    hosal_timer_init(&lv_timer_dev);
    hosal_timer_start(&lv_timer_dev);

    create_ui();

    xTaskCreate(ui_routine, "ui_routine", 8192, NULL, 10, &ui_task_handle);
}
