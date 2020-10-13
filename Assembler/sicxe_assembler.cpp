#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

typedef struct {
    char inst[10];
    int opcode;
    int format;
}OPTAB;

OPTAB optab[] = {
    {"ADD",0x18, 3}, {"ADDF",0x58,3}, {"ADDR",0x90,2}, {"AND",0x40,3}, {"CLEAR",0xB4,2}, {"COMP",0x28,3}, {"COMPF",0x88,3}, 
    {"COMPR",0xA0,2}, {"DIV",0x24,3}, {"DIVF",0x64,3}, {"DIVR",0x9C,2}, {"FIX",0xC4,1}, {"FLOAT",0xC0,1}, {"HIO",0xF4,1}, 
    {"J",0x3C,3}, {"JEQ",0x30,3}, {"JGT",0x34,3}, {"JLT",0x38,3}, {"JSUB",0x48,3}, {"LDA",0x00,3}, {"LDB",0x68,3}, 
    {"LDCH",0x50,3}, {"LDF",0x70,3}, {"LDL",0x08,3}, {"LDS",0x6C,3}, {"LDT",0x74,3}, {"LDX",0x04,3}, {"LPS",0xD0,3}, {"MUL",0x20,3},

    {"MULF",0x60,3}, {"MULR",0x98,2}, {"NORM",0xC8,1}, {"OR",0x44,3}, {"RD",0xD8,3}, {"RMO",0xAC,2}, {"RSUB",0x4C,3}, 
    {"SHIFTL",0xA4,2}, {"SHIFTR",0xA8,2}, {"SIO",0xF0,1}, {"SSK",0xEC,3}, {"STA",0x0C,3}, {"STB",0x78,3}, {"STCH",0x54,3}, {"STF",0x80,3}, 
    {"STI",0xD4,3}, {"STL",0x14,3}, {"STS",0x7C,3}, {"STSW",0xE8,3}, {"STT",0x84,3}, 
    {"STX",0x10,3}, {"SUB",0x1C,3}, {"SUBF",0x5C,3}, 

    {"SUBR",0x94,2}, {"SVC",0xB0,2}, {"TD",0xE0,3}, {"TIO",0xF8,1}, {"TIX",0x2C,3}, {"TIXR",0xB8,2}, {"WD",0xDC,3} 
};

const int op_cnt = (int)(sizeof(optab)/sizeof(optab[0]));

typedef struct{
    char name[10];
    int value;
}SYMTAB;

SYMTAB symtab[MAX_LINE_LENGTH];
int sym_cnt = 0;
int format;
unsigned int line_count = 1;

typedef struct{
    int loc;
    char symbol[10];
    char opcode[10];
    char operand[10];
}INPUT;

INPUT input[MAX_LINE_LENGTH];


bool is_opcode(char *);
bool is_opcode2(char* ,int*);
bool write_imediate();

