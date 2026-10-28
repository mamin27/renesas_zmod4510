/* Compare the C reimplementation (src/reimpl, symbols renamed re_*) with the
 * original lib/lib_no2_o3.a on identical random inputs, bit by bit.
 * Build and run: test_script/reimpl_compare.sh */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "no2_o3.h"

int8_t re_init_no2_o3(no2_o3_handle_t *handle);
int8_t re_calc_no2_o3(no2_o3_handle_t *handle, const zmod4xxx_dev_t *dev,
                      const no2_o3_inputs_t *algo_input,
                      no2_o3_results_t *results);

static uint32_t rng_state = 12345u;

static uint32_t rnd(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state >> 8;
}

static float frnd(float lo, float hi)
{
    return lo + (hi - lo) * ((float)(rnd() & 0xFFFF) / 65535.0f);
}

int main(int argc, char **argv)
{
    int samples = (argc > 1) ? atoi(argv[1]) : 2000;
    int runs = (argc > 2) ? atoi(argv[2]) : 20;
    int run, i, bad = 0, total = 0;

    for (run = 0; run < runs; run++) {
        no2_o3_handle_t ha, hb;
        zmod4xxx_dev_t dev;
        uint8_t prod[32];
        uint8_t adc[32];
        no2_o3_inputs_t in;
        uint16_t lr = (uint16_t)(rnd() % 3000 + 100);
        uint16_t er = (uint16_t)(rnd() % 20000 + 30000);

        memset(&dev, 0, sizeof(dev));
        for (i = 0; i < 6; i++) {
            dev.config[i] = (uint8_t)rnd();
        }
        dev.config[0] = (uint8_t)(rnd() % 60 + 1);
        dev.mox_lr = lr;
        dev.mox_er = er;
        for (i = 0; i < 32; i++) {
            prod[i] = (uint8_t)rnd();
        }
        dev.prod_data = prod;
        in.adc_result = adc;
        init_no2_o3(&ha);
        re_init_no2_o3(&hb);
        if (memcmp(&ha, &hb, sizeof(ha)) != 0) {
            printf("init differs (run %d)\n", run);
            bad++;
        }
        for (i = 0; i < samples; i++) {
            no2_o3_results_t ra, rb;
            int8_t ea, eb;
            int k;

            for (k = 0; k < 32; k += 2) {
                uint16_t v = (uint16_t)(lr + rnd() % (uint32_t)(er - lr + 200));
                if (rnd() % 50 == 0) {
                    v = (rnd() & 1) ? 0 : 0xFFFF;
                }
                adc[k] = (uint8_t)(v >> 8);
                adc[k + 1] = (uint8_t)v;
            }
            in.humidity_pct = frnd(0.0f, 100.0f);
            in.temperature_degc = (rnd() % 10 == 0) ? -300.0f : frnd(-10.0f, 60.0f);
            memset(&ra, 0xA5, sizeof(ra));
            memset(&rb, 0xA5, sizeof(rb));
            ea = calc_no2_o3(&ha, &dev, &in, &ra);
            eb = re_calc_no2_o3(&hb, &dev, &in, &rb);
            total++;
            if (ea != eb || memcmp(&ra, &rb, sizeof(ra)) != 0 ||
                memcmp(&ha, &hb, sizeof(ha)) != 0) {
                if (bad < 10) {
                    printf("DIFF run %d sample %d: ret %d/%d aqi %u/%u epa %u/%u o3 %g/%g no2 %g/%g\n",
                           run, i, ea, eb, ra.FAST_AQI, rb.FAST_AQI,
                           ra.EPA_AQI, rb.EPA_AQI, ra.O3_conc_ppb,
                           rb.O3_conc_ppb, ra.NO2_conc_ppb, rb.NO2_conc_ppb);
                }
                bad++;
            }
        }
    }
    printf("compared %d samples, %d differences\n", total, bad);
    return bad != 0;
}
