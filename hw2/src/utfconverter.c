#include "utfconverter.h"

char* filename = NULL;
char* fileout = "";
endianness source;
endianness conversion;
char* encoding = NULL;
char verbosity = 0;
int v_counter = 0;
char* fullname = NULL;
int surrogate_ctr = 0;
int glyph_ctr = 0;
int new_line = 0;
int ascii_ctr = 0;
int write_to = 0;
Glyph* gref = NULL;
int fdref = 0;
int writeref = 0;

clock_t read_start = 0;
clock_t read_end = 0;
clock_t write_start = 0;
clock_t write_end = 0;
clock_t convert_start = 0;
clock_t convert_end = 0;

/*Variables for times*/
struct tms read_start1;
struct tms read_end1;
struct tms write_start1;
struct tms write_end1;
struct tms convert_start1;
struct tms convert_end1;
/*-------------------*/

 
int main(int argc, char** argv) {
	char file_path [120];
	int fd = 0;
	Glyph* glyph = malloc(sizeof(Glyph)); 
	unsigned char buf[2] = {0, 0};
	unsigned char eight_check = 0;
	unsigned char bom[2] = {0, 0};
	unsigned char eight_buf[4] = {0, 0, 0, 0};
	int rv = 0;
	int fd_write = 0;
	mode_t mode = S_IRWXU | S_IRUSR | S_IRGRP | S_IROTH;
	gref = glyph;
	encoding = NULL;
	memset(glyph, 0, sizeof(Glyph));
	/* After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);

	memset(file_path, 0, sizeof(file_path));

	if (strcmp(filename, fileout) == 0) {
		print_help1();
	}

	fd = open(filename, O_RDONLY); 
	fdref = fd; 
	/*Invalid fd*/
	if (fd == -1) {
		close(fd);
		print_help1();
	}
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	read_start = times(&read_start1);
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) { 
		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			source = LITTLE; 
			encoding = "16LE";
		} else if(buf[0] == 0xfe && buf[1] == 0xff) {
			/*file is big endian*/
			source = BIG;
			encoding = "16BE";
		} else if ((rv = read(fd, &eight_check, 1)) == 1) {
			/*Check if there is a third byte then check for correct BOM for UTF 8*/
			if (buf[0] == 0xef && buf[1] == 0xbb && eight_check == 0xbf) {
				/*Handle BOM*/
				encoding = "8";
			} else {
				/*Not the right BOM. Print USAGE and exit*/
				close(fd);
				print_help1();
			}
		} else {
			/*file has no BOM*/
			close(fd);
			print_help1();
		}
		
		memset(glyph, 0, sizeof(Glyph));
	} else {
		/*Empty file*/ 
		close(fd);
		print_help1();
	}

	/* Now deal with the rest of the bytes.*/
	/*Either UTF 8 or UTF 16*/

	fd_write = open(fileout, O_RDWR | O_CREAT | O_APPEND, mode);
	writeref = fd_write;
	if (fd_write == -1) {
		fd_write = STDOUT_FILENO;
	}

	if (fd_write != STDOUT_FILENO) {
			/*File exists so check edge cases*/
		if((rv = read(fd_write, &buf[0], 1)) == 1 && (rv = read(fd_write, &buf[1], 1)) == 1) { 
			/*Read in first two bytes since you're expecting a UTF 16 file to write to*/
			if(buf[0] == 0xff && buf[1] == 0xfe){
				/*file is UTF16LE*/
				/*Check to see if conversion is LITTLE*/
				if (conversion == LITTLE) {
					/*Loop through original file and convert properly to the proper value for UTF 16*/
					new_line = 1;	
					if (strcmp(encoding, "8") == 0) {
						read_eight (fd, eight_buf, glyph, fd_write);
					} else {
						read_sixteen (fd, buf, glyph, fd_write);
					}
					/*If appropriate then add and new line and then begin appending*/
				} else {
					close(fd);
					close(fd_write);
					print_help1();
				}

			} else if(buf[0] == 0xfe && buf[1] == 0xff) {
				/*file is UTF16BE*/
				/*Check to see if conversion is BIG*/
				if (conversion == BIG) {
					/*Loop through original file and convert properly to the proper value for UTF 16*/
					new_line = 1;
					if (strcmp(encoding, "8") == 0) {
						read_eight (fd, eight_buf, glyph, fd_write);
					} else {
						read_sixteen (fd, buf, glyph, fd_write);
					}
				} else {
					close(fd);
					close(fd_write);
					print_help1();
				}

			} else {
				/*File BOM is not valid*/
				close(fd);
				close(fd_write);
				print_help1();
			}

		} else {
			if ((rv = read(fd_write, &buf[0], 1)) == 1) {
				/*Only on byte in the file. Invalid*/
				close(fd);
				close(fd_write);
				print_help1();
			} else {
				/*Nothing in the file or new file! Can write freely*/
				/*Or stdout?*/
				/*Include BOM*/ 
				if (conversion == LITTLE) {
					bom[0] = 0xff;
					bom[1] = 0xfe;
				} else {
					bom[0] = 0xfe;
					bom[1] = 0xff;
				}
				write_start = times(&write_start1);

				write(fd_write, bom, NON_SURROGATE_SIZE);

				if (strcmp(encoding, "8") == 0) {
					read_eight (fd, eight_buf, glyph, fd_write);
				} else {
					read_sixteen (fd, buf, glyph, fd_write);
				}
			}
		}
	} else {
	/*Nothing in the file or new file! Can write freely*/
		/*Or stdout?*/
		/*Include BOM*/ 
		if (conversion == LITTLE) {
			bom[0] = 0xff;
			bom[1] = 0xfe;
		} else {
			bom[0] = 0xfe;
			bom[1] = 0xff;
		}
		write_start = times(&write_start1);

		write(fd_write, bom, NON_SURROGATE_SIZE);

		if (strcmp(encoding, "8") == 0) {
			read_eight (fd, eight_buf, glyph, fd_write);
		} else {
			read_sixteen (fd, buf, glyph, fd_write);
		}
	}
	 

	/*Check for verbosity and print output is necessary*/
	if (v_counter == 1){
		/*Level 1 Verbosity*/
		print_v1(filename);
	} else if (v_counter > 1) {
		/*Level 2 Verbosity*/
		print_v2(filename);
	} else {
		/*Verbosity is 0*/
	}

	quit_converter(NO_FD);
	return 0;
}

