/**
 * @file main.c
 * @brief Example usage of the wavelet spike filter.
 *
 * This file demonstrates how to use the SPIKE_FILTER function to filter a
 * sample signal. It initializes a raw signal, applies the wavelet filter,
 * and prints the original and filtered signals.
 */

#include <stdio.h>
#include <stdint.h>
#include "wavelet_filter.h"

// Sample raw signal with a spike
static int16_t raw_signal[MAX_SIGNAL_LENGTH] = {
    10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 500, 90, 80, 70, 60, 50, 40, 30, 20, 10
};

/**
 * @brief Prints the contents of a signal.
 *
 * @param[in] name The name of the signal to print.
 * @param[in] signal The signal data.
 * @param[in] length The length of the signal.
 */
void print_signal(const char* name, const int16_t* signal, uint16_t length) {
    printf("%s:\n", name);
    for (uint16_t i = 0; i < length; i++) {
        printf("%d ", signal[i]);
    }
    printf("\n\n");
}

int main() {
    uint16_t signal_length = 20;

    // Print the original signal
    print_signal("Original Signal", raw_signal, signal_length);

    // Apply the wavelet spike filter
    SPIKE_FILTER(raw_signal, signal_length);

    // Print the filtered signal
    print_signal("Filtered Signal", raw_signal, signal_length);

    return 0;
}
