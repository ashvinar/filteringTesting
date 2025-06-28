/**
 * @file wavelet_filter.c
 * @brief Implementation of an advanced, configurable wavelet filtering library.
 *
 * This file provides the core implementation for a fixed-point wavelet filter
 * that supports multiple wavelet families and thresholding strategies. It is
 * designed for performance and flexibility in embedded signal processing.
 */

#include "wavelet_filter.h"
#include <string.h>
#include <stdlib.h> // For abs(), malloc, free

// Q14 fixed-point coefficients for various wavelets
// To convert float `c` to Q14: (int16_t)(c * (1 << 14))

// Haar Wavelet (L=2)
// Analysis filters
static const int16_t haar_h0[] = {11585, 11585}; // Low-pass
static const int16_t haar_h1[] = {11585, -11585}; // High-pass
// Synthesis filters
static const int16_t haar_g0[] = {11585, 11585}; // Low-pass reconstruction
static const int16_t haar_g1[] = {-11585, 11585}; // High-pass reconstruction

// Daubechies 4 (db4) Wavelet (L=4)
// Analysis filters
static const int16_t db4_h0[] = {7913, 13705, 3672, -2120}; // Low-pass
static const int16_t db4_h1[] = {-2120, -3672, 13705, -7913}; // High-pass (derived from h0)
// Synthesis filters
static const int16_t db4_g0[] = {-2120, 3672, 13705, 7913}; // Low-pass reconstruction (time-reversed h0)
static const int16_t db4_g1[] = {-7913, 13705, -3672, -2120}; // High-pass reconstruction (time-reversed h1)

// Daubechies 6 (db6) Wavelet (L=6)
// Analysis filters
static const int16_t db6_h0[] = {3853, 9345, 5326, -1565, -990, 408}; // Low-pass
static const int16_t db6_h1[] = {408, 990, -1565, -5326, 9345, -3853}; // High-pass (derived from h0)
// Synthesis filters
static const int16_t db6_g0[] = {408, -990, -1565, 5326, 9345, 3853}; // Low-pass reconstruction (time-reversed h0)
static const int16_t db6_g1[] = {-3853, 9345, -5326, -1565, 990, 408}; // High-pass reconstruction (time-reversed h1)

static void get_wavelet_coeffs(
    wavelet_type_t wavelet,
    const int16_t** h0_kernel, const int16_t** h1_kernel,
    const int16_t** g0_kernel, const int16_t** g1_kernel,
    uint8_t* len) {

    switch (wavelet) {
        case WAVELET_DB6:
            *h0_kernel = db6_h0;
            *h1_kernel = db6_h1;
            *g0_kernel = db6_g0;
            *g1_kernel = db6_g1;
            *len = 6;
            break;
        case WAVELET_HAAR:
            *h0_kernel = haar_h0;
            *h1_kernel = haar_h1;
            *g0_kernel = haar_g0;
            *g1_kernel = haar_g1;
            *len = 2;
            break;
        case WAVELET_DB4:
        default:
            *h0_kernel = db4_h0;
            *h1_kernel = db4_h1;
            *g0_kernel = db4_g0;
            *g1_kernel = db4_g1;
            *len = 4;
            break;
    }
}

void wavelet_get_default_config(wavelet_config_t* config) {
    if (!config) return;
    config->wavelet = WAVELET_DB4;
    config->threshold_type = THRESHOLD_HARD;
    config->decomposition_levels = 6;
    config->threshold_value = 100;
    config->q_format = 14;
}

void dwt(const int16_t* input_signal, int16_t* approx_coeffs, int16_t* detail_coeffs, uint16_t n, wavelet_type_t wavelet, uint16_t q_format) {
    if (n < 2) return;

    const int16_t* h0_kernel = NULL;
    const int16_t* h1_kernel = NULL;
    const int16_t* g0_kernel = NULL;
    const int16_t* g1_kernel = NULL;
    uint8_t kernel_len = 0;
    get_wavelet_coeffs(wavelet, &h0_kernel, &h1_kernel, &g0_kernel, &g1_kernel, &kernel_len);

    if (n < kernel_len) return;

    uint16_t half = n >> 1;

    for (uint16_t i = 0; i < half; i++) {
        int32_t approx_val = 0;
        int32_t detail_val = 0;
        for (uint8_t j = 0; j < kernel_len; j++) {
            int input_idx = (2 * i - j + n) % n;
            approx_val += (int32_t)input_signal[input_idx] * h0_kernel[j];
            detail_val += (int32_t)input_signal[input_idx] * h1_kernel[j];
        }
        approx_coeffs[i] = (int16_t)(approx_val >> q_format);
        detail_coeffs[i] = (int16_t)(detail_val >> q_format);
    }
}

