//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "map_reduce.h"

//Implement map_reduce.h functions here.

// Validates the command line agruments passed in by the user
// @param argc The number of arguments
// @param argv The arguments
// @return Returns -1 if arguments are invalid
//          Returns 0 if -h optional flag is selected.
//          Returns 1 if analysis is chosen
//          Returns 2 if stats is chosen.
//          If the -v optional flag has been selected, validateargs returns 3 if analysis is chosen
//          4 is stats is chosen
int validateargs(int argc, char** argv) {

    // Check to see if there is any input
    if (argc != 0 && argc != 1) {
    	// Check to see if the second argument if arc = 2 is -h (only valid input)
    	if (argc >= 2) {
    		// Check for -h. Only valid argument for position
    		if (strcmp(argv[1], "-h") == 0) {
    			return 0;
    		} else if (argc >= 3) {	
	    		// Check for FUNC
	    		if (strcmp(argv[1], "ana") == 0) {
	    			DIR* dirptr = opendir(argv[2]);
	    			// Check to see if DIR is valid
	    			if (dirptr == NULL) {
	    				return -1;
	    			} else {
	    				// Valid DIR
	    				return 1;
	    			}
	    		// Check for other FUNC
	    		} else if (strcmp(argv[1], "stats") == 0) {
	    			DIR* dirptr = opendir(argv[2]);
	    			// Check to see if DIR is valid
	    			if (dirptr == NULL) {
	    				return -1;
	    			} else {
	    				// Valid DIR
	    				return 2;
	    			}
	    		}
	    		// Check with greater than 4 arguments
	    		if (argc >= 4) {
	    			if (strcmp(argv[1], "-v") == 0) {
	    				if (strcmp(argv[2], "ana") == 0) {
    						DIR* dirptr = opendir(argv[3]);
    						// Check to see if DIR is valid
    						if (dirptr == NULL) {
    							return -1;
    						} else {
    							// Valid DIR
    							return 3;
    						}
    					} else if (strcmp(argv[2], "stats") == 0) {
    						DIR* dirptr = opendir(argv[3]);
    						// Check to see if DIR is valid
    						if (dirptr == NULL) {
    							return -1;
    						} else {
    							// Valid DIR
    							return 4;
    						}
    					} else {
    						return -1;
    					} 	
	    			} else {
	    				return -1;
	    			}
	    		} else {
	    			return -1;
	    		}
			} else {
				return -1;
			}	
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

// Counts the number of files in a directory EXCLUDING . and ..
// @param dir The directory for which number of files is desired
// @return The number of files in directory EXCLUDING . and ..
//          If nfiles returns 0, then print "No files present in the
//          directory." and the program should return EXIT_SUCCESS.
//          Returns -1 if any sort of failure or error occurs
int nfiles(char* dir) {
    // dir = array of characters with a null terminator
    // Check to see if opening the directory works
    // have a pointer to the directory
    // Keep track of the number of files
    int fileNum = 0;
    DIR* dirptr = opendir(dir);
    struct dirent* rdptr;
    // Check to see if the directory name is valid
    if (dirptr == NULL) {
        // Directory not found
        return 0;
    } else {
        // Directory found
        //rdptr = readdir(dirptr);
        // Loop through while there are files to be read
        // Each call to readdir returns the next entry
        while ((rdptr = readdir(dirptr)) != NULL) {
            // Check to see if the file is regular
            if (rdptr -> d_type == DT_REG) {
            // exclude . and ..
                if ((strcmp(rdptr -> d_name, ".") == 0) || (strcmp(rdptr -> d_name, "..") == 0)) {    
                } else {
                    fileNum++;
                }
            } else {
                //Do nothing
            }
        }     
    }
    closedir(dirptr);
    // Return the number of files
    return fileNum;
}

// @param dir The directory that was specified by the user
// @param results The space where map can store the result for each file
// @param size The size of struct containing result data for each file
// @param act The action that map will perform on each file
// @return The map function returns -1 on failure, sum of act results on success
int map(char* dir, void* results, size_t size, int (*act) (FILE* f, void* res, char* fn)) {
    // Open the directory to work on
    DIR* dirptr = opendir(dir);
    struct dirent* rdptr;
    FILE* filestream;
    int act_results = 0;
    void* res = results;
    struct Analysis* ana_ptr = NULL;
    Stats* stats_ptr = NULL;

    if(sizeof(Stats) == size) {
    	stats_ptr = res;
    } else {
    	ana_ptr = res;
    }
    // Check to see if directory is valid
    if (dirptr == NULL) {
        // Directory not found
        return -1;
    } else {
        // Directory found
        // rdptr = readdir(dirptr);
        // Loop through while there are files to be read
        // Each call to readdir returns the next entry
        // 1. for each file in the directory
        while ((rdptr = readdir(dirptr)) != NULL) {
            // 2. get full path of file
        	if ((strcmp(rdptr -> d_name, ".") == 0) || (strcmp(rdptr -> d_name, "..") == 0)) {
        		// If the filename is . or .. then do nothing and reloop
        	} else {

                // get the length of the filename
                size_t path_length = strlen(dir) + strlen(rdptr->d_name) + 1;
                // Check to see is slash is last thing on dir
                if(dir[strlen(dir) - 1] == '/') {
                	// Do nothing
                } else {         	
                	path_length++;	
                }
                // 1 accounts for null terminator
                char file_path [path_length];
                memset(file_path, 0, path_length);
                // Concatenate the filename and filepath
                strcat(file_path, dir);
                if(dir[strlen(dir) - 1] == '/') {
                	// Do nothing
                } else {         	
                	strcat(file_path, "/");		
                }
                strcat(file_path, rdptr->d_name);
            	// 3. Open file
                filestream = fopen(file_path, "r");
                // Check if the file can be opened
                if (filestream == NULL) {
                	return -1;
                } else {
	                // What happens if file doesn't exist
	                // What is you don't have permission to access the file?

	            	// 4. Perform some action and store result
	                // make res for storing function results for file
	                //memset(res, 0, size);
	                // make res a char pointer     
	                //res = (char*) res; //should this not be here?
	                // Clear space up to size for the result of the function called
	                if (sizeof(Stats) == size) {
	                	// If the size equals Stats
	                	//stats_ptr = res;
	                	memset(stats_ptr->histogram, 0, sizeof(stats_ptr->histogram));
	                	stats_ptr->sum = 0;
	                	stats_ptr->n = 0;
	                	stats_ptr->filename = NULL;
	                } else {
	                	// If the size equals Analysis
	                	//ana_ptr = res;
	                	memset(ana_ptr->ascii, 0, sizeof(ana_ptr->ascii));
	                	ana_ptr->lnlen = 0;
	                	ana_ptr->lnno = 0;
	                	ana_ptr->filename = NULL;
	                }
	                // @param f filestream on which the action will be performed
	                // @param res The slot in the results array in which the data will be stored
	                // @param filename The filename of the file currently being processed
	                
	                // Increment res
	                if (ana_ptr != NULL) {
	                	act_results += act(filestream, ana_ptr, rdptr->d_name); 
	                	// If the struct is analysis
	                	ana_ptr++;
	                } else {
	                	act_results += act(filestream, stats_ptr, rdptr->d_name); 
	                	// The struct is stats
	                	stats_ptr++;
	                }
            	}
            	// 5. Close the file
            	fclose(filestream);
            	// Reloop
       		}
    	}
    	return act_results;
    }

}

// Takes the results produced by map and calculates all the data to give one final Analysis struct
// Final struct should contain filename of file which has longest line
// @param n The number of files analyzed
// @param results The results array that has been populated by map
// @return The struct containing all the cumulative data
struct Analysis analysis_reduce(int n, void* results) {
    // 1. for each result in results
    // 2. total += results

	// Dereference results and cast it to struct Analysis
	struct Analysis* res = results;
    // Final struct containing cumulated data
    struct Analysis final_struct;
    // Pointer to memory address where memset should begin
    int* ptr = final_struct.ascii;
    // Clear the memory
    memset(ptr, 0, sizeof(int) * 128);
    // Keep track of the filename with the longest line
    char* file_name = NULL;
    // Keep track of the previous line length number
    int line_num = 0;
    // Keep track of the previoud line length
    int line_length = 0;
    
    // Loop through the number of files analyzed = size of the results array
    for (int i = 0;i < n; i++) {
    	// Need to look through the int ascii array and cumulate all instances of the ascii characters
    	for (int j = 0; j < 128; j++) {
    		final_struct.ascii[j] += res[i].ascii[j];
    	}
    	// Keep track of the line lengths (which one is higher)
    	if(res[i].lnlen >= line_length) {
    		// Save new longest length
    		line_length = res[i].lnlen;
			// Keep track of line number associated with the longest line length
    		line_num = res[i].lnno;
    		// Keep track of the filename associated with the longest line length
    		file_name = res[i].filename;
    	} else {
    		// Do Nothing
    	}	
    }

    final_struct.filename = file_name;
    final_struct.lnno = line_num;
    final_struct.lnlen = line_length;

    return final_struct;
}

//
Stats stats_reduce(int n, void* results) {
	// 1. for each result in results
    // 2. total += results

	// Casting results and cast it to struct Stats
	Stats* res = results;
	// Final struct containing cumulated data
	Stats final_struct;
    // Pointer to memory address where memset should begin
    int* ptr = final_struct.histogram;
    // Clear the memory
    memset(ptr, 0, sizeof(int)* NVAL);
    final_struct.sum = 0;
  	final_struct.n = 0;	 
  	final_struct.filename = NULL;
    // Keep track of the filename with the longest line
    
    // Loop through the number of files analyzed = size of the results array
    for (int i = 0;i < n; i++) {
    	// Need to look through the int histogram array and cumulate all instances of the ascii characters
    	for (int j = 0; j < NVAL; j++) {
    		final_struct.histogram[j] += res[i].histogram[j];
    	}
  			final_struct.sum += res[i].sum;
  			final_struct.n += res[i].n;	   	
    }
    // Filename field in the final struct should be set to NULL
	

	// Issue? Does this not exist when the function returns? This place in mem is not reliable at that point
    return final_struct;

}

// @param res The final result returned by analysis_reduce
// @param nbytes The number of bytes in the directory
// @param hist If this is non-zero, prints additional information
void analysis_print(struct Analysis res, int nbytes, int hist) {
	if (nbytes != 0) {
		// Print the filename
    	printf("File: %s\n", res.filename);
    	// Print the longest line in the directory's length
    	printf("Longest line length: %d\n", res.lnlen);
    	// Print the longest line in the directory's line number
    	printf("Longest line number: %d\n", res.lnno);
	} else {
		// Print the filename
    	printf("File: %s\n", "-");
    	// Print the longest line in the directory's length
    	printf("Longest line length: %s\n", "-");
    	// Print the longest line in the directory's line number
    	printf("Longest line number: %s\n", "-");
    	printf("Total Bytes in directory: %d\n", 0);
	}
	

    // When to print final results??? For nbytes???
    // Check to see if hist is non zero
    if (hist != 0 && nbytes != 0) {
    	printf("Total Bytes in directory: %d\n", nbytes);
    	printf("%s\n", "Histogram:");
    	for (int i = 0; i < 128; i++) {
    		if (res.ascii[i] != 0) {
    			printf("%d:", i);
    			for (int j = 0; j < res.ascii[i]; j++) {
    				printf("%s", "-");
    			}
    			printf("\n");
    		} else {
    			// Do nothing
    		}
    	}
    } else {
    	//printf("Total Bytes in directory: %d\n", nbytes);
    	printf("\n");
    }

}

// @param res The final result returned by stats_reduce
// @param hist If this is non-zero, prints additional information
void stats_print(Stats res, int hist) {
	// Total count of values in the file
	int count = 0;
	// Total of the values summed together
	double total = 0;
	// Keeps track of the highest value in the histogram
	int highest_value = 0;
	// Median
	double median = 0;
	// Min value in the file
	int min = 0;
	// Max value in the file
	int max = 0;
	// Keep track of the number of distinct values in the array
	int num_values = 0;
	double Q1 = 0;
	double Q3 = 0;

	// Check to see if hist can be printed
	if (hist != 0 && res.n != 0) {
		printf("%s\n", "Histogram:");
		// Loop through the stats histogram
		for (int i = 0; i < NVAL; i++) {
			if(res.histogram[i] != 0) {	
				// Print the start of the histogram row
				printf("%d :", i);
				for (int j = 0; j < res.histogram[i]; j++) {
					printf("%s", "-");
				}
				printf("\n");
			} else {
				// Do nothing
			}
		}
		printf("\n");
	}
	// Put the values into an array to easily use/manage for calculations
	// Values are already sorted
	// Max and Min already determined
	for (int i = 0; i < NVAL; i++) {
		if(res.histogram[i] != 0) {
			if (res.histogram[i] > highest_value) {
					highest_value = res.histogram[i];
				} else {
					// Do nothing
				}
			// Increment the number of distinct values counter
			num_values++;
			for (int j = 0; j < res.histogram[i]; j++) {
				count ++;
			}
			total += ((double)res.histogram[i]) * ((double) i);
		}
	}
	int values [count];
	int* values_ptr = values;
	// Clear the array
	memset (values_ptr, 0, sizeof(int) * count);

	int mode_index = 0;
	int mode [num_values];
	int* mode_ptr = mode;
	// Clear the array
	memset (mode_ptr, 0, sizeof(int) * num_values);	

	int values_index = 0;
	for (int i = 0; i < NVAL; i++) {
		if(res.histogram[i] != 0) {
			// Check for max
			if (i > max) {
				// Set max to i
				max = i;
			} else {
				// Do nothing
			}

			// Check for values that might be the mode
			if (res.histogram[i] == highest_value) {
				// Save the mode number in the array of mode values
				mode [mode_index] = i;
				mode_index++;
			}

			// Loop through each value in the index
			for (int j = 0; j < res.histogram[i]; j++) {
					values[values_index] = i;
					// Increment the index to store 
					values_index++;
				}
			// There is something in the values array so the first index
			// since the array is sorted will have the min value
			min = values[0];
		}
	}


	//  Calculate Mean
	double mean = 0;
	// Check to see if the file has any values
	if (count != 0) {
		mean = total / (double) count;
	} else {
		mean = 0;
	}

	// Calculate Median
	if (count != 0) {
		// Calculated index of the median
		double calc_index = ((double)count + 1) / 2;
		// Is Count Odd? 
		if (count % 2 == 1) {
			// List of values is odd
			int index = (int) calc_index;
			median = (double)values[index - 1];
		} else {
			// List of values is even
			// Rounds the double down
			// Index on the right of middle
			int index = (int) calc_index;
			int index_1 = index - 1;
			// Index on left of middle
			median = ((double) values[index] + (double) values[index_1]) / 2;
		}
		// Calculate Q1 & Q3
			double np = 0;
			np = ((double) count) * 0.25;

			int temp_num1 = (int) np;
			double sub = np - (double)temp_num1;
			if (sub == 0) {
				// Do nothing
			} else {
				// Round up!
				temp_num1++;
			}

			Q1 = values[temp_num1 - 1];

			np = ((double) count) * 0.75;

			int temp_num2 = (int) np;
			double sub1 = np - (double)temp_num1;
			if (sub1 == 0) {
				// Do nothing
			} else {
				// Round up!
				temp_num2++;
			}

			Q3 = values[temp_num2 - 1];
	} else {
		// Do nothing Median already initialized 
	}


	if (res.n == 0){
		printf("%s", "File: ");
		printf("%s\n", "-");

		printf("%s", "Count: ");
		printf("%d\n", 0);

		printf("%s", "Mean: ");
		printf("%s\n", "-");

		printf("%s", "Mode: ");
		printf("%s\n", "-");

		printf("%s", "Median: ");
		printf("%s\n", "-");

		printf("%s", "Q1: ");
		printf("%s\n", "-");

		printf("%s", "Q3: ");
		printf("%s\n", "-");

		printf("%s", "Min: ");
		printf("%s\n", "-");

		printf("%s", "Max: ");
		printf("%s\n", "-");

		if (hist == 0) {
			printf("\n");
		}
	} else {

		if (hist == 0) {
			printf("%s", "File: ");
			printf("%s\n", res.filename);
		}
		printf("%s", "Count: ");
		printf("%d\n", count);

		printf("%s", "Mean: ");
		printf("%1f\n", mean);

		printf("%s", "Mode: ");
		for (int m = 0; m < mode_index; m ++) {
			printf("%d ", mode[m]);
		}
		printf("\n");

		printf("%s", "Median: ");
		printf("%1f\n", median);

		printf("%s", "Q1: ");
		printf("%1f\n", Q1);

		printf("%s", "Q3: ");
		printf("%1f\n", Q3);

		printf("%s", "Min: ");
		printf("%d\n", min);

		printf("%s", "Max: ");
		printf("%d\n", max);

		if (hist == 0) {
			printf("\n");
		}
	}

}

// Calculates the total number of bytes in the file
// Stores the longest line length and the line number and the frequencies of ASCII characters in the file
// @param f Filestream
// @param res The slot in the results array in which the data will be stored
// @param filename The filename of the file currently being processed.
// @return The number of bytes read
int analysis (FILE* f, void* res, char* filename) {
	// At this point the file has been opened and awaiting work to be done on it
	// Loop through and read the file char by char
	struct Analysis* results = res;
	int total_bytes = 0;
	int lnlen = 0;
	int lnno = 0;
	int lnno_temp = 0;
	int lnlen_temp = 0;
	int c = 0;
	while ((c = fgetc(f)) != EOF) {
		// Add the the ascii value count in the array
		results -> ascii[c] = results -> ascii[c] + 1;
		// Does the value equal a new line character?
		if (c != 10) {
			// increment the line length temp
			lnlen_temp++;
		} else {
			// Character read is a new line characer
			// Increment the number of lines in the file
			lnno_temp++;
			if (lnlen < lnlen_temp) {
				lnlen = lnlen_temp;
				lnno = lnno_temp;
			}
			lnlen_temp = 0;
		}
		// Keep track of the total number of bytes
		total_bytes++;
	}
	results -> filename = filename;
	results -> lnno = lnno;
	results -> lnlen = lnlen;

	return total_bytes;
}

// Counts the number of occurrences of each number in a file
// Calculates the sum total of all numbers in the file and how many numbers are in the file
// @param f filestream
// @param res slot in the results array in which the data will be stored
// @param filename the filename
// @return Return 0 on success and -1 on failure
int stats (FILE* f, void* res, char* filename) {
	Stats* results = res;
	// Total sum of all numbers
	int sum = 0;
	// How many numbers are in the file
	int n = 0;
	// Keep track of the number read
	int c = 0;

	while((fscanf(f, "%d", &c)) != EOF) {
		// Check to see if the integer is valid
		if (c >= 0 && c < NVAL) {
			sum += c;
			n++;
			results -> histogram[c] = results -> histogram[c] + 1;
		} else {
			return -1;
		}
	}

	results -> sum = sum;
	results -> n = n;
	results -> filename = filename;
	return 0;
}
