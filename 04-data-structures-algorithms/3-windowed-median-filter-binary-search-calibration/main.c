#include <inttypes.h>
#include "i2c.h"
#include "systick.h"
#include "bmp280.h"
#include "uart.h"

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

const LUT_Pressure_Altitude_t lut_array[4] = {{25909299, 10}, {25921255, 6}, {25930226, 3}, {25939200, 0}};

int32_t temperature_window[7];
uint32_t pressure_window[7];

// void InsertionSort(void *ptr, DataType type)
void InsertionSort(uint32_t *arr, int n)
{
    for (int i = 1; i < n; i++)
    {
        uint8_t key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j]; // shift right
            j--;
        }
        arr[j + 1] = key; // insert
    }
}

int BinarySearch(const LUT_Pressure_Altitude_t *arr, int size, uint32_t press_target, int *range_start, int *range_end)
{
    // edge cases handling - pressure is lower than min or higher than max
    if (press_target < arr[0].pressure || press_target > arr[size - 1].pressure)
    {
        return -1;
    }

    int low = 0;
    int high = size - 1;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (arr[mid].pressure == press_target)
        {
            *range_start = mid;
            *range_end = mid;
            return 1;
        }
        if (mid < (size - 1) && arr[mid].pressure <= press_target && arr[mid + 1].pressure >= press_target)
        {
            *range_start = mid;
            *range_end = mid + 1;
            return 1;
        }
        else if (arr[mid].pressure < press_target)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    // defensive return, even though if the target made past edge case checker, then the range of 2 points exists between which the target sits.
    return -1;
}

void main(void)
{
    // define and initialize structs
    hi2c.channel = I2C_CHANNEL_1;
    hi2c.scl_port = GPIOB;
    hi2c.scl_pin = 6;
    hi2c.sda_port = GPIOB;
    hi2c.sda_pin = 7;
    hi2c.state = I2C_STATE_IDLE;
    hi2c.sb_hits = 0;
    hi2c.stop_hits = 0;
    hi2c.start_pending_hits = 0;

    BMP280_HandleTypeDef hbmp = {
        .hi2c = &hi2c,
        .slave_addr = BMP280_I2C_ADDR,
        .isInitialized = 0};

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    // peripherals init
    I2C_Init();
    SysTick_Init((uint32_t)SYSTICK_FREQUENCY_16MHZ);
    usart2_init();

    hbmp.state = BMP280_STATE_INIT;

    uint8_t window_entries_counter = 0;

    // test loop
    while (1)
    {
        I2C_Process();

        switch (hbmp.state)
        {
        case BMP280_STATE_IDLE:
            break;
        case BMP280_STATE_INIT:
            if (BMP280_Init(&hbmp, meas) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_READ_CALIBRATION:
            if (BMP280_ReadCalibration(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_RECONSTRUCT_CALIBRATION:
            BMP280_ReconstructCalibration(&hbmp);
            break;
        case BMP280_STATE_CTRL_MEAS:
            if (BMP280_WriteCtrlMeas(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_MEASURING:
            if (BMP280_Measuring(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_READ_MEASURAMENTS:
            if (BMP280_ReadMeasurements(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_RECONSTRUCT_MEASURAMENTS:
            BMP280_ReconstructMeasurements(&hbmp);
            break;
        case BMP280_STATE_COMPENSATE:
            BMP280_CalculateData(&hbmp);
            break;
        case BMP280_STATE_READY:
            // printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", hbmp.temp_value / 100, hbmp.press_value / 256 / 100);

            // printf("start_pending_hits: %d | sb_hits: %d\r\n", hi2c.start_pending_hits, hi2c.sb_hits);

            temperature_window[window_entries_counter] = hbmp.temp_value;
            pressure_window[window_entries_counter] = hbmp.press_value;

            window_entries_counter++;

            // from idx 0 to 6 are legic, idx 7 is the signal that 7 samples are in the arrays
            if (window_entries_counter >= 7)
            {
                // select the median elements of the temperature and pressure windows

                // temperature in DegC, resolution is 0.01 DegC (5123 equals 51.23 Degrees)
                uint32_t temp_median = temperature_window[3]; // 0, 1, 2 - left part; 4, 5, 6 - right side
                // pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits)
                uint32_t press_median = pressure_window[3];

                float found_altitude = 0;
                int *press_range_start, *press_range_end;

                int binary_res = BinarySearch(lut_array, 4, press_median, press_range_start, press_range_end);

                if (binary_res)
                {
                    // the press_mediat landed in the lookup table element
                    // so we are using the lookup table altitude
                    if (*press_range_start == *press_range_end)
                    {
                        found_altitude = lut_array[*press_range_start].altitude;
                    }

                    // we need to find where the target sits in the range
                    // for that we are using the linear interpolation
                    // A = C + (fraction * (D - C))

                    float fraction = (press_median - lut_array[*press_range_start].pressure) / (lut_array[*press_range_end].pressure - lut_array[*press_range_start].pressure);

                    found_altitude = lut_array[*press_range_start].altitude + fraction * (lut_array[*press_range_end].altitude - lut_array[*press_range_start].altitude);

                    printf("Temp: %f degC | Press: %f hPa | Altitude: %f m\r\n", temp_median / 100, press_median / 256 / 100, found_altitude);
                }
                else
                {
                    // target is out of bound for the lookup table
                    printf("Temp: %f degC | Press: %f hPa | Unkhown altitude\r\n", temp_median / 100, press_median / 256 / 100);
                }

                window_entries_counter = 0;
            }

            hbmp.request_status = BMP280_REQUEST_NONE;
            hbmp.retries = 0;
            hbmp.measure_start_tick = 0;
            hbmp.measure_start_tick_status = BMP280_START_TICK_NEVER_CAPTURED;
            // BMP start measurements
            hbmp.state = BMP280_STATE_CTRL_MEAS;

            break;
        case BMP280_STATE_ERROR:
            if (hbmp.retries >= 3)
            {
                // the counter is exhausted
                hbmp.state = BMP280_STATE_FAULT;
                break;
            }

            if (hbmp.hi2c->state == I2C_STATE_IDLE)
            {
                hbmp.retries++;

                // begin a transaction from the beginning (Calibration is read once at the very beginning, so omit the state)
                hbmp.state = BMP280_STATE_CTRL_MEAS;
            }

            break;
        case BMP280_STATE_FAULT:
            printf("The BMP280 sensor experienced hard fault!");
            break;
        }
    }
}