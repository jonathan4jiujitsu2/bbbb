#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <time.h>

int rtc_init(const char *device, int addr);
void rtc_close();
int rtc_read_time(struct tm *tm);
int rtc_set_time(const struct tm *tm);

#endif