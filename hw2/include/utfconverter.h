#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <time.h>

#define MAX_BYTES 4
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2
#define NO_FD -1
#define OFFSET 2

#define FIRST  0
#define SECOND 1
#define THIRD  2
#define FOURTH 3

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

/** The enum for endianness. */
typedef enum {LITTLE, BIG} endianness;

/** The struct for a codepoint glyph. */
typedef struct Glyph {
	unsigned char bytes[MAX_BYTES];
	endianness end;
	bool surrogate;
} Glyph;

static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"UTF", required_argument, 0, 'u'},
		{0, 0, 0, 0}
};


/** The given filename. */
extern char* filename;

/** The usage statement. */
const char* USAGE = { "Command line utility for converting files from UTF-16LE to UTF-16BE or vice versa.\n\nUsage: ./utf [-h|--help] [-v|-vv] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n\n  Option arguments:\n    -h, --help\t    Displays this usage.\n    -v, -vv\t    Toggles the verbosity of the program to level 1 or 2.\n\n  "
};

const char* USAGE2 = {"Mandatory argument:\n    -u OUT_ENC, --UTF=OUT_ENC   Sets the output encoding.\n\t\t\t\tValid values for OUT_ENC: 16LE, 16BE\n\n  Positional Arguments:\n    IN_FILE\t    The file to convert.\n    [OUT_FILE]\t    Output file name. If not present, defaults to stdout.\n"
};
/** Which endianness to convert to. */
extern endianness conversion;

/** Which endianness the source file is in. */
extern endianness source;

/*Handles the writing for UTF 16*/
void read_eight (int fd, unsigned char buf[], Glyph* glyph, int fd_write);

/*Handles the writing for UTF 16*/
void read_sixteen (int fd, unsigned char buf[], Glyph* glyph, int fd_write);

/**
* Prints Verbosity Level 1
*
*/
void print_v1(char* file_path);

/**
* Prints Verbosity Level 2
*
*/
void print_v2(char* file_path);

/**
 * A function that swaps the endianness of the bytes of an encoding from
 * LE to BE and vice versa.
 *
 * @param glyph The pointer to the glyph struct to swap.
 * @return Returns a pointer to the glyph that has been swapped.
 */
Glyph* swap_endianness (Glyph* glyph);

/**
 * Fills in a glyph with the given data in data[2], with the given endianness 
 * by end.
 *
 * @param glyph 	The pointer to the glyph struct to fill in with bytes.
 * @param data[]	The array of data to fill the glyph struct with.
 * @param end	   	The endianness enum of the glyph.
 * @param fd 		The int pointer to the file descriptor of the input 
 * 			file.
 * @return Returns a pointer to the filled-in glyph.
 */
Glyph* fill_glyph (Glyph* glyph, unsigned char data[], endianness end, int* fd);

/**
* A function that converts a UTF-8 glyph to a UTF-16LE or UTF-16BE
* glyph, and returns the result as a pointer to the converted glyph.
*
* @param glyph The UTF-8 glyph to convert.
* @param end The endianness to convert to (UTF-16LE or UTF-16BE).
* @return The converted glyph.
*/
Glyph* convert (Glyph* glyph, endianness end);
/**
 * Writes the given glyph's contents to stdout.
 *
 * @param glyph The pointer to the glyph struct to write to stdout.
 */
void write_glyph (Glyph* glyph, int fd);

/**
 * Calls getopt() and parses arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments as an array of string.
 */
void parse_args (int argc, char** argv);

/**
 * Prints the usage statement.
 */
void print_help (void);

void print_help1 (void);

/**
 * Closes file descriptors and frees list and possibly does other
 * bookkeeping before exiting.
 *
 * @param The fd int of the file the program has opened. Can be given
 * the macro value NO_FD (-1) to signify that we have no open file
 * to close.
 */
void quit_converter (int fd);

void quit_converter1(int fd);
