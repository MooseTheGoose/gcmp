#include "gcmp_io.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_CONTENTS 0x1000000

char *read_file(char *fname)
{
    FILE *fp = fopen(fname, "r");
    char *contents;

    if(fp)
    {
        long bytesRead;
        contents = malloc(MAX_CONTENTS + 1);
        assert(contents != NULL);
        bytesRead = fread(contents, 1, MAX_CONTENTS, fp);
        contents[bytesRead] = '\0';
        fclose(fp);        
    }
    else
    {
        fprintf(stderr, "Error opening file %s in read_file()\n", fname);
        exit(-1);
    }

    return contents;
}