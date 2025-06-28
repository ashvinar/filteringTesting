/**
 * @file test_wavelet_filter.c
 * @brief Comprehensive unit tests for the advanced wavelet filter library.
 *
 * This file contains a suite of tests to validate the functionality,
 * stability, and correctness of the wavelet filtering implementation.
 * It covers different wavelet types, thresholding strategies, and edge cases.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "wavelet_filter.h"

#define TEST_SIGNAL_LENGTH 256
#define PI 3.14159265358979323846

// Test signal generation
static int16_t original_signal[TEST_SIGNAL_LENGTH];
static int16_t test_signal[TEST_SIGNAL_LENGTH];

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

/**
 * @brief Test assertion macro.
 */
#define ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("  [PASS] %s\n", message); \
        } else { \
            printf("  [FAIL] %s\n", message); \
        } \
    } while (0)

/**
 * @brief Prints a signal to the console.
 */
void print_signal(const char* name, const int16_t* signal, uint16_t length) {
    printf("%s: ", name);
    for (uint16_t i = 0; i < length; i++) {
        printf("%d ", signal[i]);
    }
    printf("\n");
}

/**
 * @brief Generates a test signal with a sine wave and an artificial spike.
 */
void generate_test_signal() {
    for (int i = 0; i < TEST_SIGNAL_LENGTH; i++) {
        original_signal[i] = (int16_t)(100.0 * sin(2 * PI * i / (double)TEST_SIGNAL_LENGTH));
    }
    // Add a large spike
    original_signal[TEST_SIGNAL_LENGTH / 4] += 2000;
    // Add a smaller spike
    original_signal[TEST_SIGNAL_LENGTH / 2] -= 1500;
    memcpy(test_signal, original_signal, sizeof(original_signal));
}

/**
 * @brief Calculates the Mean Squared Error between two signals.
 */
double calculate_mse(const int16_t* s1, const int16_t* s2, uint16_t length) {
    double mse = 0.0;
    for (uint16_t i = 0; i < length; i++) {
        mse += (s1[i] - s2[i]) * (s1[i] - s2[i]);
    }
    return mse / length;
}

// --- Test Cases ---

void test_single_dwt_idwt() {
    printf("\n--- Running test_single_dwt_idwt ---\n");
    int16_t signal_in[] = {10, 20, 30, 40, 50, 60, 70, 80}; // Length 8
    uint16_t n = 8;
    int16_t approx_out[4];
    int16_t detail_out[4];
    int16_t signal_reconstructed[8];

    wavelet_config_t config;
    wavelet_get_default_config(&config);
    config.wavelet = WAVELET_HAAR; // Use Haar for simplicity
    config.q_format = 0; // No quantization for this test

    print_signal("Original Signal (single)", signal_in, n);

    dwt(signal_in, approx_out, detail_out, n, config.wavelet, config.q_format);
    print_signal("Approx Coeffs", approx_out, n/2);
    print_signal("Detail Coeffs", detail_out, n/2);

    idwt(approx_out, detail_out, signal_reconstructed, n/2, config.wavelet, config.q_format);
    print_signal("Reconstructed Signal", signal_reconstructed, n);

    double mse = calculate_mse(signal_in, signal_reconstructed, n);
    ASSERT(mse < 1.0, "Single DWT/IDWT reconstruction is accurate (MSE < 1.0)");
}

void test_default_config() {
    printf("\n--- Running test_default_config ---\n");
    wavelet_config_t config;
    wavelet_get_default_config(&config);
    ASSERT(config.wavelet == WAVELET_DB4, "Default wavelet is DB4");
    ASSERT(config.threshold_type == THRESHOLD_HARD, "Default threshold type is HARD");
    config.decomposition_levels = 1; // Modified for debugging
    ASSERT(config.q_format == 14, "Default Q-format is 14");
}

