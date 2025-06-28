/**
 * @file wavelet_filter.h
 * @brief Public API for an advanced wavelet-based filtering library.
 *
 * This file defines the public interface for a configurable fixed-point
 * wavelet filter. It supports multiple wavelet families (Daubechies, Haar)
 * and thresholding strategies, making it adaptable for various signal
 * processing applications in embedded systems.
 */

#ifndef WAVELET_FILTER_H
#define WAVELET_FILTER_H

#include <stdint.h>

/**
 * @brief Maximum signal length supported by the filter.
 * This can be adjusted based on available memory.
 */
#define MAX_SIGNAL_LENGTH 256

/**
 * @brief Maximum number of decomposition levels.
 */
#define MAX_DECOMPOSITION_LEVELS 8

/**
 * @brief Maximum length of a wavelet coefficient kernel.
 */
#define MAX_WAVELET_KERNEL_LENGTH 12

/**
 * @brief Enumeration for supported wavelet types.
 */
typedef enum {
    WAVELET_DB4,
    WAVELET_DB6,
    WAVELET_HAAR
} wavelet_type_t;

/**
 * @brief Enumeration for supported thresholding strategies.
 */
typedef enum {
    THRESHOLD_HARD,
    THRESHOLD_SOFT,
    THRESHOLD_ZERO // Special case to zero out coefficients
} threshold_type_t;

/**
 * @brief Configuration structure for the wavelet filter.
 *
 * This structure holds all the parameters needed to configure the
 * wavelet filtering process, allowing for flexible and powerful control.
 */
typedef struct {
    wavelet_type_t wavelet;         ///< Wavelet type to use.
    threshold_type_t threshold_type;  ///< Thresholding strategy.
    uint8_t decomposition_levels;   ///< Number of DWT levels.
    int16_t threshold_value;        ///< Threshold for coefficient filtering.
    uint16_t q_format;              ///< Q-format for fixed-point arithmetic.
} wavelet_config_t;

/**
 * @brief Initializes the wavelet filter configuration with default values.
 *
 * @param[out] config Pointer to the configuration structure to initialize.
 */
void wavelet_get_default_config(wavelet_config_t* config);

/**
 * @brief Performs a single-level discrete wavelet transform (DWT).
 *
 * @param[in,out] signal The signal to transform.
 * @param[in] n The length of the signal.
 * @param[in] wavelet The wavelet type to use.
 * @param[in] q_format The Q-format for fixed-point arithmetic.
 */
void dwt(const int16_t* input_signal, int16_t* approx_coeffs, int16_t* detail_coeffs, uint16_t n, wavelet_type_t wavelet, uint16_t q_format);

/**
 * @brief Performs a single-level inverse discrete wavelet transform (IDWT).
 *
 * @param[in,out] signal The signal to reconstruct.
 * @param[in] n The length of the signal.
 * @param[in] wavelet The wavelet type to use.
 * @param[in] q_format The Q-format for fixed-point arithmetic.
 */
void idwt(const int16_t* approx_coeffs, const int16_t* detail_coeffs, int16_t* output_signal, uint16_t output_length, wavelet_type_t wavelet, uint16_t q_format);

/**
 * @brief Applies thresholding to wavelet coefficients.
 *
 * @param[in,out] coeffs The wavelet coefficients to filter.
 * @param[in] length The number of coefficients.
 * @param[in] config The filter configuration.
 */
void apply_thresholding(int16_t* coeffs, uint16_t length, const wavelet_config_t* config);

/**
 * @brief Main function to perform wavelet-based filtering based on a configuration.
 *
 * This function applies a multi-level wavelet decomposition, applies the
 * specified thresholding, and reconstructs the signal. The filtering is
 * done in-place.
 *
 * @param[in,out] signal Pointer to the signal buffer.
 * @param[in] length The length of the signal.
 * @param[in] config The configuration for the filtering process.
 */
void wavelet_filter(int16_t* signal, uint16_t length, const wavelet_config_t* config);

#endif /* WAVELET_FILTER_H */
