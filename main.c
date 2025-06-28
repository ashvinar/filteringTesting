/**
 * @file main.c
 * @brief Demonstration of the advanced wavelet filtering library.
 *
 * This file provides an example of how to use the configurable wavelet
 * filter. It generates a test signal, applies different filter
 * configurations (e.g., different wavelets and thresholding), and
 * prints the results to showcase the library's capabilities.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "wavelet_filter.h"

#define SIGNAL_LENGTH 256
#define PI 3.14159265358979323846

// Signal buffers
static int16_t original_signal[SIGNAL_LENGTH];
static int16_t filtered_signal[SIGNAL_LENGTH];

/**
 * @brief Generates a sine wave with two spikes.
 */
void generate_demo_signal() {
    printf("Generating signal with spikes...\n");
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        original_signal[i] = (int16_t)(150.0 * sin(4 * PI * i / (double)SIGNAL_LENGTH));
    }
    original_signal[SIGNAL_LENGTH / 4] += 2500; // Large positive spike
    original_signal[3 * SIGNAL_LENGTH / 4] -= 2000; // Large negative spike
}

/**
 * @brief Prints a signal to the console.
 */
void print_signal(const char* name, const int16_t* signal, uint16_t length) {
    printf("\n--- %s ---\n", name);
    for (uint16_t i = 0; i < length; i++) {
        printf("%d ", signal[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

/**
 * @brief Runs a demonstration of a specific filter configuration.
 */
void run_filter_demo(const char* demo_name, const wavelet_config_t* config) {
    printf("==================================================\n");
    printf("Running Demo: %s\n", demo_name);
    printf("==================================================\n");

    // Copy original signal for filtering
    memcpy(filtered_signal, original_signal, sizeof(original_signal));

    // Apply the filter
    wavelet_filter(filtered_signal, SIGNAL_LENGTH, config);

    // Print results
    print_signal("Filtered Signal", filtered_signal, SIGNAL_LENGTH);
}

int main() {
    // Generate the base signal for all demos
    generate_demo_signal();
    print_signal("Original Signal", original_signal, SIGNAL_LENGTH);

    // --- Demo 1: Classic Spike Filtering (like original implementation) ---
    wavelet_config_t config1;
    wavelet_get_default_config(&config1);
    config1.wavelet = WAVELET_DB4;
    config1.threshold_type = THRESHOLD_ZERO; // Zero out approximation coefficients
    config1.decomposition_levels = 5;
    run_filter_demo("Classic Spike Filter (DB4, Zero Approx)", &config1);

    // --- Demo 2: Hard Thresholding for Denoising ---
    wavelet_config_t config2;
    wavelet_get_default_config(&config2);
    config2.wavelet = WAVELET_DB6;
    config2.threshold_type = THRESHOLD_HARD;
    config2.threshold_value = 400;
    config2.decomposition_levels = 6;
    run_filter_demo("Denoising (DB6, Hard Threshold)", &config2);

    // --- Demo 3: Soft Thresholding for Denoising ---
    wavelet_config_t config3;
    wavelet_get_default_config(&config3);
    config3.wavelet = WAVELET_HAAR;
    config3.threshold_type = THRESHOLD_SOFT;
    config3.threshold_value = 300;
    config3.decomposition_levels = 7;
    run_filter_demo("Denoising (Haar, Soft Threshold)", &config3);

    printf("\nAll demonstrations complete.\n");

    return 0;
}