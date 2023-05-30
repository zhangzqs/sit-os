#pragma once

typedef void (*TaskEntry)();
void task_init();
int task_create(TaskEntry entry, void* params, void* desc);
void task_yield();
void task_delay(int count);
void* task_get_current_params();
int task_get_current_id();
void task_block();
void task_wakeup(int id);
void task_exit();
int taskmgr_cmd(int argc, char** argv);