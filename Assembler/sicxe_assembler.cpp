#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

typedef struct {
    char inst[10];
    int opcode;
}OPTAB;

OPTAB optab[] = {
    {"ADD",0x18}, {"ADDF",0x58}, {"ADDR",0x90}, {"AND",0x40}, {"CLEAR",0xB4}, {"COMP",0x28}, {"COMPF",0x88}, {"COMPR",0xA0}, {"DIV",0x24}, {"DIVF",0x64}, 
    {"DIVR",0x9C}, {"FIX",0xC4}, {"FLOAT",0xC0}, {"HIO",0xF4}, {"J",0x3C}, {"JEQ",0x30}, {"JGT",0x34}, {"JLT",0x38}, {"JSUB",0x48}, {"LDA",0x00}, {"LDB",0x68}, 
    {"LDCH",0x50}, {"LDF",0x70}, {"LDL",0x08}, {"LDS",0x6C}, {"LDT",0x74}, {"LDX",0x04}, {"LPS",0xD0}, {"MUL",0x20},

    {"MULF",0x60}, {"MULR",0x98}, {"NORM",0xC8}, {"OR",0x44}, {"RD",0xD8}, {"RMO",0xAC}, {"RSUB",0x4C}, {"SHIFTL",0xA4}, {"SHIFTR",0xA8}, {"SIO",0xF0}, 
    {"SSK",0xEC}, {"STA",0x0C}, {"STB",0x78}, {"STCH",0x54}, {"STF",0x80}, {"STI",0xD4}, {"STL",0x14}, {"STS",0x7C}, {"STSW",0xE8}, {"STT",0x84}, 
    {"STX",0x10}, {"SUB",0x1C}, {"SUBF",0x5C}, 

    {"SUBR",0x94}, {"SVC",0xB0}, {"TD",0xE0}, {"TIO",0xF8}, {"TIX",0x2C}, {"TIXR",0xB8}, {"WD",0xDC} 
};

const int op_cnt = (int)(sizeof(optab)/sizeof(optab[0]));
bool is_opcode(char *);

typedef struct{
    char name[10];
    int value;
}SYMTAB;

SYMTAB symtab[MAX_LINE_LENGTH];
int sym_cnt = 0;

typedef struct{
    char symbol[10];
    char opcode[10];
    char operand[10];
}INPUT;

INPUT input[MAX_LINE_LENGTH];

int main(int argc, char** argv)
{
    char *r_path;
    char *w_path;
    char line[MAX_LINE_LENGTH];
    unsigned int line_count = 1;

    if(argc < 1)
        return EXIT_FAILURE;

    r_path = argv[1];
    w_path = argv[2];

    /* Open file */
    FILE *r_file = fopen(r_path, "r");
    FILE *w_file = fopen(w_path, "w");

    if(!r_file)
    {
        perror(r_path);
        return EXIT_FAILURE;
    }
    if(!w_file)
    {
        perror(w_path);
        return EXIT_FAILURE;
    }

    char delimit[] = " \t\n";
    char *token;
	char temp1[10], temp2[10], temp3[10];
	int tok_num;
    // Get each line until there are none left
    while(fgets(line, 50, r_file))
    {
		tok_num = 0;
        printf("line[%06d]: %s", line_count, line);
        
		token = strtok(line, delimit);
	
		if(strcmp(token, ".") == 0){
			continue;
		}
		while(token != NULL){
			tok_num++;
			if(tok_num == 1) {
				strcpy(temp1, token);
				printf("temp1 : %s\n",temp1);
			} else if(tok_num == 2) {
				strcpy(temp2, token);
				printf("temp2 : %s\n",temp2);
			} else{
				strcpy(temp3, token);
				printf("temp3 : %s\n",temp3);
			}
			token = strtok(NULL, delimit);
		}
		printf("tok_num is   %d   \n",tok_num);

        if(line[strlen(line)-1] = '\n'){
            printf("\n");
        }

		if(tok_num==1){		// Only RSUB
			strcpy(input[line_count].opcode, temp1);
		}else if(tok_num==2){
			strcpy(input[line_count].opcode, temp1);
			strcpy(input[line_count].operand, temp2);
		}else{
			strcpy(input[line_count].symbol, temp1);
			strcpy(input[line_count].opcode, temp2);
			strcpy(input[line_count].operand, temp3);
		}
        line_count++;
    }

    int LOCCTR;

    if(strcmp(input[1].opcode, "START") == 0){
        LOCCTR = strtol(input[1].operand,NULL,16);
    }else{
        LOCCTR = 0;
    }


    int i=2;
    bool chk_sym;
    // until END
    do{
        chk_sym = true;
        if((int)strlen(input[i].symbol) != 0)
        {
            // 
            for(int j=0;j<sym_cnt;j++){
                if(strcmp(input[i].symbol, symtab[j].name) == 0){
                    chk_sym = false;
                    break;
                }
            }
            if(chk_sym == true){
                strcpy(symtab[sym_cnt].name, input[i].symbol);
                symtab[sym_cnt++].value = LOCCTR;
            }else{
                fputs("duplicate symbol", w_file);
                return EXIT_FAILURE;
            }
        }

        if(is_opcode(input[i].opcode) == true){
            LOCCTR += 3;
        }
        else{
            printf("     not opcode!\n");
        }


        
    }while(strcmp(input[i++].opcode, "END") != 0);




    // print symtable
    for(int j=0;j<sym_cnt;j++){
        printf("%d : symbol : %s, value : %d\n",j, symtab[j].name, symtab[j].value);
    }


    if(fclose(r_file)){
        return EXIT_FAILURE;
        perror(r_path);
    }
    if(fclose(w_file)){
        return EXIT_FAILURE;
        perror(w_path);
    }

    //printf("optable length : %d\n",op_cnt);

}



bool is_opcode(char* str){
    bool result = false;
    if(str[0] == '+'){
        char* temp;
        strcpy(temp, str+1);
        for(int i=0;i<op_cnt;i++){
            if(strcmp(optab[i].inst, temp) == 0){
                result = true;
                break;
            }
        }
    }else{
        for(int i=0;i<op_cnt;i++){
            if(strcmp(optab[i].inst, str) == 0){
                result = true;
                break;
            }
        }
    }
    return result;
}