void read_eight (int fd, unsigned char eight_buf[], Glyph* glyph, int fd_write) {
	/*Take necessary steps to read correct number of bytes from the file at a time*/
	int rv = 0;
	unsigned int bits = 0;
	int marker = 0;
	unsigned char val = 0;
	int counter = 1;
	int index = 1;
	memset(glyph, 0, sizeof(Glyph));
	/*Check if anything can be read from the UTF 8 file*/
	if (new_line == 1) {

			if (write_start == 0) {
				write_start = times(&write_start1);
			}

			/*Insert newline into file*/
			if (fd_write != -1) {
				write(fd_write, "\n", 2);
			} else {
			}
			
	}
	while ((rv = read(fd, &eight_buf[0], 1)) == 1) {
		index = 1;
		counter = 1;
		memset(glyph, 0, sizeof(Glyph));
		glyph->bytes[0] = eight_buf[0];
		val = eight_buf[0];
		bits = val >> 7;
		if (bits == 0) {
			marker = 1;
		} else {
			if ((bits = val >> 5) == 6) {
				marker = 2;
			} else if ((bits = val >> 4) == 14) {
				marker = 3;
			} else if ((bits = val >> 3) == 30){
				marker = 4;
			} else {
				/*Not valid UTF 8 encoding*/
				close(fd);
				close(fd_write);
				print_help1();
			}
		}

		/*Loop through to get bytes needed and fill glyph with UTF 8*/
		while (counter != marker) {
			if((rv = read(fd, &eight_buf[index], 1)) == 1) {
				glyph->bytes[index] = eight_buf[index];
				counter++;
				index++;
			} else {
				/*Incomplete bytes so printhelp*/
				close(fd);
				close(fd_write);
				print_help1();
			}
		}
		/*Convert to specified UTF 16 and print. Write to it. Reloop.*/
		write_glyph(convert(glyph, conversion), fd_write);

	}
	write_end = times(&write_end1);
	read_end = times(&read_end1);
}

