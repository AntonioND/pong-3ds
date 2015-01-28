
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//--------------------------------------------------------------------

TTF_Font * font;
uint32_t char_buffer[300][300]; // X, Y, RGBA
int font_maxy_global = 0;
int font_miny_global = 0;

void TextInit(const char * path, int size)
{
    if(TTF_Init()==-1)
    {
        printf("TTF_Init() error: %s\n\n", TTF_GetError());
        exit(1);
    }
    atexit(TTF_Quit);

    font = TTF_OpenFont(path,size);
    if(font == NULL)
    {
        printf("TTF_OpenFont() error: %s\n\n", TTF_GetError());
        exit(2);
    }

    int i;
    for(i = 0; i < 256; i++)
    {
        int maxy,miny;
        if(TTF_GlyphMetrics(font,i,NULL,NULL,&miny,&maxy,NULL)==-1)
        {
            printf("TTF_GlyphMetrics() error: %s\n\n", TTF_GetError());
            exit(4);
        }
        if(maxy > font_maxy_global) font_maxy_global = maxy;
        if(miny < font_miny_global) font_miny_global = miny;
    }

    printf("Min/Max : %d/%d\n\n",font_miny_global,font_maxy_global);
}

int TextGetCharWidth(char c)
{
    char text[2] = { c, '\0' };
    int _w;
    TTF_SizeText(font,text,&_w,NULL);
    return _w;
}

int TextGetAscent(void)
{
    return font_maxy_global;
}

int TextGetDescent(void)
{
    return font_miny_global;
}

void TextDrawChar(char c)
{
    char text[2] = { c, '\0' };

    SDL_Color color = {0,0,0, 255};
    SDL_Color bg = {255,255,255, 255};
    SDL_Surface * text_surface;
    if( !( text_surface = TTF_RenderText_Shaded(font,text,color,bg) ) )
    {
        printf("TTF_RenderText_Shaded() error: %s\n\n", TTF_GetError());
        return;
    }

    SDL_Surface * surface = SDL_ConvertSurfaceFormat(text_surface,SDL_PIXELFORMAT_ABGR8888,SDL_SWSURFACE);
    SDL_FreeSurface(text_surface);

//char name[100];sprintf(name,"%d.bmp",c);
//SDL_SaveBMP(surface,name);

    memset(char_buffer,0xFF,sizeof(char_buffer));

    int maxy, miny;
    if(TTF_GlyphMetrics(font,c,NULL,NULL,&miny,&maxy,NULL)==-1)
    {
        printf("TTF_GlyphMetrics() error: %s\n\n", TTF_GetError());
        exit(4);
    }

    int yoffset = TTF_FontAscent(font) - TextGetAscent();

    int i,j;
    for(j = 0; (j+yoffset) < surface->h; j++) for(i = 0; i < surface->w; i++)
    {
        char_buffer[j][i] = *(uint32_t*)&(((uint8_t*)surface->pixels)[surface->pitch*(j+yoffset)+i*4]);
    }

    SDL_FreeSurface(surface);
}

uint32_t TextGetPixel(int x, int y)
{
    return char_buffer[y][x];
}

//--------------------------------------------------------------------

void Draw_Font(const char * font_path, const int font_size, const char * output_path)
{
    TextInit(font_path,font_size);
    printf("\nFont loaded from:\n   %s\n   Size = %d\n\n",font_path,font_size);

    int hmax = TextGetAscent() - TextGetDescent();
    //int hmax = TTF_FontAscent(font) - TTF_FontDescent(font) + 1;

    int wmax = 0;
    int i;
    for(i = 0; i < 256; i++)
    {
        int w = TextGetCharWidth(i);
        if(w > wmax) wmax = w;
    }

    printf("Max char font size: %dx%d\n\n",wmax,hmax);

    FILE * f = fopen(output_path,"wb");
    if(f == NULL)
    {
        printf("Couldn't open output file!\n\n");
        exit(3);
    }

    uint32_t dummy = 0xFFFFFFFF;
    for(i = 0; i < 256; i++) // Reserve space
        fwrite(&dummy,1,sizeof(dummy),f);

    int glyph_present[256];
    memset(glyph_present,0,sizeof(glyph_present));

    uint32_t glyph_file_offset[256];
    memset(glyph_file_offset,0xFF,sizeof(glyph_file_offset)); //0xFFFFFFFF = not present

    uint32_t current_file_offset = 256; // 256 pointers

    // FILE = 256 OFFSETS | GLYPH0 | GLYPH1 ...

    for(i = 0; i < 256; i++) // GLYPH = WIDTH, HEIGHT, DATA...
    {
        glyph_present[i] = TTF_GlyphIsProvided(font,i);
        printf("%d",glyph_present[i]!=0);
        if((i&31) == 31) printf("\n");

        if(glyph_present[i])
        {
            glyph_file_offset[i] = current_file_offset;

            int w = TextGetCharWidth(i);
            TextDrawChar(i);

            unsigned int w_ = w, h_ = hmax;
            fwrite(&w_,1,sizeof(w_),f);
            fwrite(&h_,1,sizeof(h_),f);
            current_file_offset += 2;

#define FLIP_3DS

            int x,y;
#ifdef FLIP_3DS
            for(x = 0; x < w; x++)
            {
                for(y = hmax-1; y >= 0; y--)
#else
            for(y = 0; y < hmax; y++)
            {
                for(x = 0; x < w; x++)
#endif // FLIP_3DS
                {
                    uint32_t px = TextGetPixel(x,y);
                    fwrite(&px,1,sizeof(px),f);
                    current_file_offset ++;
                }
            }
        }
    }

    rewind(f);

    for(i = 0; i < 256; i++)
    {
        uint32_t off = glyph_file_offset[i];
        fwrite(&off,1,sizeof(off),f);
    }

    fclose(f);
}

//--------------------------------------------------------------------

int main(int argc, char * argv[])
{
    if(argc < 4)
    {
        printf("\nUsage: GenFontTTF [font_path.ttf] [font_size] [output_file.c]\n\n");
        return 1;
    }

    if(SDL_Init(0)) atexit(SDL_Quit);

    Draw_Font(argv[1],atoi(argv[2]),argv[3]);

    printf("Done!\n\n");

    return 0;
}

//--------------------------------------------------------------------
