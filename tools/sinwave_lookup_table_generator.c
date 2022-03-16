/**
 * Generates and prints the values of a sinware to use in a lookup table for
 * sinwave generation by Pulse Width Modulation.
 *
 * $ gcc sinwave_lookup_table_generator.c -o sinwave_lut_gen -lm
 * $ ./sinwave_lut_gen
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLES_COUNT 256 // the number of samples on a sinewawe period (8bit)
#define VALUES_RANGE 255 // generate values in the range [0 : RANGE] (8bit)

void compute(unsigned int table[]);
void print(unsigned int table[]);

void compute(unsigned int table[])
{
    unsigned int i = 0;
    double step = (2 * M_PI) / SAMPLES_COUNT;
    double sin_val;

    for (i = 0; i < SAMPLES_COUNT; i++)
    {

        sin_val = sin(i * step);

        // as a sin val is [-1 : 1], but we're looking for positive values only
        // in order to generate a voltage from a digital output, we apply an
        // offset of 1 to make the value positive.
        sin_val += 1.0;
        // and we scale the value to our range, on order to have [0 : RANGE]
        // final values.
        sin_val = sin_val * (VALUES_RANGE / 2);

        // store as unsigned int
        table[i] = (unsigned int) round(sin_val);
    }
}

void print(unsigned int table[])
{
    unsigned int i, j;
    const unsigned int columns_count = 8;
    unsigned int cell_size = floor(log10(abs(VALUES_RANGE))) + 1;
    unsigned int number_length;
    unsigned int pad_amount;
    char* cr = "";
    char* pad = " ";

    j = 0;

    for (i = 0; i < SAMPLES_COUNT; i++) {
        ++j;

        if (0 == j % columns_count) {
            cr = "\n";
            j = 0;
        } else {
            cr = "";
        }

        if (0 == table[i]) {
            number_length = 1;
        } else {
            // log10 not valid on 0 value
            number_length = floor(log10(abs(table[i]))) + 1;
        }

        pad_amount = cell_size - number_length;

        printf("%*.*s%d, %s", pad_amount, pad_amount, pad, table[i], cr);
    }
}

int main (void) {
    unsigned int table[SAMPLES_COUNT];

    compute(table);
    print(table);

    return 0;
}
