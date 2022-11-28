#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>

extern TaskHandle_t ui_task_handle;

extern void start_ui_task();

void main(void)
{
    start_ui_task();
}
