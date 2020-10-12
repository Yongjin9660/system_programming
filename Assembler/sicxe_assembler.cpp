#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

struct OPTAB{
    char Mnemonic[7];
    int opcode;
};

optab[] = {
    {"ADD",0x18}, {"ADDF",0x58}, {"ADDR",0x90}, {"AND",0x40}, {"CLEAR",0xB4}, {"COMP",0x28}, {"COMPF",0x88}, {"COMPR",0xA0}, {"DIV",0x24}, {"DIVF",0x64}, 
    {"DIVR",0x9C}, {"FIX",0xC4}, {"FLOAT",0xC0}, {"HIO",0xF4}, {"J",0x3C}, {"JEQ",0x30}, {"JGT",0x34}, {"JLT",0x38}, {"JSUB",0x48}, {"LDA",0x00}, {"LDB",0x68}, 
    {"LDCH",0x50}, {"LDF",0x70}, {"LDL",0x08}, {"LDS",0x6C}, {"LDT",0x74}, {"LDX",0x04}, {"LPS",0xD0}, {"MUL",0x20},

    {"MULF",0x60}, {"MULR",0x98}, {"NORM",0xC8}, {"OR",0x44}, {"RD",0xD8}, {"RMO",0xAC}, {"RSUB",0x4C}, {"SHIFTL",0xA4}, {"SHIFTR",0xA8}, {"SIO",0xF0}, 
    {"SSK",0xEC}, {"STA",0x0C}, {"STB",0x78}, {"STCH",0x54}, {"STF",0x80}, {"STI",0xD4}, {"STL",0x14}, {"STS",0x7C}, {"STSW",0xE8}, {"STT",0x84}, 
    {"STX",0x10}, {"SUB",0x1C}, {"SUBF",0x5C}, 

    {"SUBR",0x94}, {"SVC",0xB0}, {"TD",0xE0}, {"TIO",0xF8}, {"TIX",0x2C}, {"TIXR",0xB8}, {"WD",0xDC} 
};



int main(int argc, char** argv)
{
    char *path;
    char *w_path;
    char line[MAX_LINE_LENGTH] = {0};
    unsigned int line_count = 0;

    if(argc < 1)
        return EXIT_FAILURE;

    path = argv[1];
    w_path = argv[2];

    /* Open file */
    FILE *file = fopen(path, "r");
    FILE *w_file = fopen(w_path, "w");

    if(!file)
    {
        perror(path);
        return EXIT_FAILURE;
    }
    if(!w_file)
    {
        perror(w_path);
        return EXIT_FAILURE;
    }

    // Get each line until there are none left
    while(fgets(line, MAX_LINE_LENGTH, file))
    {
        printf("line[%06d]: %s",++line_count, line);
        

        if(line[strlen(line)-1] = '\n'){
            printf("\n");
        }

        int l = fputs(line, w_file);
    }

    if(fclose(file)){
        return EXIT_FAILURE;
        perror(path);
    }
    if(fclose(w_file)){
        return EXIT_FAILURE;
        perror(w_path);
    }

}

