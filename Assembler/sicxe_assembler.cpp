#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 500
#define OP_SIZE 100

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
OPTAB* hash_optab[OP_SIZE];

const int op_cnt = (int)(sizeof(optab)/sizeof(optab[0]));

typedef struct{
    char name[10];
    int value;
}SYMTAB;

SYMTAB symtab[MAX_LINE_LENGTH];
SYMTAB* hash_symtab[MAX_LINE_LENGTH];
SYMTAB *st;
SYMTAB* hash_symtab_2[MAX_LINE_LENGTH];

int sym_cnt = 0;
int format;
unsigned int line_count = 1;

typedef struct{
    int loc;
    char symbol[10];
    char opcode[10];
    char operand[10];
    char* comment;
}INPUT;

INPUT input[MAX_LINE_LENGTH];

bool is_opcode(char *);
bool is_opcode2(char* , int*);
bool findSymbol(char*);
bool findSymbol2(char*, int*);
unsigned int getRegisterNum(char);
char* substring(char*, int, int);
char* toHex(int, int);
char *toHex_Original(int);


OPTAB *createOP(const char *inst, int opcode, int format)
{
    OPTAB *optab = (OPTAB *)malloc(sizeof(OPTAB));
    strcpy(optab->inst, inst);
    optab->opcode = opcode;
    optab->format = format;
    return optab;
}
SYMTAB *createSYM(const char *name, int value)
{
    SYMTAB *s = (SYMTAB *)malloc(sizeof(SYMTAB));
    strcpy(s->name, name);
    s->value = value;
    return s;
}

int hashcode(char *key)
{
    int hash = 0;

    for (int i = 0; i < strlen(key); i++)
    {
        hash += key[i];
    }
    return hash % OP_SIZE;
}

int hashcode_SYM(char *key)
{
    int hash = 0;

    for (int i = 0; i < strlen(key); i++)
    {
        hash += key[i];
    }
    return hash % MAX_LINE_LENGTH;
}

OPTAB *get(const char *inst)
{
    char key[20] = "";
    strcpy(key, inst);
    int idx = hashcode(key);
    do
    {
        OPTAB *o = hash_optab[idx];
        if (o == NULL)
            return NULL;

        if (strcmp(o->inst, inst) == 0)
            return o;

        idx++;
    } while (1);
}

SYMTAB *get_SYM(const char *name)
{
    char key[20] = "";
    strcpy(key, name);
    int idx = hashcode_SYM(key);
    do
    {
        SYMTAB *o = hash_symtab[idx];
        if (o == NULL)
            return NULL;

        if (strcmp(o->name, name) == 0)
            return o;
        idx++;
    } while (1);
}
SYMTAB *get_SYM_2(const char *name)
{
    char key[20] = "";
    strcpy(key, name);
    int idx = hashcode_SYM(key);
    do
    {
        SYMTAB *o = hash_symtab_2[idx];
        if (o == NULL)
            return NULL;

        if (strcmp(o->name, name) == 0)
            return o;
        idx++;
    } while (1);
}

void insert(OPTAB* op)
{
    char* key = op->inst;
    int idx = hashcode(key);
    do
    {
        if(hash_optab[idx] == NULL){
            hash_optab[idx] = op;
            break;
        }
        else if(hash_optab[idx]->opcode == op->opcode)
        {
            free(hash_optab[idx]);
            hash_optab[idx] = op;
        }
        idx++;
    } while (1);
}
bool insert_SYM(SYMTAB *sy)
{
    char *key = sy->name;
    int idx = hashcode_SYM(key);
    bool result;
    do
    {
        if (hash_symtab[idx] == NULL)
        {
            hash_symtab[idx] = sy;
            result = true;
            break;
        }
        else if (strcmp(hash_symtab[idx]->name, sy->name) == 0)
        {
            result = false;
            break;
        }
        idx++;
    } while (1);
    return result;
}
bool insert_SYM_2(SYMTAB *sy)
{
    char *key = sy->name;
    int idx = hashcode_SYM(key);
    bool result;
    do
    {
        if (hash_symtab_2[idx] == NULL)
        {
            hash_symtab_2[idx] = sy;
            result = true;
            break;
        }
        else if (strcmp(hash_symtab_2[idx]->name, sy->name) == 0)
        {
            result = false;
            break;
        }
        idx++;
    } while (1);
    return result;
}

