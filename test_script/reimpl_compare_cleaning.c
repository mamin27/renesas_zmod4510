/* Compare src/reimpl/zmod4xxx_cleaning.c (renamed re_*) with the original
 * lib/lib_zmod4xxx_cleaning.a on a MOCK I2C bus. No hardware is touched.
 * The full trace of reads, writes and delays plus the return code must match,
 * also with an I2C failure injected at every possible operation. */
#include <stdio.h>
#include <string.h>
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"

int8_t re_zmod4xxx_cleaning_run(zmod4xxx_dev_t *dev);

static char trace[200000];
static size_t tlen;
static int op_count, fail_at, counter_reg, step, running;

static void log_add(const char *s)
{
    size_t n = strlen(s);

    if (tlen + n < sizeof(trace) - 1) {
        memcpy(trace + tlen, s, n + 1);
        tlen += n;
    }
}

static int8_t mock_read(uint8_t addr, uint8_t reg, uint8_t *b, uint8_t len)
{
    char s[64];

    op_count++;
    if (op_count == fail_at) {
        snprintf(s, sizeof(s), "R%02x:%02x:FAIL\n", addr, reg);
        log_add(s);
        return -1;
    }
    if (reg == 0x38) {
        b[0] = (uint8_t)counter_reg;
    } else if (reg == 0x94) {
        step ^= 1;
        b[0] = (uint8_t)((running ? 0x80 : 0x00) | step);
    } else {
        memset(b, 0, len);
    }
    snprintf(s, sizeof(s), "R%02x:%02x:%u=%02x\n", addr, reg, len, b[0]);
    log_add(s);
    return 0;
}

static int8_t mock_write(uint8_t addr, uint8_t reg, uint8_t *b, uint8_t len)
{
    char s[64];
    uint8_t i;

    op_count++;
    snprintf(s, sizeof(s), "W%02x:%02x:%u:", addr, reg, len);
    log_add(s);
    for (i = 0; i < len; i++) {
        snprintf(s, sizeof(s), "%02x", b[i]);
        log_add(s);
    }
    if (op_count == fail_at) {
        log_add(":FAIL\n");
        return -1;
    }
    log_add("\n");
    if (reg == 0x93) {
        running = (b[0] != 0);
    }
    if (reg == 0x38) {
        counter_reg = b[0];
    }
    return 0;
}

static void mock_delay(uint32_t ms)
{
    char s[32];

    snprintf(s, sizeof(s), "D%u\n", (unsigned)ms);
    log_add(s);
}

static int8_t run(int8_t (*fn)(zmod4xxx_dev_t *), int fail, int counter,
                  char *out, size_t outsz)
{
    zmod4xxx_dev_t dev;
    uint8_t prod[8] = {0};
    int8_t ret;

    memset(&dev, 0, sizeof(dev));
    dev.i2c_addr = 0x33;
    dev.config[2] = 0x12;
    dev.config[3] = 0x34;
    dev.config[4] = 5;
    dev.config[5] = 6;
    dev.prod_data = prod;
    dev.read = mock_read;
    dev.write = mock_write;
    dev.delay_ms = mock_delay;
    tlen = 0;
    trace[0] = 0;
    op_count = 0;
    fail_at = fail;
    counter_reg = counter;
    step = 0;
    running = 0;
    ret = fn(&dev);
    strncpy(out, trace, outsz - 1);
    out[outsz - 1] = 0;
    return ret;
}

static char ta[200000], tb[200000];

int main(void)
{
    int fail, bad = 0, cases = 0, counter;

    for (counter = 0; counter < 2; counter++) {
        for (fail = 0; fail < 90; fail++) {
            int8_t ra, rb;

            ra = run(zmod4xxx_cleaning_run, fail, counter, ta, sizeof(ta));
            rb = run(re_zmod4xxx_cleaning_run, fail, counter, tb, sizeof(tb));
            cases++;
            if (ra != rb || strcmp(ta, tb) != 0) {
                printf("DIFF counter=%d fail_at=%d ret %d/%d\n", counter, fail, ra, rb);
                bad++;
            }
        }
    }
    printf("compared %d cases, %d differences\n", cases, bad);
    return bad != 0;
}
