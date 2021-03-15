#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "uart.h"

#include "status_led.h"

static const char moduleStr[] = "TSK";
#define PTS_dbg(fmt, ...) PTS_d(moduleStr, fmt, ##__VA_ARGS__)
#define PTS_dbg_f(fmt, ...) PTS_df(moduleStr, fmt, ##__VA_ARGS__)
#define SIZEOF(arr) (sizeof(arr) / sizeof(arr[0]))

void _tTaskManager(void *argument);

xTaskHandle _hTaskManager;
xTaskHandle _hStatusLed;
xTaskHandle _hPrintBlink;

typedef struct {
    const char * const name;
    TaskFunction_t func;
    TaskHandle_t * const handle;
    void * const parameters;
    const configSTACK_DEPTH_TYPE stack_size;
    UBaseType_t priority;
    uint8_t autorun;
} taskStruct_t;

const static taskStruct_t tasks[] = {
    {"statusLed",           _tStatusLed,        &_hStatusLed,        NULL,       configMINIMAL_STACK_SIZE,       16,        1},
    {"printBlink",          _tPrintBlink,       &_hPrintBlink,       NULL,       configMINIMAL_STACK_SIZE,       16,        0},
};

void task_manager_init(void) {
    xTaskCreate(_tTaskManager, "taskManager", configMINIMAL_STACK_SIZE, NULL, 16, &_hTaskManager);
}

void _tTaskManager(void *argument) {
    uint8_t i;
    const taskStruct_t* task;

    for (i = 0; i < SIZEOF(tasks); i++) {
        task = &tasks[i];
        if (task->autorun) {
            xTaskCreate(task->func, task->name, task->stack_size, task->parameters, task->priority, task->handle);
            PTS_dbg_f(" +%-16s START", task->name);
        }
    }

    // while(1) {
    //     /* print some task statistics */
    // }

    vTaskDelete(_hTaskManager);
}

void task_toggle(void) {
    const taskStruct_t* task = &tasks[1];
    // PTS_dbg_f(" %s-16s handle:%d", task->name, *task->handle);

    if (*task->handle == 0) {
        xTaskCreate(task->func, task->name, task->stack_size, task->parameters, task->priority, task->handle);
        PTS_dbg_f(" +%-16s START", task->name);
    } else {
        eTaskState state = eTaskGetState(*task->handle);
        if (state <= eSuspended) {
            vTaskDelete(*task->handle);
            PTS_dbg_f(" -%-16s STOP", task->name);
        } else {
            xTaskCreate(task->func, task->name, task->stack_size, task->parameters, task->priority, task->handle);
            PTS_dbg_f(" +%-16s START", task->name);
        }
    }
}
