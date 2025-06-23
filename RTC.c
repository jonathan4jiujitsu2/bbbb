#include "rtc.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/ioctl.h>

static int i2c_fd = -1;

static uint8_t bcd_to_bin(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

static uint8_t bin_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

int rtc_init(const char *device, int addr) {
    i2c_fd = open(device, O_RDWR);
    if (i2c_fd < 0) {
        perror("Failed to open I2C device");
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
        perror("Failed to set I2C address");
        close(i2c_fd);
        return -1;
    }

    return 0;
}

void rtc_close() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

int rtc_read_time(struct tm *tm) {
    uint8_t buf[7];
    uint8_t reg = 0x00;
    
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("Failed to select register");
        return -1;
    }

    if (read(i2c_fd, buf, 7) != 7) {
        perror("Failed to read time");
        return -1;
    }

    tm->tm_sec  = bcd_to_bin(buf[0] & 0x7F);
    tm->tm_min  = bcd_to_bin(buf[1]);
    tm->tm_hour = bcd_to_bin(buf[2] & 0x3F);
    tm->tm_mday = bcd_to_bin(buf[4]);
    tm->tm_mon  = bcd_to_bin(buf[5]) - 1;
    tm->tm_year = bcd_to_bin(buf[6]) + 100;

    return 0;
}

int rtc_set_time(const struct tm *tm) {
    uint8_t buf[8];

    buf[0] = 0x00; // Start register
    buf[1] = bin_to_bcd(tm->tm_sec);
    buf[2] = bin_to_bcd(tm->tm_min);
    buf[3] = bin_to_bcd(tm->tm_hour);
    buf[4] = bin_to_bcd(0); // Day of week, not used
    buf[5] = bin_to_bcd(tm->tm_mday);
    buf[6] = bin_to_bcd(tm->tm_mon + 1);
    buf[7] = bin_to_bcd(tm->tm_year - 100);

    if (write(i2c_fd, buf, 8) != 8) {
        perror("Failed to write time");
        return -1;
    }

    return 0;
}