void MakeOptable()
{
    insert(createOP("ADD", 0x18, 3));
    insert(createOP("ADDF", 0x58, 3));
    insert(createOP("ADDR", 0x90, 2));
    insert(createOP("AND", 0x40, 3));
    insert(createOP("CLEAR", 0xB4, 2));
    insert(createOP("COMP",0x28,3));
    insert(createOP("COMPF",0x88,3));
    insert(createOP("COMPR",0xA0,2));
    insert(createOP("DIV",0x24,3));
    insert(createOP("DIVF",0x64,3));
    insert(createOP("DIVR",0x9C,2));
    insert(createOP("FIX",0xC4,1));
    insert(createOP("FLOAT",0xC0,1));
    insert(createOP("HIO",0xF4,1));
    insert(createOP("J",0x3C,3));
    insert(createOP("JEQ",0x30,3));
    insert(createOP("JGT",0x34,3));
    insert(createOP("JLT",0x38,3));
    insert(createOP("JSUB",0x48,3));
    insert(createOP("LDA",0x00,3));
    insert(createOP("LDB",0x68,3));
    insert(createOP("LDCH",0x50,3));
    insert(createOP("LDF",0x70,3));
    insert(createOP("LDL",0x08,3));
    insert(createOP("LDS",0x6C,3));
    insert(createOP("LDT",0x74,3));
    insert(createOP("LDX",0x04,3));
    insert(createOP("LPS",0xD0,3));
    insert(createOP("MUL",0x20,3));
    insert(createOP("MULF",0x60,3));
    insert(createOP("MULR",0x98,2));
    insert(createOP("NORM",0xC8,1));
    insert(createOP("OR",0x44,3));
    insert(createOP("RD",0xD8,3));
    insert(createOP("RMO",0xAC,2));
    insert(createOP("RSUB",0x4C,3));
    insert(createOP("SHIFTL",0xA4,2));
    insert(createOP("SHIFTR",0xA8,2));
    insert(createOP("SIO",0xF0,1));
    insert(createOP("SSK",0xEC,3));
    insert(createOP("STA",0x0C,3));
    insert(createOP("STB",0x78,3));
    insert(createOP("STCH",0x54,3));
    insert(createOP("STF",0x80,3));
    insert(createOP("STI",0xD4,3));
    insert(createOP("STL",0x14,3));
    insert(createOP("STS",0x7C,3));
    insert(createOP("STSW",0xE8,3));
    insert(createOP("STT",0x84,3));
    insert(createOP("STX",0x10,3));
    insert(createOP("SUB",0x1C,3));
    insert(createOP("SUBF",0x5C,3));
    insert(createOP("SUBR",0x94,2));
    insert(createOP("SVC",0xB0,2));
    insert(createOP("TD",0xE0,3));
    insert(createOP("TIO",0xF8,1));
    insert(createOP("TIX",0x2C,3));
    insert(createOP("TIXR",0xB8,2));
    insert(createOP("WD",0xDC,3));
}

bool _UseHash = false;

