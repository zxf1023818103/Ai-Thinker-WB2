#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include "bl_sys.h"
#include "lv_conf.h"
#include "lv_font.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include <hosal_timer.h>

extern const lv_font_t FontAwesomePro;

TaskHandle_t ui_task_handle;

static void create_ui(void)
{
    lv_obj_t * tv = lv_tileview_create(lv_scr_act());

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
        lv_obj_set_layout(tile, LV_LAYOUT_GRID);

        static lv_coord_t column_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(tile, column_dsc, row_dsc);

        lv_obj_t * humidity_icon_label = lv_label_create(tile);
        lv_label_set_text(humidity_icon_label, "\xEF\x9D\x90");
        lv_obj_set_style_text_font(humidity_icon_label, &FontAwesomePro, 0);
        lv_obj_set_style_text_align(humidity_icon_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(humidity_icon_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        lv_obj_t * humidity_value_label = lv_label_create(tile);
        lv_label_set_text(humidity_value_label, "N/A%");
        lv_obj_set_style_text_font(humidity_value_label, &FontAwesomePro, 0);
        lv_obj_set_style_text_align(humidity_value_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(humidity_value_label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        lv_obj_t * temperature_icon_label = lv_label_create(tile);
        lv_label_set_text(temperature_icon_label, "\xEF\x8B\x89");
        lv_obj_set_style_text_font(temperature_icon_label, &FontAwesomePro, 0);
        lv_obj_set_style_text_align(temperature_icon_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(temperature_icon_label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

        lv_obj_t * temperature_value_label = lv_label_create(tile);
        lv_label_set_text(temperature_value_label, "N/A℃");
        lv_obj_set_style_text_font(temperature_value_label, &FontAwesomePro, 0);
        lv_obj_set_style_text_align(temperature_value_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_grid_cell(temperature_value_label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    }

    {
        lv_obj_t * tile = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP);
        lv_obj_set_layout(tile, LV_LAYOUT_FLEX);

        lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
        
    }

}

static void timer_cb(void* arg)
{
    lv_tick_inc(1);
}

static void ui_routine(void *arg)
{
    (void) arg;

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

    while (1) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_timer_handler();
    }
}

void start_ui_task()
{
    xTaskCreate(ui_routine, "ui_routine", 8192, NULL, 10, &ui_task_handle);
}
