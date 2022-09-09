#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MaxExt 50

int extentionCount = 0, fileCount=0;
char *extensions[MaxExt];

static int display_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    if(tflag == FTW_F){
        char *fileExtension = strrchr(fpath + ftwbuf->base, '.');
        if (fileExtension){
            fileExtension++;
            for(int i=0; i<extentionCount; i++){
                if(!strcmp(extensions[i], fileExtension)){
                    fileCount++;
                    printf("%-32s  %s\n", fpath + ftwbuf->base,fpath);
                    break;
                }     
            }
        } 
    }
    return 0;           /* To tell nftw() to continue */
}

int main(int argc, char *argv[])
{
    if(argc==1) {
        printf("No file extensions provided in argument\n");
        exit(0);
    }

    char *targetDirectory;

    if(!strcmp(argv[argc-2],"-t")){
        extentionCount = argc - 3;
        targetDirectory = argv[argc-1];
    }
    else {
        extentionCount = argc - 1;
        targetDirectory = ".";
    }
        
    for(int j=0; j<extentionCount; j++)
        extensions[j] = argv[j+1];

    if (nftw(targetDirectory, display_info, 20, 0) == -1){
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    if(fileCount==0)
        printf("No file exists with given extension.\n");
    exit(EXIT_SUCCESS);
}