void idwt(const int16_t* approx_coeffs, const int16_t* detail_coeffs, int16_t* output_signal, uint16_t n_input_coeffs, wavelet_type_t wavelet, uint16_t q_format) {
    if (n_input_coeffs < 1) return;

    const int16_t* g0_kernel = NULL;
    const int16_t* g1_kernel = NULL;
    uint8_t kernel_len = 0;
    get_wavelet_coeffs(wavelet, NULL, NULL, &g0_kernel, &g1_kernel, &kernel_len);

    if (n_input_coeffs < (kernel_len >> 1)) return;

    uint16_t output_len = n_input_coeffs * 2;

    memset(output_signal, 0, output_len * sizeof(int16_t));

    for (uint16_t i = 0; i < n_input_coeffs; i++) {
        for (uint8_t j = 0; j < kernel_len; j++) {
            int output_idx = (2 * i - j + output_len) % output_len;
            output_signal[output_idx] += (int16_t)(((int32_t)approx_coeffs[i] * g0_kernel[j]) >> q_format);
        }
    }

    for (uint16_t i = 0; i < n_input_coeffs; i++) {
        for (uint8_t j = 0; j < kernel_len; j++) {
            int output_idx = (2 * i - j + output_len) % output_len;
            output_signal[output_idx] += (int16_t)(((int32_t)detail_coeffs[i] * g1_kernel[j]) >> q_format);
        }
    }
}

void apply_thresholding(int16_t* coeffs, uint16_t length, const wavelet_config_t* config) {
    if (!coeffs || !config || length == 0) return;

    int16_t threshold = config->threshold_value;

    switch (config->threshold_type) {
        case THRESHOLD_HARD:
            for (uint16_t i = 0; i < length; i++) {
                if (abs(coeffs[i]) < threshold) {
                    coeffs[i] = 0;
                }
            }
            break;
        case THRESHOLD_SOFT:
            for (uint16_t i = 0; i < length; i++) {
                if (abs(coeffs[i]) < threshold) {
                    coeffs[i] = 0;
                } else {
                    coeffs[i] = (coeffs[i] > 0) ? (coeffs[i] - threshold) : (coeffs[i] + threshold);
                }
            }
            break;
        case THRESHOLD_ZERO:
            memset(coeffs, 0, length * sizeof(int16_t));
            break;
    }
}

void wavelet_filter(int16_t* signal, uint16_t length, const wavelet_config_t* config) {
    if (!signal || !config || length == 0 || length > MAX_SIGNAL_LENGTH) return;
    if (config->decomposition_levels == 0 || config->decomposition_levels > MAX_DECOMPOSITION_LEVELS) return;

    uint16_t current_n = length;
    uint16_t levels = config->decomposition_levels;

    size_t total_coeffs_size = 0;
    uint16_t temp_n = length;
    for (uint8_t i = 0; i < levels; i++) {
        if (temp_n < 2) break;
        total_coeffs_size += (temp_n >> 1) * 2;
        temp_n >>= 1;
    }

    int16_t* all_coeffs = (int16_t*)malloc(total_coeffs_size * sizeof(int16_t));
    if (!all_coeffs) {
        return;
    }

    int16_t* approx_coeffs_ptrs[MAX_DECOMPOSITION_LEVELS];
    int16_t* detail_coeffs_ptrs[MAX_DECOMPOSITION_LEVELS];

    size_t current_offset = 0;
    temp_n = length;
    for (uint8_t i = 0; i < levels; i++) {
        if (temp_n < 2) break;
        uint16_t half_n = temp_n >> 1;
        approx_coeffs_ptrs[i] = all_coeffs + current_offset;
        detail_coeffs_ptrs[i] = all_coeffs + current_offset + half_n;
        current_offset += half_n * 2;
        temp_n >>= 1;
    }

    int16_t* current_input_signal = signal;

    for (uint8_t i = 0; i < levels; i++) {
        if (current_n < 2) {
            levels = i;
            break;
        }
        uint16_t half_n = current_n >> 1;

        dwt(current_input_signal, approx_coeffs_ptrs[i], detail_coeffs_ptrs[i], current_n, config->wavelet, config->q_format);
        
        current_input_signal = approx_coeffs_ptrs[i];
        current_n = half_n;
    }

    for (uint8_t i = 0; i < levels; i++) {
        apply_thresholding(detail_coeffs_ptrs[i], length >> (i + 1), config);
    }

    int16_t* current_reconstructed_signal = (int16_t*)malloc(current_n * sizeof(int16_t));
    if (!current_reconstructed_signal) {
        free(all_coeffs);
        return;
    }
    memcpy(current_reconstructed_signal, approx_coeffs_ptrs[levels - 1], current_n * sizeof(int16_t));

    uint16_t current_reconstructed_length = current_n;

    for (int8_t i = levels - 1; i >= 0; i--) {
        if (i == 0) { // Last level of reconstruction
            idwt(current_reconstructed_signal, detail_coeffs_ptrs[i], signal, current_reconstructed_length, config->wavelet, config->q_format);
            free(current_reconstructed_signal);
            break; // Exit loop after last reconstruction
        }

        uint16_t next_reconstructed_length = current_reconstructed_length << 1;
        
        int16_t* temp_reconstructed_level = (int16_t*)malloc(next_reconstructed_length * sizeof(int16_t));
        if (!temp_reconstructed_level) {
            free(all_coeffs);
            free(current_reconstructed_signal);
            return;
        }

        idwt(current_reconstructed_signal, detail_coeffs_ptrs[i], temp_reconstructed_level, current_reconstructed_length, config->wavelet, config->q_format);

        free(current_reconstructed_signal);

        current_reconstructed_signal = temp_reconstructed_level;
        current_reconstructed_length = next_reconstructed_length;
    }

    free(all_coeffs);
}