int main(int argc, char** argv)
{
    char *r_path;
    char *w_path;
    char *isHash;
    char line[100];
    
    clock_t t_start = clock();

    if(argc < 1)
        return EXIT_FAILURE;

    r_path = argv[1];
    w_path = argv[2];
    isHash = argv[3];

    /* Open file */
    FILE *r_file = fopen(r_path, "r");
    // Write Intermediate File
    FILE *wfp = fopen("Intermediate_file", "w");
    // Assembly listing file
    FILE *fp_Assembly = fopen("Assembly_listing_file" ,"w");
    // Check Hash
    if(isHash != NULL && strcmp(isHash, "hash") == 0)
    {
        _UseHash = true;
        MakeOptable();
    }

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
    while (fgets(line, 50, r_file))
    {
        if(line[0] == '.')
        {
            input[line_count].comment = (char *)malloc((int)strlen(line) + 1);
            memset(input[line_count].comment, 0, (int)strlen(line) + 1);
            strcpy(input[line_count].comment, line);
            line_count++;
            continue;
        }
        else if (strstr(line, ".") != NULL)
        {
            char *temp = strchr(line, '.');
            int dot_idx = (int)(temp - &line[0]);
            char *comment = substring(line, dot_idx, (int)strlen(line) - 2);
            input[line_count].comment = (char *)malloc((int)strlen(comment) + 1);
            memset(input[line_count].comment, 0, (int)strlen(comment) + 1);
            strcpy(input[line_count].comment, comment);

            char *temp_line = substring(line, 0, dot_idx - 1);
            strcpy(line, temp_line);
        }
        tok_num = 0;
        token = strtok(line, delimit);

        if(token == NULL)
        {
            strcpy(input[line_count].opcode, " ");
            line_count++;
            continue;
        }

        while (token != NULL)
        {
            tok_num++;
            if (tok_num == 1)
                strcpy(temp1, token);
            else if (tok_num == 2)
                strcpy(temp2, token);
            else
                strcpy(temp3, token);

            token = strtok(NULL, delimit);
        }
        if (tok_num == 1)
            strcpy(input[line_count].opcode, temp1);
        else if (tok_num == 2)
        {
            strcpy(input[line_count].opcode, temp1);
            strcpy(input[line_count].operand, temp2);
        }
        else
        {
            strcpy(input[line_count].symbol, temp1);
            strcpy(input[line_count].opcode, temp2);
            strcpy(input[line_count].operand, temp3);
        }
        line_count++;
    }

    int LOCCTR;
    int i = 1;
    bool hasStart = false;

    for (i = 1; i < line_count; i++)
    {
        if (strcmp(input[i].opcode, "START") == 0)
        {
            hasStart = true;
            break;
        }
    }

    i = 1;

    if(hasStart)
    {
        while (i < line_count)
        {
            if (strcmp(input[i].opcode, "START") == 0)
            {
                LOCCTR = strtol(input[i].operand, NULL, 16);
                hasStart = true;
                input[i++].loc = LOCCTR;
                break;
            }
            i++;
        }
    }
    else
        LOCCTR = 0;

    do
    {
        if(input[i].comment != NULL && (int)strlen(input[i].opcode) == 0)
        {
            i++;
            continue;
        }
        if(strcmp(input[i].opcode, " ") == 0)
        {
            i++;
            continue;
        }

        bool chk_sym = true;
    
        if ((int)strlen(input[i].symbol) != 0)
        {
            chk_sym = findSymbol(input[i].symbol);
            if (chk_sym == true)
            {
                if (_UseHash == true)
                {
                    bool rightSym = insert_SYM(createSYM(input[i].symbol, LOCCTR));
                    if (!rightSym)
                    {
                        fputs("duplicate symbol", fp_Assembly);
                        return EXIT_FAILURE;
                    }
                    else
                        sym_cnt++;
                }
                else
                {
                    strcpy(symtab[sym_cnt].name, input[i].symbol);
                    symtab[sym_cnt].value = LOCCTR;
                    sym_cnt++;
                }
            }
            else
            {
                fputs("duplicate symbol", fp_Assembly);
                return EXIT_FAILURE;
            }
        }
        input[i].loc = LOCCTR;
        
        if (is_opcode(input[i].opcode) == true)
        {
            if (format == 1)
                LOCCTR += 1;
            else if (format == 2)
                LOCCTR += 2;
            else if (format == 3)
                LOCCTR += 3;
            else if (format == 4)
                LOCCTR += 4;
        }
        else if (strcmp(input[i].opcode, "WORD") == 0)
            LOCCTR += 3;
        else if (strcmp(input[i].opcode, "RESW") == 0)
            LOCCTR += 3 * atoi(input[i].operand);
        else if (strcmp(input[i].opcode, "RESB") == 0)
            LOCCTR += atoi(input[i].operand);
        else if (strcmp(input[i].opcode, "BYTE") == 0)
        {
            if (input[i].operand[0] == 'C')
                LOCCTR += strlen(input[i].operand) - 3;
            else
                LOCCTR += 1;
            
        }
        else if (strcmp(input[i].opcode, "BASE") == 0){}
        else
        {
            fputs("invalid operation code", fp_Assembly);
            return EXIT_FAILURE;
        }
        i++;
        if(i == line_count)
            break;
    } while (strcmp(input[i].opcode, "END") != 0);

    // write symbol table in Intermediate file
    if (_UseHash == true)
    {
        char c_sym_cnt[10];
        sprintf(c_sym_cnt, "%d", sym_cnt);
        strcat(c_sym_cnt, "\n");
        fputs(c_sym_cnt, wfp);
        int cnt = 0;
        for (int i = 0; i < MAX_LINE_LENGTH; i++)
        {
            SYMTAB *s = hash_symtab[i];
            if (s == NULL)
                continue;
            cnt++;
            fprintf(wfp, "%s\t%d\n", s->name, s->value);
            if (cnt == sym_cnt + 1)
                break;
        }
        for (int j = 1; j < line_count - 1; j++)
        {
            if (input[j].comment == NULL)
            {
                if (strcmp(input[j].opcode, " ") == 0)
                {
                    fprintf(wfp, "\n");
                    continue;
                }
                if (strcmp(input[j].opcode, "BASE") == 0)
                    fprintf(wfp, "\t\t%s\t%s\n", input[j].opcode, input[j].operand);
                else
                    fprintf(wfp, "%d\t%s\t%s\t%s\n", input[j].loc, input[j].symbol, input[j].opcode, input[j].operand);
            }
            else if(strlen(input[j].opcode) == 0)
                fprintf(wfp, "%s", input[j].comment);
            else
            {
                if (strcmp(input[j].opcode, "BASE") == 0)
                    fprintf(wfp, "\t\t%s\t%s\t%s\n", input[j].opcode, input[j].operand, input[j].comment);
                else
                    fprintf(wfp, "%d\t%s\t%s\t%s\t%s\n", input[j].loc, input[j].symbol, input[j].opcode, input[j].operand, input[j].comment);
            }
        }
        fprintf(wfp, "\t%s\t%s\t%s\n", input[line_count - 1].symbol, input[line_count - 1].opcode, input[line_count - 1].operand);

        clock_t t_end = clock();

        fputs("\n========= optable ============\n", wfp);
        fputs("index \tkey \topcode \tformat\n", wfp);
        for (int i = 0; i < OP_SIZE; i++)
        {
            OPTAB *o = hash_optab[i];
            if (o == NULL)
                continue;
            fprintf(wfp, "%d \t%s \t%.2X \t%d\n", i, o->inst, o->opcode, o->format);
        }
        fprintf(wfp, "\nEstimated Time(Using Hash Table) : %lf\n", (double)(t_end - t_start)/CLOCKS_PER_SEC);
    }
    else
    {
        char c_sym_cnt[10];

        sprintf(c_sym_cnt, "%d", sym_cnt);
        strcat(c_sym_cnt, "\n");
        fputs(c_sym_cnt, wfp);
        for (int j = 0; j < sym_cnt; j++)
        {
            fprintf(wfp, "%s\t%d\n", symtab[j].name, symtab[j].value);
        }

        for (int j = 1; j < line_count - 1; j++)
        {
            if (input[j].comment == NULL)
            {
                if (strcmp(input[j].opcode, " ") == 0)
                {
                    fprintf(wfp, "\n");
                    continue;
                }
                if (strcmp(input[j].opcode, "BASE") == 0)
                    fprintf(wfp, "\t\t%s\t%s\n", input[j].opcode, input[j].operand);
                else
                    fprintf(wfp, "%d\t%s\t%s\t%s\n", input[j].loc, input[j].symbol, input[j].opcode, input[j].operand);
            }
            else if(strlen(input[j].opcode) == 0)
                fprintf(wfp, "%s", input[j].comment);
            else
            {
                if (strcmp(input[j].opcode, "BASE") == 0)
                    fprintf(wfp, "\t\t%s\t%s\t%s\n", input[j].opcode, input[j].operand, input[j].comment);
                else
                    fprintf(wfp, "%d\t%s\t%s\t%s\t%s\n", input[j].loc, input[j].symbol, input[j].opcode, input[j].operand, input[j].comment);
            }
        }
        fprintf(wfp, "\t%s\t%s\t%s\n", input[line_count - 1].symbol, input[line_count - 1].opcode, input[line_count - 1].operand);

        clock_t t_end = clock();
        fprintf(wfp, "\nEstimated Time : %lf\n", (double)(t_end - t_start)/CLOCKS_PER_SEC);
    }

    fclose(r_file);
    fclose(wfp);
    ////////////////////////////////////////
    //          Finish Pass1                
    ////////////////////////////////////////

    ////////////////////////////////////////
    //          Start Pass2                
    ////////////////////////////////////////

    int SYMTAB_size;
    int line_cnt;
    int base;
    int program_length;
    char object_code[MAX_LINE_LENGTH][10];
    char program_name[10];
    int arr_loc[MAX_LINE_LENGTH];
    int object_cnt = 0;
    int Modification_arr[MAX_LINE_LENGTH];
    int modifi_cnt=0;

    FILE *fp = fopen("Intermediate_file","r");
    FILE *F_object = fopen(w_path, "w");

    if(!F_object)
        return EXIT_FAILURE;
    if(!fp)
        return EXIT_FAILURE;

    fgets(line, 50, fp);
    token = strtok(line, delimit);
    SYMTAB_size = atoi(token);
    st = (SYMTAB *)malloc(sizeof(SYMTAB) * SYMTAB_size);

    if (_UseHash == true)
    {
        for (int i = 0; i < SYMTAB_size; i++)
        {
            fgets(line, 50, fp);
            char temp_name[10];
            int temp_value;
            token = strtok(line, delimit);
            strcpy(temp_name, token);
            token = strtok(NULL, delimit);
            temp_value = atoi(token);
            insert_SYM_2(createSYM(temp_name, temp_value));
        }
    }
    else
    {
        for (int j = 0; j < SYMTAB_size; j++)
        {
            fgets(line, 50, fp);
            token = strtok(line, delimit);
            strcpy(st[j].name, token);
            token = strtok(NULL, delimit);
            st[j].value = atoi(token);
        }
    }

    while(fgets(line, 50, fp))
    {
        char t1[10];
        char t2[10];
        char t3[10];
        char t4[10];
        
        char *comment;
        bool hasComment = false;

        if(strlen(line) == 1)
        {
            fprintf(fp_Assembly, "\n");
            continue;
        }
        
        if(line[0] == '.'){
            fputs(line, fp_Assembly);
            continue;
        }
        else if (strstr(line, ".") != NULL)
        {
            hasComment = true;
            char *temp = strchr(line, '.');
            int dot_idx = (int)(temp - &line[0]);
            comment = substring(line, dot_idx, (int)strlen(line) - 2);

            char *temp_line = substring(line, 0, dot_idx - 1);
            strcpy(line, temp_line);
        }

        tok_num = 0;
        token = strtok(line, delimit);

        if(token == NULL)
            break;

        int t_loc;
        char t_symbol[10];
        char t_opcode[10];
        char t_operand[10];
        int Base_addr;

        while (token != NULL)
        {
            tok_num++;
            if (tok_num == 1)
                strcpy(t1, token);
            else if (tok_num == 2)
                strcpy(t2, token);
            else if (tok_num == 3)
                strcpy(t3, token);
            else
                strcpy(t4, token);
            
            token = strtok(NULL, delimit);
        }

        if(tok_num == 2)
        {
            // END
            if(strcmp(t1, "END") == 0)
            {
                fprintf(fp_Assembly, "\t\t%s\t%s\n", t1, t2);
                break;
            }
            if(strcmp(t1, "BASE") == 0)
            {
                fprintf(fp_Assembly, "\t\t%s\t%s\n", t1, t2);
                findSymbol2(t2, &Base_addr);
                continue;
            }

            // RSUB
            t_loc = atoi(t1);
            strcpy(t_opcode, t2);
            strcpy(t_operand,"");
            strcpy(t_symbol,"");
        }
        else if(tok_num == 3)
        {
            t_loc = atoi(t1);
            strcpy(t_opcode, t2);
            strcpy(t_operand, t3);
            strcpy(t_symbol,"");
        }
        else if(tok_num == 4)
        {
            t_loc = atoi(t1);
            strcpy(t_symbol, t2);
            strcpy(t_opcode, t3);
            strcpy(t_operand, t4);
        }

        int opcode = 0;
        int ni;
        int xbpe = 2;
        int addr;
        bool print = false;

        if(strcmp(t_opcode, "START") == 0)
        {
            if(hasComment)
                fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%s\n", t_loc, t_symbol, t_opcode, t_operand, comment);
            else
                fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\n", t_loc, t_symbol, t_opcode, t_operand);
            
            // Header
            fprintf(F_object, "H%s", t_symbol);
            strcpy(program_name, t_symbol);

            int symbol_length = strlen(t_symbol);
            int temp = 6 - symbol_length;
            for (int i = 0; i < temp; i++)
            {
                fprintf(F_object, " ");
            }

            fprintf(F_object, "%06d", atoi(t_operand));
            continue;
        }

        if(is_opcode2(t_opcode, &opcode) == true){
            if(format == 1)
            {
                if(!hasComment)
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t\t\t%.2X\n", t_loc, t_symbol, t_opcode, opcode);
                else
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t\t\t%.2X\t%s\n", t_loc, t_symbol, t_opcode, opcode, comment);
                program_length = t_loc + 1;
                print = true;
            }
            else if (format == 2)
            {
                int pos = 0;
                char *test = toHex(4214,6);
                if (strlen(t_operand) == 1)
                {
                    if(!hasComment)
                        fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X0\n", t_loc, t_symbol, t_opcode, t_operand, opcode, getRegisterNum(t_operand[0]));
                    else
                        fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X0\t%s\n", t_loc, t_symbol, t_opcode, t_operand, opcode, getRegisterNum(t_operand[0]), comment);
                    char* op = toHex(opcode, 2);
                    char* disp = toHex(getRegisterNum(t_operand[0]),1);
                    
                    char temp[4] = "";
                    strcpy(temp, op);
                    strcat(temp, disp);
                    strcat(temp,"0");
                    strcpy(object_code[object_cnt], temp);
                    arr_loc[object_cnt] = t_loc;
                    object_cnt++;
                }
                else
                {
                    if(!hasComment)
                        fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X%X\n", t_loc, t_symbol, t_opcode, t_operand, opcode, getRegisterNum(t_operand[0]), getRegisterNum(t_operand[2]));
                    else
                        fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X%X\t%s\n", t_loc, t_symbol, t_opcode, t_operand, opcode, getRegisterNum(t_operand[0]), getRegisterNum(t_operand[2]), comment);
                    char* op = toHex(opcode, 2);
                    char* disp1 = toHex(getRegisterNum(t_operand[0]), 1);
                    char* disp2 = toHex(getRegisterNum(t_operand[2]), 1);
                    char temp[4] = "";
                    strcpy(temp, op);
                    strcat(temp, disp1);
                    strcat(temp, disp2);
                    strcpy(object_code[object_cnt], temp);
                    arr_loc[object_cnt] = t_loc;
                    object_cnt++;
                }
               
                program_length = t_loc + 2;
                print = true;
            }
            else{
                if(tok_num == 3 || tok_num == 4){
                    // Exist operand
                    if(t_operand[0] == '#' || t_operand[0] == '@')
                    {
                        char temp[10];
                        bool isSymbol = false;
                        strcpy(temp, t_operand + 1);
                        isSymbol = findSymbol2(temp, &addr);
                        if (isSymbol)
                            addr = addr - t_loc - 3;
                       
                        if(!isSymbol && t_operand[0] == '#')
                        {
                            addr = atoi(temp);
                            xbpe = 0;
                        }
                        if(t_operand[0] == '#')
                            ni = 1;
                        else
                            ni = 2;
                        
                        program_length = t_loc + 3;
                    }
                    else if (strstr(t_operand, ",") != NULL)
                    {
                        int comma_position;
                        for(int k=0;k<strlen(t_operand);k++)
                        {
                            if(t_operand[k] == ',')
                            {
                                comma_position = k;
                                break;
                            }
                        }
                        char *sym = substring(t_operand, 0, comma_position - 1);

                        bool isSymbol = findSymbol2(sym, &addr);
                        int ori_addr = addr;
                        if (isSymbol)
                            addr = addr - t_loc - 3;

                        if (addr > 2047 || addr < -2048)
                        {
                            addr = ori_addr - Base_addr;
                            xbpe = 12;
                        }
                        else if (addr < 0)
                        {
                            opcode += 3;
                            xbpe += 8;
                            fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X", t_loc, t_symbol, t_opcode, t_operand, opcode, xbpe);

                            char s[10];
                            sprintf(s, "%X", addr);

                            int len = strlen(s) - 3;

                            char *op = toHex(opcode, 2);
                            char *xb = toHex(xbpe, 1);
                            char *temp_addr = substring(s, 5, strlen(s) - 1);
                    
                            char temp[6] = "";
                            strcat(temp, op);
                            strcat(temp, xb);
                            strcat(temp, temp_addr);
                            strcpy(object_code[object_cnt], temp);
                            arr_loc[object_cnt] = t_loc;
                            object_cnt++;
                            if(!hasComment)
                                fprintf(fp_Assembly, "%s\n", temp_addr);
                            else
                                fprintf(fp_Assembly, "%s\t%s\n", temp_addr, comment);
                            print = true;
                        }
                    }
                    else
                    {
                        bool findSym = findSymbol2(t_operand, &addr);
                        int ori_addr = addr;
               
                        if (format == 4){}
                        else
                        {
                            addr = addr - t_loc - 3;

                            if (addr > 2047 || addr < -2048)
                            {
                                addr = ori_addr - Base_addr;
                                xbpe = 4;
                            }
                            else if (addr < 0)
                            {
                                opcode += 3;
                                fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X", t_loc, t_symbol, t_opcode, t_operand, opcode, xbpe);

                                char s[10];
                                sprintf(s, "%X", addr);
                                int len = strlen(s) - 3;

                                char *op = toHex(opcode, 2);
                                char *xb = toHex(xbpe, 1);
                                char disp[3];

                                char *temp_addr = substring(s, 5, strlen(s) - 1);
                                if(!hasComment)
                                    fprintf(fp_Assembly, "%s\n", temp_addr);
                                else
                                    fprintf(fp_Assembly, "%s\t%s\n", temp_addr, comment);
                                print = true;
                                char temp[6];
                                strcpy(temp, "");
                                strcat(temp, op);
                                strcat(temp, xb);
                                strcat(temp, temp_addr);
                                strcpy(object_code[object_cnt], temp);
                                arr_loc[object_cnt] = t_loc;
                                object_cnt++;
                            }
                        }
                        ni = 3;

                        if (!findSym)
                        {
                            fputs("undefined symbol\n", fp_Assembly);
                            fputs(t_operand, fp_Assembly);
                            addr = 0;
                            return EXIT_FAILURE;
                        }
                    }
                }
                else
                {
                    xbpe = 0;
                    addr = 0;
                }
            }
        }
        else if(strcmp(t_opcode,"BYTE") == 0)
        {
            fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t",t_loc, t_symbol, t_opcode, t_operand);
            if (t_operand[0] == 'C')
            {
                char ob[10];
                char tmp[10];
                strcpy(object_code[object_cnt],"");
                for (int k = 2; k < strlen(t_operand) - 1; k++)
                {
                    fprintf(fp_Assembly, "%X", t_operand[k]);
                    char *temp = toHex_Original(t_operand[k]);
                    strcpy(tmp, temp);
                    strcat(object_code[object_cnt], tmp);
                }
                arr_loc[object_cnt] = t_loc;
                program_length = t_loc + strlen(t_operand) - 3;
            }
            else if (t_operand[0] == 'X')
            {
                strcpy(object_code[object_cnt], "");
                char temp[10]="";
                int a = 0;
                for (int k = 2; k < strlen(t_operand) - 1; k++)
                {
                    fprintf(fp_Assembly, "%c", t_operand[k]);
                    temp[a] = t_operand[k];
                    a++;
                }

                strcpy(object_code[object_cnt], temp);
                arr_loc[object_cnt] = t_loc;
                if (((int)strlen(t_operand) - 3) % 2 == 0)
                    program_length = t_loc + (strlen(t_operand)-3) / 2;
                else
                    program_length = t_loc + (strlen(t_operand) / 2-3) + 1;
            }
            if(!hasComment)
                fprintf(fp_Assembly, "\n");
            else
                fprintf(fp_Assembly, "\t%s\n",comment);
            object_cnt++;
            print = true;
        }
        else if(strcmp(t_opcode,"RESW") == 0 || strcmp(t_opcode,"RESB")==0)
        {
            fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\n", t_loc, t_symbol, t_opcode, t_operand);
            continue;
        }
        else if(strcmp(t_opcode, "WORD") == 0)
        {
            char *tmp = toHex(atoi(t_operand), 6);

            fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%s\n", t_loc, t_symbol, t_opcode, t_operand, tmp);
            strcpy(object_code[object_cnt], tmp);
            arr_loc[object_cnt] = t_loc;
            object_cnt++;
            continue;
        }
        
        if(!print)
        {
            opcode += ni;
            if(format == 4)
            {
                xbpe = 1;
                if(!hasComment)
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X%.5X\n", t_loc, t_symbol, t_opcode, t_operand, opcode, xbpe, addr);
                else
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X%.5X\t%s\n", t_loc, t_symbol, t_opcode, t_operand, opcode, xbpe, addr, comment);
                //opcode, xbpe, addr
                char *op = toHex(opcode, 2);
                char *xb = toHex(xbpe, 1);
                char *disp = toHex(addr, 5);
                char temp[8];
                strcpy(temp, "");
                strcat(temp, op);
                strcat(temp, xb);
                strcat(temp, disp);
                strcpy(object_code[object_cnt], temp);
                arr_loc[object_cnt++] = t_loc;

                if (!findSymbol(t_operand) == true)
                {
                    Modification_arr[modifi_cnt] = t_loc;
                    modifi_cnt++;
                }
            }
            else
            {
                if(strlen(t_operand) == 8)
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t%.2X%X%.3X", t_loc, t_symbol,t_opcode,t_operand, opcode, xbpe, addr);
                else
                    fprintf(fp_Assembly, "%.4X\t%s\t%s\t%s\t\t%.2X%X%.3X", t_loc, t_symbol,t_opcode,t_operand, opcode, xbpe, addr);

                if(!hasComment)
                    fprintf(fp_Assembly, "\n");
                else
                    fprintf(fp_Assembly, "\t%s\n", comment);
                char* op = toHex(opcode, 2);
                char* xb = toHex(xbpe, 1);
                char* disp = toHex(addr, 3);
                char temp[8];
                strcpy(temp, "");
                strcpy(temp, op);
                strcat(temp, xb);
                strcat(temp, disp);
                strcpy(object_code[object_cnt], temp);
                arr_loc[object_cnt] = t_loc;
                object_cnt++;
            }
        }
    }

    fclose(fp_Assembly);
    
    fprintf(F_object, "%06X\n", program_length);
    
    int cnt=0;
    int First_addr = arr_loc[0];
    while(cnt < object_cnt)
    {
        char temp[70] = "";
        strcpy(temp,"");
        int len = 0;
        int start_addr = arr_loc[cnt];
    
        while(1)
        {
            strcat(temp, object_code[cnt]);
            len += strlen(object_code[cnt]);

            if(cnt + 1 < object_cnt)
            {
                if(len + strlen(object_code[cnt+1]) > 60)
                {
                    break;
                }
                int t;
                if(strlen(object_code[cnt]) %2 == 0)
                    t = strlen(object_code[cnt])/2;
                else
                    t = strlen(object_code[cnt])/2 + 1;

                if (arr_loc[cnt + 1] - arr_loc[cnt] > t)
                    break;
            }
            else
                break;

            cnt++;
        }
        cnt++;
        int stringlen;
        if (strlen(temp) % 2 == 0)
            stringlen = strlen(temp) / 2;
        else
            stringlen = strlen(temp) / 2 + 1;

        fprintf(F_object, "T%06X%02X%s\n", start_addr, stringlen, temp);   
    }

    for(int k=0;k<modifi_cnt;k++)
    {
        fprintf(F_object, "M%06X05+%s\n", Modification_arr[k]+1, program_name);
    }

    fprintf(F_object, "E%06X\n", First_addr);

    fclose(F_object);
}

