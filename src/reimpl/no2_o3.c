/* Reimplementation of no2_o3.c.o from lib_no2_o3.a, derived from the AArch64
 * disassembly (see doc/decompiled-no2-o3.md). Float operations keep the
 * evaluation order and the fused multiply-adds of the original; build this
 * file with -ffp-contract=off so the compiler adds no other fusing.
 * Verified against the archive by test_script/reimpl_compare.c. */
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "no2_o3.h"
#include "no2_o3_data.h"

#define CNT_STABILIZATION (0x32u)
#define CNT_ALPHA_MAX     (144000u)

algorithm_version no2_o3_ver = {1, 0, 1};

void calc_no2_o3_O3_nnet(const float *in, float *out);
void calc_no2_o3_NO2_nnet(const float *in, float *out);

/* fcvtau: round to nearest, ties away, unsigned saturating */
static uint32_t cvt_au(float x)
{
    if (!(x > 0.0f)) {
        return 0;
    }
    if (x >= 4294967296.0f) {
        return 0xFFFFFFFFu;
    }
    return (uint32_t)roundf(x);
}

/* fcvtzu: truncate, unsigned saturating */
static uint32_t cvt_zu(float x)
{
    if (!(x > 0.0f)) {
        return 0;
    }
    if (x >= 4294967296.0f) {
        return 0xFFFFFFFFu;
    }
    return (uint32_t)x;
}

/* AQI by linear interpolation between EPA breakpoints. Below the first
 * breakpoint the AQI is conc * k, above the last one it is capped. */
static float aqi_from_conc(float conc, const int16_t *aqi, const int16_t *c,
                           int n, float k)
{
    int i;

    if (conc <= (float)c[0]) {
        return conc * k;
    }
    for (i = 1; i < n; i++) {
        if (conc <= (float)c[i]) {
            float slope =
                (float)(aqi[i] - aqi[i - 1]) / (float)(c[i] - c[i - 1]);
            return fmaf(slope, conc - (float)c[i - 1], (float)aqi[i - 1]);
        }
    }
    return (float)aqi[n - 1];
}

int8_t init_no2_o3(no2_o3_handle_t *handle)
{
    handle->sample_counter = 0;
    memset(handle->rmoxs_smooth, 0, sizeof(handle->rmoxs_smooth));
    memset(handle->logrmoxs_mean, 0, sizeof(handle->logrmoxs_mean));
    memcpy(handle->logrmoxs_var, no2_o3_longtermvar_start,
           sizeof(handle->logrmoxs_var));
    handle->o3_1min_ppb = 0.0f;
    handle->o3_1h_ppb = 0.0f;
    handle->o3_8h_ppb = 0.0f;
    handle->no2_1min_ppb = 0.0f;
    handle->no2_1h_ppb = 0.0f;
    return 0;
}

/* Constants of the original, kept as raw bit patterns (literal pool of
 * no2_o3.c.o) so they are bit-exact. */
static float KF(uint32_t bits)
{
    float f;

    memcpy(&f, &bits, sizeof(f));
    return f;
}

static double KD(uint64_t bits)
{
    double d;

    memcpy(&d, &bits, sizeof(d));
    return d;
}