void read_sixteen (int fd, unsigned char buf[], Glyph* glyph, int fd_write) {
	int rv = 0;
	memset(glyph, 0, sizeof(Glyph));
	if (new_line == 1) {

		if (write_start == 0) {
				write_start = times(&write_start1);
		}

		/*Insert newline into file*/
		write(fd_write, "\n", 2);
	}
	while((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) {
		if (conversion == source) {
			write_glyph(fill_glyph(glyph, buf, source, &fd), fd_write);
		} else{
			write_glyph(swap_endianness(fill_glyph(glyph, buf, source, &fd)), fd_write);
		}
		memset(glyph, 0, sizeof(Glyph));
	        
	}
	write_end = times(&write_end1);
	read_end = times(&read_end1);
}


void print_v1(char* file_path) {
	struct stat buffer;
	int status = 0;
	double size = 0;
	char host [120];
	char* convert = NULL;
	struct utsname buf;
	char * file_p = NULL;
	char b[PATH_MAX + 1];
	/* Call stats and get the info to print for the file*/
	status = stat(file_path, &buffer);
	fflush(stderr);
	fflush(stdout);
	if (status == 0) {
		/* SUCCESS! */
		size = (double)(buffer.st_size) / 1000;
		fprintf(stderr, "Input file size: %1f kb\n", size);

		file_p = realpath(filename, b);
		if (file_p) {
			fprintf(stderr, "Input file path: %s\n", b);
		} else {
			close(fdref);
			close(writeref);
			print_help1();
		}
		fprintf(stderr, "Input file encoding: UTF-%s\n", encoding);
		if (conversion == BIG) {
			convert = "BE";
		} else {
			convert = "LE";
		}
		fprintf(stderr, "Output encoding: UTF-16%s\n", convert);
		gethostname(host, sizeof(host));
		fprintf(stderr, "Hostmachine: %s\n", host);
		uname(&buf);
		fprintf(stderr, "Operating System: %s\n", buf.sysname);
	} else {
		close(fdref);
		close(writeref);
		print_help1();
	}
}

void print_v2(char* file_path) {
	struct stat buffer;
	double temp = 0;
	int temp1 = 0;
	double t1 = 0;
	double t2 = 0;
	double t3 = 0;
	double dp = 0;
	double sur = 0;
	double clocks = (double) CLOCKS_PER_SEC;
	int status = 0;
	double size = 0;
	char host [120];
	char* convert = NULL;
	struct utsname buf;
	char * file_p = NULL;
	char b[PATH_MAX + 1];
	/* Call stats and get the info to print for the file*/
	status = stat(file_path, &buffer);
	fflush(stderr);
	if (status == 0) {
		/* SUCCESS! */
		size = (buffer.st_size);
		fprintf(stderr, "Input file size: %1f kb\n", size);
		file_p = realpath(filename, b);
		if (file_p) {
			fprintf(stderr, "Input file path: %s\n", b);
		} else {
			close(fdref);
			close(writeref);
			print_help1();
		}
		fprintf(stderr, "Input file encoding: UTF-%s\n", encoding);
		if (conversion == BIG) {
			convert = "BE";
		} else {
			convert = "LE";
		}
		fprintf(stderr, "Output encoding: UTF-16%s\n", convert);
		gethostname(host, sizeof(host));
		fprintf(stderr, "Hostmachine: %s\n", host);
		uname(&buf);
		fprintf(stderr, "Operating System: %s\n", buf.sysname);
		t1 = ((double)read_end / clocks) - ((double)read_start / clocks);
		t2 = ((double)read_end1.tms_utime / clocks) - ((double)read_start1.tms_utime / clocks);
		t3 = ((double)read_end1.tms_stime / clocks) - ((double)read_start1.tms_stime / clocks);
		fprintf(stderr, "Reading: real:%1f, user:%1f, sys:%1f\n", t1, t2, t3);
		t1 = ((double)convert_end / clocks) - ((double)convert_start / clocks);
		t2 = ((double)convert_end1.tms_utime / clocks) - ((double)convert_start1.tms_utime / clocks);
		t3 = ((double)convert_end1.tms_stime / clocks) - ((double)convert_start1.tms_stime / clocks);
		fprintf(stderr, "Converting: real:%1f, user:%1f, sys:%1f\n", t1, t2, t3);
		t1 = ((double)write_end / clocks) - ((double)write_start / clocks);
		t2 = ((double)write_end1.tms_utime / clocks) - ((double)write_start1.tms_utime / clocks);
		t3 = ((double)write_end1.tms_stime / clocks) - ((double)write_start1.tms_stime / clocks);
		fprintf(stderr, "Writing: real:%1f, user:%1f, sys:%1f\n", t1, t2, t3);
		glyph_ctr++;
		dp = ((double)ascii_ctr / (double)glyph_ctr) * 100;
		temp1 = (int) dp;
		temp = dp - (double) temp1;
		if (temp > 0 && temp >= 0.5) {
			dp++;
		}
		fprintf(stderr, "ASCII: %d%%\n", (int)dp);
		sur = ((double)surrogate_ctr / (double)glyph_ctr) * 100;
		temp1 = (int) sur;
		temp = sur - (double) temp1;
		if (temp > 0 && temp >= 0.5) {
			sur++;
		}
		fprintf(stderr, "Surrogates: %d%%\n", (int)sur);
		fprintf(stderr, "Glyphs: %d\n", glyph_ctr);

	} else {
		close(fdref);
		close(writeref);
		print_help1();
	}
}

Glyph* swap_endianness(Glyph* glyph) {
	unsigned char temp;
	convert_start = times(&convert_start1);
	temp = glyph->bytes[0];
	glyph->bytes[0] = glyph->bytes[1];
	glyph->bytes[1] = temp;
	if(glyph->surrogate){  /* If a surrogate pair, swap the next two bytes. */
		temp = glyph->bytes[2];
		glyph->bytes[2] = glyph->bytes[3];
		glyph->bytes[3] = temp;
	}
	glyph->end = conversion;
	convert_end = times(&convert_end1);
	return glyph;
}

Glyph* fill_glyph(Glyph* glyph, unsigned char data[], endianness end, int* fd) {
	unsigned int bits = 0; 
	glyph->bytes[0] = data[0];
	glyph->bytes[1] = data[1];
	bits = (data[FIRST] + (data[SECOND] << 8));
	convert_start = times(&convert_start1);
	/* Check high surrogate pair using its special value range.*/
	if(bits > 0xD800 && bits < 0xDBFF) { 
		if(read(*fd, &data[SECOND], 1) == 1 && read(*fd, (&data[FIRST]), 1) == 1) {
			bits = 0;
			bits = (data[FIRST] + (data[SECOND] << 8));
			if(bits > 0xDC00 && bits < 0xDFFF) { /* Check low surrogate pair.*/
				glyph->surrogate = true; 
				surrogate_ctr++;
			} else {
				lseek(*fd, -OFFSET, SEEK_CUR); 
				glyph->surrogate = false;
			}
		}
	}

	if (glyph->surrogate == true) {
		surrogate_ctr++;
	}
	if(!glyph->surrogate){
		glyph->bytes[THIRD] = glyph->bytes[FOURTH] |= 0;
	} else {
		glyph->bytes[THIRD] = data[FIRST]; 
		glyph->bytes[FOURTH] = data[SECOND];
	}
	glyph->end = end;
	convert_end = times(&convert_end1);
	return glyph;
}

Glyph* convert (Glyph* glyph, endianness end) {
	int temp = 0;
	int code_pt = 0;
	unsigned int bits = 0;
	int marker = 0;
	unsigned char val = glyph->bytes[FIRST];

	int v = 0;
	int l = 0;
	int h = 0;
	int x = 0x10000;
	int w1 = 0;
	int w2 = 0;
	temp = 0;
	convert_start = times(&convert_start1);
	bits = val >> 7;
	if (bits == 0) {
		marker = 1;
	} else {
		if ((bits = val >> 5) == 6) {
			marker = 2;
		} else if ((bits = val >> 4) == 14) {
			marker = 3;
		} else if ((bits = val >> 3) == 30){
			marker = 4;
		} else {
			/*Not valid UTF 8 encoding*/
			close(fdref);
			close(writeref);
			print_help1();
		}
	}

	/*Convert UTF 8 to code point - also UTF BE*/
	if (marker == 1) {
		code_pt = glyph->bytes[0];
	} else if (marker == 2) {
		temp = glyph->bytes[1];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		code_pt |=temp;

		temp = glyph->bytes[0];
		temp &= 0x1f;
		temp <<= 6;
		code_pt |= temp;
	} else if (marker == 3) {
		temp = glyph->bytes[2];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		code_pt |=temp;

		temp = glyph->bytes[1];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		temp <<= 6;
		code_pt |=temp;

		temp = glyph->bytes[0];
		/*Mask the last 6 bits*/
		temp &= 0x0f;
		temp <<= 12;
		code_pt |=temp;
	} else {
		temp = glyph->bytes[3];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		code_pt |=temp;

		temp = glyph->bytes[2];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		temp <<= 6;
		code_pt |=temp;

		temp = glyph->bytes[1];
		/*Mask the last 6 bits*/
		temp &= 0x3f;
		temp <<= 12;
		code_pt |=temp;

		temp = glyph->bytes[0];
		/*Mask the last 6 bits*/
		temp &= 0x07;
		temp <<= 18;
		code_pt |=temp;
	}

	/*Now the code_pt is UTF 16 BE*/
	if (code_pt > 0x10000) {
		v = code_pt - x;
		/*Make surrogate true*/
		glyph->surrogate = true;
		surrogate_ctr++;
		/*Shift to split code point into the x pieces of each byte of our surrogate pair*/
		/*Higher 10 bits*/
		h = v >> 10;
		/*Lower 10 bits*/
		l = v & 0x3ff;

		/*Create the code unit w1 and w2*/
		w1 = 0xd800 + h;
		w2 = 0xdc00 + l;
		/*Now have the code units for UTF 16*/

		/*Put into glyph*/
		if (end == BIG) {
			temp = w1;
			temp &= 0xFF00;
			temp >>= 8;
			glyph->bytes[0] = temp;

			temp = w1;
			temp &= 0x00FF;
			glyph->bytes[1] = temp;

			temp = w2;
			temp &= 0xFF00;
			temp >>= 8;
			glyph->bytes[2] = temp;
			
			temp = w2;
			temp &= 0x00FF;
			glyph->bytes[3] = temp;

		} else {
			temp = w1;
			temp &= 0x00FF;			
			glyph->bytes[0] = temp;

			temp = w1;
			temp &= 0xFF00;
			temp >>= 8;
			glyph->bytes[1] = temp;

			temp = w2;
			temp &= 0x00FF;
			glyph->bytes[2] = temp;

			temp = w2;
			temp &= 0xFF00;
			temp >>= 8;
			glyph->bytes[3] = temp;
		}

	} else {
		/*Can put into glyph*/
		/*Mask top 2 bytes???*/
		if (end == LITTLE) {
			temp = 0;

			temp = code_pt << 24;
			temp >>= 24;
			glyph->bytes[0] = temp;

			temp = code_pt << 16;
			temp >>= 24;
			glyph->bytes[1] = temp;

			temp = code_pt << 8;
			temp >>= 24;
			glyph->bytes[2] = temp;	

			temp = code_pt >> 8;
			glyph->bytes[3] = temp;					
		} else {
			temp = code_pt << 24;
			temp >>= 24;
			glyph->bytes[1] = temp;

			temp = code_pt << 16;
			temp >>= 24;
			glyph->bytes[0] = temp;

			temp = code_pt << 8;
			temp >>= 24;
			glyph->bytes[3] = temp;	

			temp = code_pt >> 8;
			glyph->bytes[2] = temp;	
		}
	}
	convert_end = times(&convert_end1);
	return glyph;

}

void write_glyph(Glyph* glyph, int fd) {
	/*Check to see if glyph byte is valid ascii*/
	int temp = glyph->bytes[0];
	int temp1 = glyph->bytes[1];
	if (0x00 <= temp && temp1 == 0x00) {
		if (temp <= 0x7F) {
			ascii_ctr++;
		}
	} else if (temp == 0x00 && 0x00 <= temp1) {
		if (temp1 <= 0x7F) {
			ascii_ctr++;
		}
	} else {
		/*Do nothing*/
	}

	if (write_start == 0) {
		write_start = times(&write_start1);
	}
	glyph_ctr++;
	if(glyph->surrogate){
		write(fd, glyph->bytes, SURROGATE_SIZE);
	} else {
		write(fd, glyph->bytes, NON_SURROGATE_SIZE);
	}
}

void parse_args(int argc, char** argv) {
	int option_index = 0;
	int c = 0;
	char* endian_convert = NULL;
	/*Edit here to support different file types*/
	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	while((c = getopt_long(argc, argv, "hvu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'u':
				endian_convert = optarg;
				break;
			case 'v': 
				v_counter++;
				break;
			case 'h':
				print_help();
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				/*quit_converter(NO_FD);*/
				print_help1();
				break;
		}

	}

	if(optind < argc){
		filename = argv[optind];
	} else {
		fprintf(stderr, "Filename not given.\n");
		print_help1();
	}

	/*Go to next argv value that is not an option*/
	if ((optind + 1) < argc) {
		/*Check for the file output name*/
		fileout = argv[optind + 1];
	} else {
		/*No output file specified*/
	}

	/*Handle extra output file or crap at end!*/

	if(endian_convert == NULL){
		fprintf(stderr, "Conversion mode not given.\n");
		print_help1();
	}

	if((strcmp(endian_convert, "16LE") == 0) || (strcmp(endian_convert, "--UTF=16LE") == 0)){ 
		conversion = LITTLE;
	} else if((strcmp(endian_convert, "16BE") == 0) || (strcmp(endian_convert, "--UTF=16BE") == 0)){
		conversion = BIG;
	} else {
		print_help1();
	}
}

void print_help(void) {
	free(gref);
	printf("%s", USAGE);
	printf("%s", USAGE2); 
	quit_converter(NO_FD);
}

void print_help1(void) {
	free(gref);
	printf("%s", USAGE);
	printf("%s", USAGE2); 
	quit_converter1(NO_FD);
}

void quit_converter(int fd) {
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD) {
		close(fd);
	}
	exit(EXIT_SUCCESS);
}

void quit_converter1(int fd) {
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD) {
		close(fd);
	}
	exit(EXIT_FAILURE);
}
