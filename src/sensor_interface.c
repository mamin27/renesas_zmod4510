#include "sensor_interface.h"
#include "hal/hal.h"
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"
#include "zmod4510_config_no2_o3.h"

/* Internal variables */
static int          ret;
static Interface_t  hal;
static char const*  errContext;

/* Gas sensor related declarations */
static zmod4xxx_dev_t dev;
static uint8_t zmod4xxx_status;
static uint8_t adc_result[ZMOD4510_ADC_DATA_LEN];
static uint8_t prod_data[ZMOD4510_PROD_DATA_LEN];

/* Algorithm related declarations */
static no2_o3_handle_t  algo_handle;
static no2_o3_results_t algo_results;
static no2_o3_inputs_t  algo_input;

static
int detect_and_configure(zmod4xxx_dev_t* sensor, int pd_len, char const** errContext) {
    uint8_t track_number[ZMOD4XXX_LEN_TRACKING];

    /* Naša nová inicializácia cez HAL */
    ret = HAL_Init(&hal);
    if (ret) {
        *errContext = "HAL initialization";
        return ret;
    }

    ret = zmod4xxx_init(sensor, &hal);
    if (ret) {
        *errContext = "sensor initialization";
        return ret;
    }

    ret = zmod4xxx_read_sensor_info(sensor);
    if (ret) {
        *errContext = "reading sensor information";
        return ret;
    }

    ret = zmod4xxx_read_tracking_number(sensor, track_number);
    if (ret) {
        *errContext = "Reading tracking number";
        return ret;
    }

    printf("Sensor tracking number: x0000");
    for (int i = 0; i < sizeof(track_number); i++) {
        printf("%02X", track_number[i]);
    }
    printf("\n");

    printf("Starting cleaning procedure. This might take up to 1 min ...\n");
    ret = zmod4xxx_cleaning_run(sensor);
    if (ERROR_CLEANING == ret) {
        printf("Skipping cleaning procedure. It has already been performed\n");
    } else if (ret) {
        *errContext = "sensor cleaning";
        return ret;
    }

    ret = zmod4xxx_prepare_sensor(sensor);
    if (ret) {
        *errContext = "sensor preparation";
        return ret;
    }
    return 0;
}

static
void read_and_verify(zmod4xxx_dev_t* sensor, uint8_t* result, char const* id) {
    ret = zmod4xxx_read_status(sensor, &zmod4xxx_status);
    if (ret) {
        HAL_HandleError(ret, "Reading sensor status");
    }

    if (zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK) {
        ret = zmod4xxx_check_error_event(sensor);
        switch (ret) {
        case ERROR_POR_EVENT:
            HAL_HandleError(ret, "Reading result: Unexpected sensor reset!");
            break;
        case ZMOD4XXX_OK:
            HAL_HandleError(ret, "Reading result: Wrong sensor setup!");
            break;
        default:
            HAL_HandleError(ret, "Reading result: Unknown error!");
            break;
        }
    }

    ret = zmod4xxx_read_adc_result(sensor, result);
    if (ret) {
        HAL_HandleError(ret, "Reading ADC results");
    }

    ret = zmod4xxx_check_error_event(sensor);
    if (ret) {
        HAL_HandleError(ret, "Reading sensor status");
    }
}

int sensor_init() {
    dev.i2c_addr = ZMOD4510_I2C_ADDR;
    dev.pid = ZMOD4510_PID;
    dev.init_conf = &zmod_no2_o3_sensor_cfg[INIT];
    dev.meas_conf = &zmod_no2_o3_sensor_cfg[MEASUREMENT];
    dev.prod_data = prod_data;

    ret = detect_and_configure(&dev, ZMOD4510_PROD_DATA_LEN, &errContext);
    if (ret) {
        printf("Error during %s (err %d)\n", errContext, ret);
        return ret;
    }

    return init_no2_o3(&algo_handle);
}

void sensor_step(float temp, float humidity, sensor_results_t* out) {
    ret = zmod4xxx_start_measurement(&dev);
    if (ret) {
        out->status = NO2_O3_DAMAGE;
        return;
    }

    dev.delay_ms(ZMOD4510_NO2_O3_SAMPLE_TIME);

    read_and_verify(&dev, adc_result, "ZMOD4510");

    algo_input.adc_result = adc_result;
    algo_input.humidity_pct = humidity;
    algo_input.temperature_degc = temp;

    ret = calc_no2_o3(&algo_handle, &dev, &algo_input, &algo_results);

    out->o3_ppb = algo_results.O3_conc_ppb;
    out->no2_ppb = algo_results.NO2_conc_ppb;
    out->fast_aqi = algo_results.FAST_AQI;
    out->epa_aqi = algo_results.EPA_AQI;
    out->status = ret;
}

void sensor_close() {
    HAL_Deinit(&hal);
}
