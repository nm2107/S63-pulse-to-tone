/**
 * Generates and prints the values of a sinware to use in a lookup table for
 * sinwave generation by Pulse Width Modulation.
 *
 * $ gcc sinwave_lookup_table_generator.c -o slg -lm
 * $ ./slg --help
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#define PROGRAM_NAME "slg"
#define PROGRAM_VERSION "0.1.0"

#define DEFAULT_SAMPLES_COUNT 256 // the number of samples on a sinewawe period (8bit)
#define DEFAULT_VALUES_RANGE 255 // generate values in the range [0 : RANGE] (8bit)
#define DEFAULT_COLUMNS 8 // the amount of columns on which print the results

static unsigned int samples_count, values_range, columns;

static struct option const longopts[] =
{
    {"samples-count", required_argument, NULL, 's'},
    {"values-range", required_argument, NULL, 'r'},
    {"columns", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
};

void usage (int status)
{
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    } else {
        printf("\
Usage: %s [OPTION]...\n\
", PROGRAM_NAME);
        printf("\
\n\
Generates and prints a sinewave period, composed of `samples-count` samples.\n\
The samples are represented by numbers from 0 to `values-range` included.\n\
");

        printf("\
\n\
The options below may be used to select how much samples the sinewave period\n\
should be composed of, and what should be the values range to use.\n\
    -s, --samples-count    The amount of samples the sinewave period is\n\
                           composed of. Defaults to %d.\n\
    -r, --values-range     The value of the [0 : values-range] range in which\n\
                           represent the sample value. Defaults to %d.\n\
                           The value should be greater or equal to 0.\n\
", DEFAULT_SAMPLES_COUNT, DEFAULT_VALUES_RANGE);
        printf("\
\n\
Printing options :\n\
    -c, --columns          The number of columns on which print the results.\n\
                           Defaults to %d. The results are comma separated.\n\
", DEFAULT_COLUMNS);

        printf("\
\n\
Common options :\n\
    --help                 Display this help and exit.\n\
    --version              Output version information and exit.\n\
\n\
");
    }

    exit(status);
}

void compute(unsigned int table[])
{
    unsigned int i = 0;
    double sin_val;

    for (i = 0; i < samples_count; i++)
    {
        sin_val = sin(i * ((2 * M_PI) / samples_count));

        // as a sin val is [-1 : 1], but we're looking for positive values only
        // in order to generate a voltage from a digital output, we apply an
        // offset of 1 to make the value positive.
        sin_val += 1.0;
        // and we scale the value to our range, on order to have [0 : RANGE]
        // final values.
        sin_val = sin_val * (values_range / 2);

        // store as unsigned int
        table[i] = (unsigned int) round(sin_val);
    }
}

void print_on_one_line(unsigned int table[])
{
    unsigned int i;

    for (i = 0; i < samples_count; i++) {
        printf("%d, ", table[i]);
    }

    printf("\n");
}

void print_on_columns(unsigned int table[])
{
    unsigned int i, j, cell_size, number_length, pad_amount;
    char* cr = "";
    char* pad = " ";

    if (0 == values_range) {
        cell_size = 1;
    } else {
        cell_size = floor(log10(abs(values_range))) + 1;
    }

    j = 0;

    for (i = 0; i < samples_count; i++) {
        ++j;

        if (0 == j % columns) {
            cr = "\n";
            j = 0;
        } else {
            cr = "";
        }

        if (0 == table[i]) {
            // log10 not valid on 0 value
            number_length = 1;
        } else {
            number_length = floor(log10(abs(table[i]))) + 1;
        }

        pad_amount = cell_size - number_length;

        printf("%*.*s%d, %s", pad_amount, pad_amount, pad, table[i], cr);
    }
}

void print(unsigned int table[])
{
    if (columns > 1) {
        print_on_columns(table);
    } else {
        print_on_one_line(table);
    }
}

int main (int argc, char **argv) {
    int optc;
    int received_values_range_opt;
    unsigned int* table;

    samples_count = DEFAULT_SAMPLES_COUNT;
    values_range = DEFAULT_VALUES_RANGE;
    columns = DEFAULT_COLUMNS;

    while ((optc = getopt_long(argc, argv, "s:r:c:hv", longopts, NULL)) != -1) {
        switch (optc) {
            case 's':
                samples_count = (unsigned int) atoi(optarg);
                break;

            case 'r':
                received_values_range_opt = atoi(optarg);

                if (!(received_values_range_opt) >= 0) {
                    fprintf(stderr, "\
The values range should be greater or equal to 0, received \"%d\".\n\
", received_values_range_opt);
                    usage(EXIT_FAILURE);
                }

                values_range = (unsigned int) received_values_range_opt;
                break;

            case 'c':
                columns = (unsigned int) atoi(optarg);
                break;

            case 'h':
                usage(EXIT_SUCCESS);
                break;

            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(EXIT_SUCCESS);
                break;

            default:
                usage(EXIT_FAILURE);
        }
    }

    if (!(samples_count > 0)) {
        exit(EXIT_SUCCESS);
    }

    table = (unsigned int*) malloc(samples_count * sizeof(unsigned int));

    compute(table);
    print(table);

    free(table);

    return 0;
}