bool is_opcode(char *str)
{
    bool result = false;

    if (_UseHash == true)
    {
        if (str[0] == '+')
        {
            char temp[10];
            strcpy(temp, str + 1);

            OPTAB *opt = get(temp);
            if (opt == NULL)
                return false;
            else
            {
                result = true;
                format = 4;
            }
        }
        else
        {
            OPTAB *opt = get(str);
            if (opt == NULL)
                return false;
            else
            {
                result = true;
                format = opt->format;
            }
        }
    }
    else
    {
        if (str[0] == '+')
        {
            char temp[10];
            strcpy(temp, str + 1);
            for (int i = 0; i < op_cnt; i++)
            {
                if (strcmp(optab[i].inst, temp) == 0)
                {
                    result = true;
                    format = 4;
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < op_cnt; i++)
            {
                if (strcmp(optab[i].inst, str) == 0)
                {
                    result = true;
                    format = optab[i].format;
                    break;
                }
            }
        }
    }
    return result;
}

bool is_opcode2(char* str, int* opcode)
{
    bool result = false;

    if (_UseHash == true)
    {
        if (str[0] == '+')
        {
            char temp[10];
            strcpy(temp, str + 1);


            OPTAB *opt = get(temp);
            if (opt == NULL)
                return false;
            else
            {
                result = true;
                format = 4;
                *opcode = opt->opcode;
            }
        }
        else
        {
            OPTAB *opt = get(str);
            if (opt == NULL)
                return false;
            else
            {
                result = true;
                format = opt->format;
                *opcode = opt->opcode;
            }
        }
    }
    else
    {
        if (str[0] == '+')
        {
            char temp[10];
            strcpy(temp, str + 1);
            for (int i = 0; i < op_cnt; i++)
            {
                if (strcmp(optab[i].inst, temp) == 0)
                {
                    result = true;
                    format = 4;
                    *opcode = optab[i].opcode;
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < op_cnt; i++)
            {
                if (strcmp(optab[i].inst, str) == 0)
                {
                    result = true;
                    *opcode = optab[i].opcode;
                    format = optab[i].format;
                    break;
                }
            }
        }
    }

    return result;
}

bool findSymbol(char* input)
{
    bool result = true;
    if(_UseHash == true)
    {
        SYMTAB *sym = get_SYM(input);
        if(sym == NULL)
            result = true;
        else
            result = false;
    }
    else
    {
        for(int i=0;i<sym_cnt;i++)
        {
            if(strcmp(input, symtab[i].name) == 0)
            {
                result = false;
                break;
            }
        }
    }
    return result;
}
bool findSymbol2(char* input, int *addr)
{
    bool result = false;
    if(_UseHash == true)
    {
        SYMTAB *sym = get_SYM_2(input);
        if(sym == NULL)
            return false;
        else
        {
            *addr = sym->value;
            return true;
        }
    }
    else
    {
        for(int i=0;i<sym_cnt;i++)
        {
            if(strcmp(input, symtab[i].name) == 0)
            {
                *addr = symtab[i].value;
                return true;
            }
        }
    }
    return false;
}

unsigned int getRegisterNum(char regi)
{
    unsigned int result;
    if (regi == 'X')
        result = 1;
    else if (regi == 'A')
        result = 0;
    else if (regi == 'L')
        result = 2;
    else if (regi == 'B')
        result = 3;
    else if (regi == 'S')
        result = 4;
    else if (regi == 'T')
        result = 5;
    return result;
}

char* substring(char *input, int begin, int end){
    int cnt = 0;
    int size = (end - begin) + 2;
    char *str = (char *)malloc(size);

    memset(str, 0, size);

    for (int i = begin; i <= end; i++)
    {
        str[cnt] = input[i];
        cnt++;
    }
    return str;
}

char *toHex(int input, int length)
{
    int pos = 0;
    char *result = (char *)malloc(length);
    char *result2 = (char*)malloc(length);
    memset(result, 0 ,length);
    memset(result2, 0 ,length);

    while (1)
    {
        int mod = input % 16;
        if (mod < 10)
            result[pos] = mod + 48;
        else
            result[pos] = 65 + (mod - 10);

        input /= 16;
        pos++;
        if (input == 0)
            break;
    }
    while(strlen(result) != length)
    {
        strcat(result, "0");
    }
    int j = length-1;
    for(int i=0;i<length;i++)
    {
        result2[i] = result[j];
        j--;
    }
    return result2;
};

char *toHex_Original(int input)
{
    int pos = 0;
    char *result = (char *)malloc(2);
    char *result2 = (char*)malloc(2);
    memset(result, 0 ,2);
    memset(result2, 0 ,2);

    while (1)
    {
        int mod = input % 16;
        if (mod < 10)
            result[pos] = mod + 48;
        else
            result[pos] = 65 + (mod - 10);

        input /= 16;
        pos++;
        if (input == 0)
            break;
    }

    int k = (int)strlen(result) - 1;
    for (int i = 0; i < strlen(result); i++)
    {
        result2[i] = result[k];
        k--;
    }
    return result2;
};
