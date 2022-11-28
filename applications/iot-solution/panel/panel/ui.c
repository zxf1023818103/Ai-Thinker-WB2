#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include "bl_sys.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include <hosal_timer.h>

TaskHandle_t ui_task_handle;

static void create_ui(void)
{
    lv_obj_t * tv = lv_tileview_create(lv_scr_act());

    /*Tile1: just a label*/
    lv_obj_t * tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
    lv_obj_t * label = lv_label_create(tile1);
    lv_label_set_text(label, "Scroll down");
    lv_obj_center(label);


    /*Tile2: a button*/
    lv_obj_t * tile2 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP | LV_DIR_RIGHT);

    lv_obj_t * btn = lv_btn_create(tile2);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Scroll up or right");

    lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(btn);

    /*Tile3: a list*/
    lv_obj_t * tile3 = lv_tileview_add_tile(tv, 1, 1, LV_DIR_LEFT);
    lv_obj_t * list = lv_list_create(tile3);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));

    lv_list_add_btn(list, NULL, "One");
    lv_list_add_btn(list, NULL, "Two");
    lv_list_add_btn(list, NULL, "Three");
    lv_list_add_btn(list, NULL, "Four");
    lv_list_add_btn(list, NULL, "Five");
    lv_list_add_btn(list, NULL, "Six");
    lv_list_add_btn(list, NULL, "Seven");
    lv_list_add_btn(list, NULL, "Eight");
    lv_list_add_btn(list, NULL, "Nine");
    lv_list_add_btn(list, NULL, "Ten");

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
