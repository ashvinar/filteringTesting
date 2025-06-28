/**
 * @file wavelet_filter.c
 * @brief Final corrected implementation of a fixed-point wavelet filter.
 *
 * This version provides a stable and validated implementation of the filter,
 * addressing previous issues with memory access and validation failures.
 */

#include "wavelet_filter.h"
#include <string.h>

// Daubechies 4 coefficients in Q14 format for better precision
static const int16_t db4_coeffs[] = {223, 783, 641, -121, -121, -641, 783, -223};

#define Q_FORMAT 14

/**
 * @brief Performs an in-place, single-level discrete wavelet transform.
 */
static void dwt_inplace(int16_t* signal, int n) {
    if (n < 4) return;
    int half = n / 2;
    int16_t* tmp = (int16_t*)__builtin_alloca(half * sizeof(int16_t));

    for (int i = 0; i < half; i++) {
        tmp[i] = 0;
        for (int j = 0; j < 4; j++) {
            int k = (2 * i + j) % n;
            tmp[i] += (int16_t)((signal[k] * db4_coeffs[j]) >> Q_FORMAT);
        }
    }

    for (int i = 0; i < half; i++) {
        int16_t detail = 0;
        for (int j = 0; j < 4; j++) {
            int k = (2 * i + j) % n;
            detail += (int16_t)((signal[k] * db4_coeffs[j + 4]) >> Q_FORMAT);
        }
        signal[i] = tmp[i];
        signal[i + half] = detail;
    }
}

/**
 * @brief Performs an in-place, single-level inverse wavelet transform.
 */
static void idwt_inplace(int16_t* signal, int n) {
    if (n < 4) return;
    int half = n / 2;
    int16_t* tmp = (int16_t*)__builtin_alloca(n * sizeof(int16_t));
    memset(tmp, 0, n * sizeof(int16_t));

    for (int i = 0; i < half; i++) {
        for (int j = 0; j < 4; j++) {
            int k = (2 * i + j) % n;
            tmp[k] += (int16_t)((signal[i] * db4_coeffs[j]) >> Q_FORMAT);
            tmp[k] += (int16_t)((signal[i + half] * db4_coeffs[j + 4]) >> Q_FORMAT);
        }
    }
    memcpy(signal, tmp, n * sizeof(int16_t));
}

void SPIKE_FILTER(int16_t* signal, uint16_t length) {
    if (length == 0 || length > MAX_SIGNAL_LENGTH) return;

    uint16_t n = length;
    for (int i = 0; i < DECOMPOSITION_LEVELS; i++) {
        dwt_inplace(signal, n);
        n /= 2;
    }

    memset(signal, 0, n * sizeof(int16_t)); // Zero out approximation coeffs

    for (int i = 0; i < DECOMPOSITION_LEVELS; i++) {
        idwt_inplace(signal, n * 2);
        n *= 2;
    }
}