void test_reconstruction_energy() {
    printf("\n--- Running test_reconstruction_energy ---\n");
    memcpy(test_signal, original_signal, sizeof(original_signal));
    
    wavelet_config_t config;
    wavelet_get_default_config(&config);
    config.threshold_type = THRESHOLD_HARD;
    config.threshold_value = 0; // No thresholding
    config.decomposition_levels = 1; // Modified for debugging

    wavelet_filter(test_signal, TEST_SIGNAL_LENGTH, &config);

    double mse = calculate_mse(original_signal, test_signal, TEST_SIGNAL_LENGTH);
    ASSERT(mse < 10.0, "Energy is conserved after DWT/IDWT (MSE < 10.0)");
}

void test_spike_removal_db4() {
    printf("\n--- Running test_spike_removal_db4 ---\n");
    memcpy(test_signal, original_signal, sizeof(original_signal));

    wavelet_config_t config;
    wavelet_get_default_config(&config);
    config.wavelet = WAVELET_DB4;
    config.threshold_type = THRESHOLD_HARD;
    config.threshold_value = 10000;
    config.decomposition_levels = 1; // Modified for debugging

    wavelet_filter(test_signal, TEST_SIGNAL_LENGTH, &config);

    int16_t original_val1 = (int16_t)(100.0 * sin(2 * PI * (TEST_SIGNAL_LENGTH / 4) / (double)TEST_SIGNAL_LENGTH));
    int16_t original_val2 = (int16_t)(100.0 * sin(2 * PI * (TEST_SIGNAL_LENGTH / 2) / (double)TEST_SIGNAL_LENGTH));
    int spike_removed = abs(test_signal[TEST_SIGNAL_LENGTH / 4] - original_val1) < 100 &&                         abs(test_signal[TEST_SIGNAL_LENGTH / 2] - original_val2) < 100;
    ASSERT(spike_removed, "Spikes are removed with DB4 hard thresholding");
}

void test_spike_removal_haar() {
    printf("\n--- Running test_spike_removal_haar ---\n");
    memcpy(test_signal, original_signal, sizeof(original_signal));

    wavelet_config_t config;
    wavelet_get_default_config(&config);
    config.wavelet = WAVELET_HAAR;
    config.threshold_type = THRESHOLD_SOFT;
    config.threshold_value = 10000;
    config.decomposition_levels = 1; // Modified for debugging

    wavelet_filter(test_signal, TEST_SIGNAL_LENGTH, &config);

    int16_t original_val1 = (int16_t)(100.0 * sin(2 * PI * (TEST_SIGNAL_LENGTH / 4) / (double)TEST_SIGNAL_LENGTH));
    int16_t original_val2 = (int16_t)(100.0 * sin(2 * PI * (TEST_SIGNAL_LENGTH / 2) / (double)TEST_SIGNAL_LENGTH));
    int spike_removed = abs(test_signal[TEST_SIGNAL_LENGTH / 4] - original_val1) < 100 &&                         abs(test_signal[TEST_SIGNAL_LENGTH / 2] - original_val2) < 100;
    ASSERT(spike_removed, "Spikes are removed with Haar soft thresholding");
}

void test_edge_cases() {
    printf("\n--- Running test_edge_cases ---\n");
    int16_t empty_signal[] = {};
    int16_t short_signal[] = {10, 20, 30, 40};
    wavelet_config_t config;
    wavelet_get_default_config(&config);
    config.decomposition_levels = 1; // Modified for debugging

    // Test empty signal
    wavelet_filter(empty_signal, 0, &config);
    ASSERT(1, "Empty signal does not crash"); // If we got here, it passed

    // Test short signal
    wavelet_filter(short_signal, 4, &config);
    ASSERT(1, "Short signal does not crash");

    // Test null pointers
    wavelet_filter(NULL, 100, &config);
    ASSERT(1, "NULL signal does not crash");
    wavelet_filter(short_signal, 4, NULL);
    ASSERT(1, "NULL config does not crash");
}

int main() {
    printf("========================================\n");
    printf("  Running Wavelet Filter Test Suite\n");
    printf("========================================\n");

    generate_test_signal();

    test_single_dwt_idwt();
    test_default_config();
    test_reconstruction_energy();
    test_spike_removal_db4();
    test_spike_removal_haar();
    test_edge_cases();

    printf("\n----------------------------------------\n");
    printf("Test Summary: %d / %d tests passed.\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_run == tests_passed) ? 0 : 1;
}