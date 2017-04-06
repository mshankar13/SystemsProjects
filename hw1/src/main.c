#include "map_reduce.h"
#include "./map_reduce.c"
#include <string.h>
#include <dirent.h>

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];



//Sample Map function action: Print file contents to stdout and returns the number bytes in the file.
int cat(FILE* f, void* res, char* filename) {
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

int main(int argc, char** argv) {
    
    // Get the validate args 
    int action = validateargs(argc, argv);
    if (action == 0) {
        printf("%s" ,"Usage: ./mapreduce [h|v] FUNC DIR\n");
        printf("%s", "\tFUNC\tWhich operation you would like to run on the data:\n");
        printf("%s", "\t\tana - Analysis of various text files in a directory.\n");
        printf("%s", "\t\tstats - Calculates stats on files which contain only numbers.\n");
        printf("%s", "\tDIR\tThe directory in which the files are located.\n\n");
        printf("%s", "\tOptions:\n");
        printf("%s", "\t-h\tPrints this help menu.\n");
        printf("%s", "\t-v\tPrints the map function's results, stating the file it's from.\n");
        return EXIT_SUCCESS;
    } else if (action == 1) {
        // Analysis is chosen
        int n = nfiles(argv[2]);
        // Get the nbytes read from map calling analysis 
        int numbytes = map(argv[2], analysis_space, sizeof(struct Analysis), analysis); 
        struct Analysis final_struct = analysis_reduce (n, analysis_space);
        analysis_print(final_struct, numbytes, 1);
    } else if (action == 2) {
        // Stats is chosen
        int n = nfiles(argv[2]);
        int check = map(argv[2], stats_space, sizeof(Stats), stats);
        if (check == -1) {
            printf("%s" ,"Usage: ./mapreduce [h|v] FUNC DIR\n");
            printf("%s", "\tFUNC\tWhich operation you would like to run on the data:\n");
            printf("%s", "\t\tana - Analysis of various text files in a directory.\n");
            printf("%s", "\t\tstats - Calculates stats on files which contain only numbers.\n");
            printf("%s", "\tDIR\tThe directory in which the files are located.\n\n");
            printf("%s", "\tOptions:\n");
            printf("%s", "\t-h\tPrints this help menu.\n");
            printf("%s", "\t-v\tPrints the map function's results, stating the file it's from.\n");
            return EXIT_FAILURE;
        }
        Stats final_struct = stats_reduce (n, stats_space);
        stats_print(final_struct, 1);
    } else if (action == 3) {
        // -v analysis
        // Get the number of files
        int n = nfiles(argv[3]);
        // Get the nbytes read from map calling analysis 
        int numbytes = map(argv[3], analysis_space, sizeof(struct Analysis), analysis);
        for (int i = 0; i < n; i++) {
            // Don't print out the histogram yet
            analysis_print(analysis_space[i], numbytes, 0);
        }
        struct Analysis final_struct = analysis_reduce (n, analysis_space);
        analysis_print(final_struct, numbytes, 1);
    } else if (action == 4) {
        // -v stats
        int n = nfiles(argv[3]);
        int check = map(argv[3], stats_space, sizeof(Stats), stats);
        if (check == -1) {
            printf("%s" ,"Usage: ./mapreduce [h|v] FUNC DIR\n");
            printf("%s", "\tFUNC\tWhich operation you would like to run on the data:\n");
            printf("%s", "\t\tana - Analysis of various text files in a directory.\n");
            printf("%s", "\t\tstats - Calculates stats on files which contain only numbers.\n");
            printf("%s", "\tDIR\tThe directory in which the files are located.\n\n");
            printf("%s", "\tOptions:\n");
            printf("%s", "\t-h\tPrints this help menu.\n");
            printf("%s", "\t-v\tPrints the map function's results, stating the file it's from.\n");
            return EXIT_FAILURE;
        }
        for (int i = 0; i < n; i++) {
            // Don't print out the histogram yet
            stats_print(stats_space[i], 0);
        }
        Stats final_struct = stats_reduce (n, stats_space);
        stats_print(final_struct, 1);
    } else {
        printf("%s" ,"Usage: ./mapreduce [h|v] FUNC DIR\n");
        printf("%s", "\tFUNC\tWhich operation you would like to run on the data:\n");
        printf("%s", "\t\tana - Analysis of various text files in a directory.\n");
        printf("%s", "\t\tstats - Calculates stats on files which contain only numbers.\n");
        printf("%s", "\tDIR\tThe directory in which the files are located.\n\n");
        printf("%s", "\tOptions:\n");
        printf("%s", "\t-h\tPrints this help menu.\n");
        printf("%s", "\t-v\tPrints the map function's results, stating the file it's from.\n");
        return EXIT_FAILURE;
    }

}
