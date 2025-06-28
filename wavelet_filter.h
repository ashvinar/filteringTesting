/**
 * @file wavelet_filter.h
 * @brief Public API for wavelet-based spike filtering.
 *
 * This file defines the public interface for a fixed-point wavelet filter
 * using Daubechies 4 (db4) wavelets, optimized for resource-constrained
 * embedded systems.
 */

#ifndef WAVELET_FILTER_H
#define WAVELET_FILTER_H

#include <stdint.h>

/**
 * @brief Maximum signal length supported by the filter.
 */
#define MAX_SIGNAL_LENGTH 512

/**
 * @brief Number of decomposition levels for the wavelet transform.
 */
#define DECOMPOSITION_LEVELS 6

/**
 * @brief Main function to perform wavelet-based spike filtering.
 *
 * This function applies a 6-level Daubechies 4 wavelet decomposition,
 * zeros out the approximation coefficients at the final level, and then
 * reconstructs the signal. The filtering is done in-place.
 *
 * @param[in,out] signal Pointer to the signal buffer. The filtered signal
 *                       will be written back to this buffer.
 * @param[in] length The length of the signal in samples. Must be less than
 *                   or equal to MAX_SIGNAL_LENGTH.
 */
void SPIKE_FILTER(int16_t* signal, uint16_t length);

#endif /* WAVELET_FILTER_H */