int main(int argc, char** argv)
{
    char *r_path;
    char line[MAX_LINE_LENGTH];

    if(argc < 1)
        return EXIT_FAILURE;

    r_path = argv[1];

    /* Open file */
    FILE *r_file = fopen(r_path, "r");

    if(!r_file)
    {
        perror(r_path);
        return EXIT_FAILURE;
    }

    char delimit[] = " \t\n";
    char *token;
	char temp1[10], temp2[10], temp3[10], temp4[10];
	int tok_num;

    // Get each line until there are none left
    while(fgets(line, 50, r_file))
    {
		tok_num = 0;
        
		token = strtok(line, delimit);
	
		if(strcmp(token, ".") == 0){
			continue;
		}
		while(token != NULL){
			tok_num++;
			if(tok_num == 1) {
				strcpy(temp1, token);
			} else if(tok_num == 2) {
				strcpy(temp2, token);
			} else{
				strcpy(temp3, token);
			}
			token = strtok(NULL, delimit);
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
    input[1].loc = LOCCTR;

    int i=2;
    bool chk_sym;
    // until END
    do{
        chk_sym = true;
        if((int)strlen(input[i].symbol) != 0)
        {
            for(int j=0;j<sym_cnt;j++){
                if(strcmp(input[i].symbol, symtab[j].name) == 0){
                    chk_sym = false;
                    break;
                }
            }
            if(chk_sym == true){
                strcpy(symtab[sym_cnt].name, input[i].symbol);
                symtab[sym_cnt].value = LOCCTR;
                //printf("symtab[%d].name : %s   value : %d\n",sym_cnt, symtab[sym_cnt].name, symtab[sym_cnt].value);
                sym_cnt++;
            }else{
                //fputs("duplicate symbol", w_file);
                return EXIT_FAILURE;
            }
        }

        input[i].loc = LOCCTR;

        if(is_opcode(input[i].opcode) == true){
            if(format == 1){
                LOCCTR += 1;
            }else if(format==2){
                LOCCTR += 2;
            }else if(format==3){
                LOCCTR += 3;
            }else if(format==4){
                LOCCTR += 4;
            }
        }
        else if(strcmp(input[i].opcode, "WORD") == 0){
            LOCCTR += 3;
        }
        else if(strcmp(input[i].opcode, "RESW") == 0){
            LOCCTR += 3 * atoi(input[i].operand);
        }
        else if(strcmp(input[i].opcode, "RESB") == 0){
             LOCCTR += atoi(input[i].operand);
        }
        else if(strcmp(input[i].opcode, "BYTE") == 0){
            if(input[i].operand[0] == 'C'){
                LOCCTR += strlen(input[i].operand)-3;
            }else{
                LOCCTR+=1;
            }
        }
        else if(strcmp(input[i].opcode, "BASE") == 0){

        } 
        else{
            // fputs("invalid operation code", w_file);
            return EXIT_FAILURE;
        }
        i++;
    }while(strcmp(input[i].opcode, "END") != 0);

    if(fclose(r_file)){
        return EXIT_FAILURE;
        perror(r_path);
    }

    if(write_imediate() == false){
        printf("Fail!\n");
        return EXIT_FAILURE;
    }
    ////////////////////////////////////////
    //          Finish Pass1                
    ////////////////////////////////////////

    // printf("line count is %d\n",line_count);
    // printf("%s", input[line_count-1].opcode);

    /*
    char *w_path;
    w_path = argv[2];
    FILE *w_file = fopen(w_path, "w");
    if(!w_file)
    {
        perror(w_path);
        return EXIT_FAILURE;
    }
    if(fclose(w_file)){
        return EXIT_FAILURE;
        perror(w_path);
    }
    */


    ////////////////////////////////////////
    //          Start Pass2                
    ////////////////////////////////////////
    printf("Start Pass2\n");

    int SYMTAB_size;
    int line_cnt;

    FILE *fp = fopen("Intermediate","r");

    if(!fp){
        return EXIT_FAILURE;
    }

    fgets(line, 50, fp);
    token = strtok(line, delimit);
    SYMTAB_size = atoi(token);
    printf("SYM:%d\n",SYMTAB_size);
    SYMTAB st[SYMTAB_size];

    for(int j=0;j<SYMTAB_size;j++){
        fgets(line, 50, fp);
        token = strtok(line, delimit);
        strcpy(st[j].name, token);
        token = strtok(NULL, delimit);
        st[j].value = atoi(token);
    }

    fgets(line, 50, fp);
    token = strtok(line, delimit);
    line_cnt = atoi(token);

    printf("line_cnt : %d\n",line_cnt);
    

    fgets(line, 50, fp);
    token = strtok(line, delimit);
    token = strtok(NULL, delimit);

    if(strcmp(token,"START")==0){
    
    }else{
        return EXIT_FAILURE;
    }

    while(fgets(line, 50, fp)){
        tok_num = 0;
        token = strtok(line, delimit);

        int t_loc;
        char t_symbol[10];
        char t_opcode[10];
        char t_operand[10];

        while(token != NULL){
			tok_num++;
			if(tok_num == 1) {
				strcpy(temp1, token);
			} else if(tok_num == 2) {
				strcpy(temp2, token);
			} else if(tok_num == 3){
				strcpy(temp3, token);
			}else{
                strcpy(temp4, token);
            }
			token = strtok(NULL, delimit);
		}

        if(tok_num == 2){
            strcpy(t_opcode, temp1);
            t_loc = atoi(temp2);
            //printf("%s\t%d\n",t_opcode,t_loc);
        }else if(tok_num == 3){
            strcpy(t_opcode, temp1);
            strcpy(t_operand, temp2);
            t_loc = atoi(temp3);
            //printf("%s\t%s\t%d\n",t_opcode,t_operand,t_loc);

            if(strcmp(t_opcode,"END")==0){
                printf("The End!!!\n");
            }

        }else{
            strcpy(t_symbol, temp1);
            strcpy(t_opcode, temp2);
            strcpy(t_operand, temp3);
            t_loc = atoi(temp4);
            //printf("%s\t%s\t%s\t%d\n",t_symbol,t_opcode,t_operand,t_loc);
        }

        int opcode=0;
        int addr;

        if(is_opcode2(t_opcode, &opcode) == true){
            if(tok_num == 3 || tok_num == 4){
                // Exist operand
                for(int j=0;j<SYMTAB_size;j++){
                    if(strcmp(st[j].name, t_operand) == 0){
                        addr = st[j].value;
                        break;
                    }
                }
            }else{
                addr = 0;
            }
        }else if(strcmp(t_opcode,"BYTE")==0 || strcmp(t_opcode,"WORD")==0){

        }else if(strcmp(t_opcode,"BASE")==0){

        }


        // for simple, PC-relative
        opcode += 3;

        printf("16 : %X\n",opcode);



    }


//     if(fclose(fp)){
//         return EXIT_FAILURE;
//     }


}

bool is_opcode(char* str)
{
    bool result = false;
    if(str[0] == '+'){
        char temp[10];
        strcpy(temp, str+1);
        for(int i=0;i < op_cnt;i++){
            if(strcmp(optab[i].inst, temp) == 0){
                result = true;
                format=4;
                break;
            }
        }
    }else{
        for(int i=0;i<op_cnt;i++){
            if(strcmp(optab[i].inst, str) == 0){
                result = true;
                format = optab[i].format;
                break;
            }
        }
    }
    return result;
}
bool is_opcode2(char* str, int* opcode)
{
    bool result = false;
    if(str[0] == '+'){
        char temp[10];
        strcpy(temp, str+1);
        for(int i=0;i < op_cnt;i++){
            if(strcmp(optab[i].inst, temp) == 0){
                result = true;
                format=4;
                *opcode = optab[i].opcode;
                break;
            }
        }
    }else{
        for(int i=0;i<op_cnt;i++){
            if(strcmp(optab[i].inst, str) == 0){
                result = true;
                *opcode = optab[i].opcode;
                format = optab[i].format;
                break;
            }
        }
    }
    return result;
}
bool write_imediate()
{
    char result[100];
    char temp[10];
    FILE *fp = fopen("Intermediate","w");

    if(!fp)
        return false;

    sprintf(temp, "%d", sym_cnt);
    strcat(temp, "\n");
    fputs(temp, fp);

    for(int i=0;i<sym_cnt;i++){
        strcpy(result, symtab[i].name);
        strcat(result,"\t");
        sprintf(temp, "%d", symtab[i].value);
        strcat(result, temp);
        strcat(result,"\n");
        //symtab[sym_cnt]
        fputs(result,fp);
    }

    //char szHex[16];

    sprintf(temp, "%d", line_count);
    strcat(temp, "\n");
    fputs(temp, fp);


    for(int i=1;i<line_count;i++){
        strcpy(result,"");
        strcat(result, input[i].symbol);
        strcat(result,"\t");
        strcat(result, input[i].opcode);
        strcat(result,"\t");
        strcat(result, input[i].operand);
        strcat(result,"\t");
        sprintf(temp, "%d", input[i].loc);
        strcat(result,temp);
        strcat(result,"\n");
        fputs(result,fp);

        //printf("%s",result);

        //printf("%d\t%s\t%s\t%s \n",input[i].loc, input[i].symbol, input[i].opcode, input[i].operand);
        /* 10 -> 16 
        sprintf(szHex, "%X", input[i].loc);
        printf("%s\t%s\t%s\t%s \n",szHex, input[i].symbol, input[i].opcode, input[i].operand);
        */
    }

     if(fclose(fp))
        return false;

    return true;
}
