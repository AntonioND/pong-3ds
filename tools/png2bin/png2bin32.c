
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../png/png_utils.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        return 1;
    }

    char * filename = argv[1];

    char * buf;
    int w,h;

    if(Read_PNG(filename, &buf, &w, &h))
    {
        return 1;
    }

    char outname[4000];
    strcpy(outname,filename);
    int l = strlen(outname);
    outname[l-4] = '_';
    outname[l] = '.';
    outname[l+1] = 'b';
    outname[l+2] = 'i';
    outname[l+3] = 'n';
    outname[l+4] = '\0';

    FILE * fout = fopen(outname,"wb");
    if(fout)
    {
        int i,j;
        for(i = 0; i < w; i++) for(j = h-1; j >= 0; j--) // transform image to blit easier
        {
            unsigned char r = buf[(j*w+i)*4+0];
            unsigned char g = buf[(j*w+i)*4+1];
            unsigned char b = buf[(j*w+i)*4+2];
            unsigned char a = buf[(j*w+i)*4+3];

            fwrite(&a,1,1,fout);
            fwrite(&b,1,1,fout);
            fwrite(&g,1,1,fout);
            fwrite(&r,1,1,fout);
        }

        fclose(fout);
    }

    free(buf);

	return 0;
}
