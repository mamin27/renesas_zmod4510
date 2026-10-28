/* Reimplementation of zmod4xxx_cleaning.c.o from lib_zmod4xxx_cleaning.a,
 * derived from the AArch64 disassembly and the .data of the object
 * (see doc/decompiled-zmod4xxx-cleaning.md).
 * Verified against the archive with a mock I2C bus only (never a real
 * sensor): test_script/reimpl_compare_cleaning.sh. */
#include <stdint.h>
#include <string.h>
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"

#define CLEANING_REG_COUNTER  (0x38) /* run counter kept by the sensor */
#define CLEANING_REG_NVM_CTRL (0xB8)
#define CLEANING_POLL_MS      (50)
#define CLEANING_STEPS_DONE   (30)

static uint8_t cl_h[2] = {0xfb, 0xb4};
static uint8_t cl_d[4] = {0x20, 0x04, 0x20, 0x04};
static uint8_t cl_m[1] = {0x03};
static uint8_t cl_s[4] = {0x01, 0xc0, 0x81, 0xc8};

static zmod4xxx_conf cleaning_conf = {
    0xC0,
    {0x40, 2, cl_h},
    {0x50, 4, cl_d},
    {0x60, 1, cl_m},
    {0x68, 4, cl_s},
    {0x99, 2, NULL},
    0,
};

static uint8_t enable_write_nvm_code = 0xEA;
static uint8_t disable_write_nvm_code = 0x13;
static uint8_t stop_cleaning_code = 0x00;

int8_t zmod4xxx_cleaning_run(zmod4xxx_dev_t *dev)
{
    zmod4xxx_dev_t d;
    uint8_t status = 0;
    uint8_t counter;
    uint8_t last = 0;
    uint16_t done = 0;
    int8_t ret;

    memcpy(&d, dev, sizeof(d));
    d.meas_conf = &cleaning_conf;

    if (d.read(d.i2c_addr, CLEANING_REG_COUNTER, &counter, 1) != 0) {
        return ERROR_I2C;
    }
    if (counter != 0) {
        return ERROR_CLEANING;
    }

    ret = zmod4xxx_init_measurement(&d);
    if (ret != 0) {
        return ret;
    }
    ret = zmod4xxx_start_measurement(&d);
    if (ret != 0) {
        return ret;
    }

    for (;;) {
        uint8_t step;

        d.delay_ms(CLEANING_POLL_MS);
        ret = zmod4xxx_read_status(&d, &status);
        if (ret != 0) {
            return ret;
        }
        step = status & STATUS_LAST_SEQ_STEP_MASK;
        if (step != last && step == (uint8_t)((cleaning_conf.s.len >> 1) - 1) &&
            ++done >= CLEANING_STEPS_DONE) {
            break;
        }
        last = step;
    }

    if (d.write(d.i2c_addr, ZMOD4XXX_ADDR_CMD, &stop_cleaning_code, 1) != 0) {
        return ERROR_I2C;
    }
    do {
        d.delay_ms(CLEANING_POLL_MS);
        ret = zmod4xxx_read_status(&d, &status);
        if (ret != 0) {
            return ret;
        }
    } while (status & STATUS_SEQUENCER_RUNNING_MASK);

    d.delay_ms(CLEANING_POLL_MS);
    if (d.write(d.i2c_addr, CLEANING_REG_NVM_CTRL, &enable_write_nvm_code, 1) != 0) {
        return ERROR_I2C;
    }
    d.delay_ms(CLEANING_POLL_MS);
    counter++;
    if (d.write(d.i2c_addr, CLEANING_REG_COUNTER, &counter, 1) != 0) {
        return ERROR_I2C;
    }
    d.delay_ms(CLEANING_POLL_MS);
    /* The original returns the raw result of this last write. */
    return d.write(d.i2c_addr, CLEANING_REG_NVM_CTRL, &disable_write_nvm_code, 1);
}
