#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "drivers/8258/flash.h"
#include "etime.h"
#include "main.h"

RAM uint16_t time_trime = 5000;// The higher the number the slower the time runs!, -32,768 to 32,767 
RAM uint32_t one_second_trimmed = CLOCK_16M_SYS_TIMER_CLK_1S;
RAM uint32_t current_unix_time;
RAM struct date_time current_date = {0};

RAM uint32_t last_clock_increase;
RAM uint32_t last_reached_period[10] = {0};
RAM uint8_t has_ever_reached[10] = {0};

_attribute_ram_code_ void init_time(void)
{
    one_second_trimmed += time_trime;
    current_unix_time = 0;
}

_attribute_ram_code_ void unixTimeToDateTime(uint32_t t) // not calculated week
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;

    // Retrieve hours, minutes and seconds
    current_date.tm_sec = t % 60;
    t /= 60;
    current_date.tm_min = t % 60;
    t /= 60;
    current_date.tm_hour = t % 24;
    t /= 24;

    // Convert Unix time to date
    a = (uint32_t)((4 * t + 102032) / 146097 + 15);
    b = (uint32_t)(t + 2442113 + a - (a / 4));
    c = (20 * b - 2442) / 7305;
    d = b - 365 * c - (c / 4);
    e = d * 1000 / 30601;
    f = d - e * 30 - e * 601 / 1000;

    // January and February are counted as months 13 and 14 of the previous year
    if (e <= 13)
    {
        c -= 4716;
        e -= 1;
    }
    else
    {
        c -= 4715;
        e -= 13;
    }

    // Retrieve year, month and day
    current_date.tm_year = c;
    current_date.tm_month = e;
    current_date.tm_day = f;

    // week not calculated
    current_date.tm_week = 0;
}

_attribute_ram_code_ void handler_time(void)
{
    if (clock_time() - last_clock_increase >= one_second_trimmed)
    {
        last_clock_increase += one_second_trimmed;
        current_unix_time++;

        unixTimeToDateTime(current_unix_time);
    }
}

_attribute_ram_code_ uint8_t time_reached_period(timer_channel ch, uint32_t seconds)
{
    if (!has_ever_reached[ch])
    {
        has_ever_reached[ch] = 1;
        return 1;
    }
    if (current_unix_time - last_reached_period[ch] >= seconds)
    {
        last_reached_period[ch] = current_unix_time;
        return 1;
    }
    return 0;
}

_attribute_ram_code_ void set_time(uint32_t time_now, uint16_t time_year, uint8_t time_month, uint8_t time_day, uint8_t time_week)
{
    current_unix_time = time_now;
    current_date.tm_year = time_year;
    current_date.tm_month = time_month;
    current_date.tm_day = time_day;
    current_date.tm_week = time_week;
}

_attribute_ram_code_ struct date_time get_time(void)
{
    return current_date;
}

_attribute_ram_code_ uint32_t get_unix_time(void)
{
    return current_unix_time;
}