#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Page structure*/
typedef struct{
    int pageNumber;
    char contents[64];
    int timer;
    int secondChance;
} page;

int numFromString(char*);
void memInit(page**, int);
void readFromPage();
void writeToPage();
void printMem();

int main(int argc, char* argv[])
{
    char inFileName[64], outFileName[64], c;
    int isLRU = 0, i, j;
    int mainMemSize, secMemSize;
    FILE *inFile, *outFile;
    page *mainMemory, *secMemory;
    size_t len = 0;
    ssize_t read;
    char *line = NULL, param[32];
    int pageNum;

    /*Checking the passed parameters*/
    if(argc < 6)
    {
        printf("Not enough parameters passed.\n");
        return 1;
    }
    else if(argc > 6)
    {
        printf("Too many parameters passed.\n");
        return 1;
    }

    /*Getting user data from program arguments*/
    if(!strcmp(argv[1], "useLru") || !strcmp(argv[1], "useLRU") || !strcmp(argv[1], "1"))
        isLRU = 1;
    else
        isLRU = 0;
    strcpy(inFileName, argv[2]);
    strcpy(outFileName, argv[3]);
    secMemSize = numFromString(argv[4]);
    mainMemSize = numFromString(argv[5]);

    printf("Program settings:\n");
    printf("Input File: %s\n", inFileName);
    printf("Output File: %s\n", outFileName);
    if(isLRU) printf("Paging with LRU\n");
    else printf("Paging with Second Chance FIFO\n");
    printf("Main memory size (RAM frames): %d\n", mainMemSize);
    printf("Secondary memory size (virtual memory pages): %d\n", secMemSize);

    /*initialization of main and seondary memory*/
    memInit(&mainMemory, mainMemSize);
    memInit(&secMemory, secMemSize);

    /*Get all data into secondary memory array*/
    inFile = fopen(inFileName, "r");
    if(inFile == NULL)
    {
        printf("Error reading file. Try again.\n");
        return 1;
    }
    while((read = getline(&line, &len, inFile)) != -1)
    {
        switch(line[0])
        {
            case 'r':
                for(i=0; line[i]!=' '; i++); //Skip all instruction letters
                i++;
                j=0;
                while (line[i]>='0' && line[i]<='9') param[j++] = line[i++];
                param[j] = '\0';
                pageNum = numFromString(param);

                if(isLRU)
                {
                    printf("Paging with LRU: ");
                }
                else
                {
                    printf("Paging with SC-FIFO: ");
                }

                printf("READ from page '%d'\n", pageNum);
                break;
            case 'w':
                for(i=0; line[i]!=' '; i++); //Skip all instruction letters
                i++;
                j=0;
                while (line[i]!=' ') param[j++] = line[i++];
                param[j] = '\0';
                pageNum = numFromString(param);
                c = line[i+1];

                if(isLRU)
                {
                    printf("Paging with LRU: ");
                }
                else
                {
                    printf("Paging with SC-FIFO: ");
                }

                printf("WRITE char '%c' to page '%d'\n", c, pageNum);
                break;
            case 'p':
                printMem();
                break;
            default:
                printf("Unknown instruction\n");
        }
    }

    fclose(inFile);
    /*Releasing the memory*/
    free(mainMemory);
    free(secMemory);

    return 0;
}

/*Service functions go here*/
int numFromString(char* number)
{
    int value = 0, i = 0;
    while(number[i] != '\0')
    {
        value = value*10 + number[i]-48;
        i++;
    }
    return value;
}

void memInit(page** memory, int size)
{
    int i;
    *memory = (page*)malloc(size*sizeof(page));
    for(i=0; i<size; i++)
    {
        (*memory)[i].pageNumber = -1;
        strcpy((*memory)[i].contents, "\0");
        (*memory)[i].secondChance = 0;
        (*memory)[i].timer = 0;
    }
}

void readFromPage()
{
}

void writeToPage()
{
}

void printMem()
{
    int i;
    printf("secondaryMemory = [");
    /*secMem printing goes here*/
    printf(" SECONDARY MEMORY ");
    /*End secMem printing*/
    printf("]\n");
}