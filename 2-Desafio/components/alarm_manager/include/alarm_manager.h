#pragma once

#include <stdbool.h>
#include "nvs_storage.h"  

void alarm_manager_init(void);
bool alarm_manager_is_enabled(void);
int alarm_manager_count(void);
bool alarm_manager_get_alarm(int index, alarm_t *alarm);

