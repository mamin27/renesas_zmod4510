/* Reimplementation of nnet_funcs.c.o, no2_o3_NO2_nnet.c.o and
 * no2_o3_O3_nnet.c.o from lib_no2_o3.a, derived from the AArch64
 * disassembly. Verified against the archive by test_script/reimpl_compare. */
#include <math.h>
#include <stdint.h>
#include "no2_o3_data.h"

#define SAFE_EXP_MAX_ARG (82.0f)

/* nnet_funcs_safe_exp exists in the archive but nothing calls it, so it is
 * not reimplemented. */

static void nnet_funcs_tanh(const float *in, float *out, uint8_t n)
{
    uint8_t i;

    for (i = 0; i < n; i++) {
        float x = in[i];
        float e;
        float inv;

        if (x > SAFE_EXP_MAX_ARG) {
            out[i] = 1.0f;
        } else if (x < -SAFE_EXP_MAX_ARG) {
            out[i] = -1.0f;
        } else {
            e = (float)exp((double)x);
            inv = 1.0f / e;
            out[i] = (e - inv) / (e + inv);
        }
    }
}

/* out[j] = bias_j + sum_i w_ji * in[i]; weights stored per neuron as
 * bias, w_0 .. w_(n_in-1). */
static void nnet_funcs_dotbias(const float *in, float *out, uint8_t n_in,
                               uint8_t n_out, const float *w)
{
    uint8_t j, i;

    for (j = 0; j < n_out; j++) {
        float acc = *w++;
        out[j] = acc;
        for (i = 0; i < n_in; i++) {
            acc = fmaf(w[i], in[i], acc); /* fmadd in the original */
            out[j] = acc;
        }
        w += n_in;
    }
}

#define DEFINE_NNET(NAME, L0, L2, L4, L6, L8, L10)                          \
    void NAME(const float *in, float *out)                                  \
    {                                                                       \
        float a[20];                                                        \
        float b[20];                                                        \
        nnet_funcs_dotbias(in, a, 14, 20, L0);                              \
        nnet_funcs_tanh(a, a, 20);                                          \
        nnet_funcs_dotbias(a, b, 20, 15, L2);                               \
        nnet_funcs_tanh(b, b, 15);                                          \
        nnet_funcs_dotbias(b, a, 15, 10, L4);                               \
        nnet_funcs_tanh(a, a, 10);                                          \
        nnet_funcs_dotbias(a, b, 10, 5, L6);                                \
        nnet_funcs_tanh(b, b, 5);                                           \
        nnet_funcs_dotbias(b, a, 5, 3, L8);                                 \
        nnet_funcs_tanh(a, a, 3);                                           \
        nnet_funcs_dotbias(a, out, 3, 1, L10);                              \
    }

DEFINE_NNET(calc_no2_o3_NO2_nnet, no2_o3_NO2_nnet_layer0,
            no2_o3_NO2_nnet_layer2, no2_o3_NO2_nnet_layer4,
            no2_o3_NO2_nnet_layer6, no2_o3_NO2_nnet_layer8,
            no2_o3_NO2_nnet_layer10)
DEFINE_NNET(calc_no2_o3_O3_nnet, no2_o3_O3_nnet_layer0,
            no2_o3_O3_nnet_layer2, no2_o3_O3_nnet_layer4,
            no2_o3_O3_nnet_layer6, no2_o3_O3_nnet_layer8,
            no2_o3_O3_nnet_layer10)