static float rmox_from_adc(const uint8_t *p, uint16_t lr, uint16_t er,
                           uint8_t cfg0)
{
    uint16_t adc = (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
    float r;

    if (lr >= adc) {
        return 1e2f;
    }
    if (er <= adc) {
        return 1e12f;
    }
    r = (float)cfg0 * 1e3f;
    r = r * (float)(uint32_t)(adc - lr);
    r = r / (float)(uint32_t)(er - adc);
    if (r > 1e12f) {
        return 1e12f;
    }
    return (r >= 1e2f) ? r : 1e2f;
}

int8_t init_no2_o3_internal(no2_o3_handle_t *handle)
{
    return init_no2_o3(handle);
}

/* AQI of one concentration, tables from no2_o3_data.c */
#define AQI_O3_1H(c) aqi_from_conc((c), no2_o3_epa_o3_1h_aqi, no2_o3_epa_o3_1h_conc, 6, KF(0x3f4ed917u))
#define AQI_NO2_1H(c) aqi_from_conc((c), no2_o3_epa_no2_1h_aqi, no2_o3_epa_no2_1h_conc, 6, KF(0x3f71c71cu))
#define AQI_O3_8H(c) aqi_from_conc((c), no2_o3_epa_o3_8h_aqi, no2_o3_epa_o3_8h_conc, 5, KF(0x3f6d61bfu))

/* args: s0 = thermal reading, s1/s2 = temperature calibration terms,
 * hum/temp_in = ambient inputs; rmox points at results->rmox. */
int8_t calc_no2_o3_internal(no2_o3_handle_t *handle, float *rmox,
                            uint8_t prod9, no2_o3_results_t *results,
                            float therm, float t_cal1, float t_cal2,
                            float hum, float temp_in)
{
    float logv[4];
    float comp[4];
    float in[14];
    float nn_o3, nn_no2;
    uint32_t cnt;
    int i;

    if (handle->sample_counter != 0xFFFFFFFFu) {
        handle->sample_counter++;
    }
    cnt = handle->sample_counter;

    if (temp_in > -280.0f && temp_in < 1000000.0f) {
        results->temperature = temp_in;
    } else {
        float t = therm / t_cal1;
        t = t - 1.0f;
        t = t / KF(0x3b73e971u);
        t = t + t_cal2;
        results->temperature = t - 1.3f;
    }

    if (cnt <= CNT_STABILIZATION) {
        for (i = 0; i < 4; i++) {
            handle->rmoxs_smooth[i] = rmox[i];
        }
    } else {
        for (i = 0; i < 4; i++) {
            float q = rmox[i] / handle->rmoxs_smooth[i];
            float p = (float)pow((double)q, KD(0x3fcc504180000000ull));
            handle->rmoxs_smooth[i] = handle->rmoxs_smooth[i] * p;
        }
    }
    for (i = 0; i < 4; i++) {
        logv[i] = (float)log10((double)handle->rmoxs_smooth[i]);
    }

    {
        float dh = hum - 50.0f;
        float dT = results->temperature - 20.0f;

        for (i = 0; i < 4; i++) {
            float v3 = fmaf(dh, no2_o3_p_02[i], no2_o3_p_01[i]);
            float v5 = fmaf(dT, no2_o3_p_20[i], no2_o3_p_10[i]);
            float v2;

            v3 = fmaf(no2_o3_p_11[i], dT, v3);
            v2 = fmaf(v5, dT, 1.0f);
            comp[i] = fmaf(v3, dh, v2) * logv[i];
        }
    }

    if (cnt <= CNT_STABILIZATION) {
        for (i = 0; i < 4; i++) {
            handle->logrmoxs_mean[i] = comp[i];
        }
    } else {
        static const uint32_t var_floor[4] = {0x3b38bb9au, 0x3b00e26du,
                                              0x3ad6f680u, 0x3ac69cc2u};
        double arg;
        float alpha;

        if (cnt <= CNT_ALPHA_MAX) {
            float a = -1.0f / (float)cnt;

            if (a > -82.0f) {
                arg = (a <= 82.0f) ? (double)a : 82.0;
            } else {
                arg = -82.0;
            }
        } else {
            arg = KD(0xbedd208a60000000ull);
        }
        alpha = 1.0f - (float)exp(arg);

        for (i = 0; i < 4; i++) {
            float m = handle->logrmoxs_mean[i];
            float v = handle->logrmoxs_var[i];
            float d;
            float fl = KF(var_floor[i]);

            m = fmaf(alpha, comp[i] - m, m);
            d = comp[i] - m;
            v = fmaf(alpha, fmaf(d, d, -v), v);
            handle->logrmoxs_mean[i] = m;
            handle->logrmoxs_var[i] = (v > fl) ? v : fl;
        }

        in[0] = hum;
        in[1] = results->temperature;
        for (i = 0; i < 4; i++) {
            in[2 + 3 * i] = comp[i];
            in[3 + 3 * i] = handle->logrmoxs_mean[i];
            in[4 + 3 * i] = handle->logrmoxs_var[i];
        }
        calc_no2_o3_O3_nnet(in, &nn_o3);
        calc_no2_o3_NO2_nnet(in, &nn_no2);

        {
            float scale = (float)pow(KD(0x3ff0ae7ee0000000ull),
                                     (double)(int8_t)prod9);
            float o3 = scale * nn_o3;
            float no2 = scale * nn_no2;
            float c9 = KF(0x3dc2e49au);
            float c6 = KF(0x3ada457au);
            float c10 = KF(0x395a6e3au);

            o3 = !(o3 < 0.0f) ? o3 : 0.0f;
            no2 = !(no2 < 0.0f) ? no2 : 0.0f;
            handle->o3_1min_ppb =
                fmaf(o3 - handle->o3_1min_ppb, c9, handle->o3_1min_ppb);
            handle->o3_1h_ppb =
                fmaf(o3 - handle->o3_1h_ppb, c6, handle->o3_1h_ppb);
            handle->o3_8h_ppb =
                fmaf(o3 - handle->o3_8h_ppb, c10, handle->o3_8h_ppb);
            handle->no2_1min_ppb =
                fmaf(no2 - handle->no2_1min_ppb, c9, handle->no2_1min_ppb);
            handle->no2_1h_ppb =
                fmaf(no2 - handle->no2_1h_ppb, c6, handle->no2_1h_ppb);
        }
    }

    {
        float a_o3 = AQI_O3_1H(handle->o3_1min_ppb);
        float a_no2 = AQI_NO2_1H(handle->no2_1min_ppb);
        float fast = !(a_no2 < a_o3) ? a_no2 : a_o3;
        float e1 = AQI_O3_1H(handle->o3_1h_ppb);
        float e8 = AQI_O3_8H(handle->o3_8h_ppb);
        float en = AQI_NO2_1H(handle->no2_1h_ppb);
        uint32_t epa;

        results->O3_conc_ppb = handle->o3_1min_ppb;
        results->NO2_conc_ppb = handle->no2_1min_ppb;
        results->FAST_AQI = (uint16_t)cvt_au(fast);
        if (e8 < e1) {
            epa = !(en < e1) ? (cvt_zu(en) & 0xffffu) : (cvt_zu(e1) & 0xffffu);
        } else {
            epa = (en < e8) ? (cvt_zu(e8) & 0xfffu) : (cvt_zu(en) & 0xffffu);
        }
        results->EPA_AQI = (uint16_t)epa;
    }

    if (cnt <= CNT_STABILIZATION) {
        return NO2_O3_STABILIZATION;
    }
    if (rmox[3] < 250.0f || rmox[3] > KF(0x551184e7u)) {
        return NO2_O3_DAMAGE;
    }
    return (therm > KF(0x42763886u)) ? NO2_O3_DAMAGE : NO2_O3_OK;
}

int8_t calc_no2_o3(no2_o3_handle_t *handle, const zmod4xxx_dev_t *dev,
                   const no2_o3_inputs_t *algo_input,
                   no2_o3_results_t *results)
{
    static const uint8_t ch[4] = {8, 18, 22, 30};
    const uint8_t *adc = algo_input->adc_result;
    uint16_t t0 = (uint16_t)(((uint16_t)adc[0] << 8) | adc[1]);
    uint16_t t23 = (uint16_t)(((uint16_t)dev->config[2] << 8) | dev->config[3]);
    float therm, c1, c2;
    int i;

    for (i = 0; i < 4; i++) {
        results->rmox[i] = rmox_from_adc(adc + ch[i], dev->mox_lr, dev->mox_er,
                                         dev->config[0]);
    }
    c1 = (float)t23 / KF(0x4423d70au);
    c2 = (float)dev->config[5] * 0.5f;
    therm = fmaf((float)t0, KF(0x3abf1746u), KF(0x417e8ba3u));
    return calc_no2_o3_internal(handle, results->rmox, dev->prod_data[9],
                                results, therm, c1, c2,
                                algo_input->humidity_pct,
                                algo_input->temperature_degc);
}
