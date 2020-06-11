#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Page structure*/
typedef struct{
    char contents[64];
    int pageNumber;
    int timer;
    int secondChance;
} page;

typedef struct{
    char contents[64];
} mempage;

int numFromString(char*);
void memInit(page**, int);
void printMem(mempage*, int, FILE*);

int main(int argc, char* argv[])
{
    char inFileName[64], outFileName[64], c[2];
    char *line = NULL, param[32];
    int isLRU = 0, i, j, pageNum, pageIndex, targetPage, maxTime;
    int emptyPageFlag, emptyPageIndex;
    int existingPageFlag, existingPageIndex;
    int mainMemSize, secMemSize;
    FILE *inFile, *outFile;
    page *mainMemory;
    mempage *secMemory;
    size_t len = 0;
    ssize_t read;

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
    secMemory = (mempage*)malloc(secMemSize*sizeof(mempage));
    for(i=0; i<secMemSize; i++) strcpy(secMemory[i].contents, "\0");

    /*Get all data into secondary memory array*/
    inFile = fopen(inFileName, "r");
    if(inFile == NULL)
    {
        printf("Error reading file. Try again.\n");
        return 1;
    }
    outFile = fopen(outFileName, "w");
    if(outFile == NULL)
    {
        printf("Error writing to file. Try again.\n");
        return 1;
    }

    while((read = getline(&line, &len, inFile)) != -1)
    {
        emptyPageFlag = 0;
        existingPageFlag = 0;
        switch(line[0])
        {
            case 'r':
                for(i=0; line[i]!=' '; i++); //Skip all instruction letters
                i++;
                j=0;
                while (line[i]>='0' && line[i]<='9') param[j++] = line[i++];
                param[j] = '\0';
                pageNum = numFromString(param);


                /*Advance the timer on each existing page*/
                for(i=0; i<mainMemSize; i++)
                    if(mainMemory[i].pageNumber != -1)
                        mainMemory[i].timer++;

                /*Choose the correct strategy for writing to a page*/
                if(isLRU)
                {
                    /*Check if a free page exists in the main memory*/
                    for(i=0; i<mainMemSize; i++)
                    {
                        if(mainMemory[i].pageNumber == pageNum)
                        {
                            existingPageFlag = 1;
                            existingPageIndex = i;
                        }
                    }
                    
                    if(existingPageFlag) //Existing page printing
                    {
                        mainMemory[existingPageIndex].timer = 0;
                        printf("Page %d contains string '%s'\n", existingPageIndex, mainMemory[existingPageIndex].contents);
                    }
                    else //Page replacement and printing
                    {
                        maxTime = 0;
                        for(i=0; i<mainMemSize; i++)
                        {
                            if(mainMemory[i].timer > maxTime) //Find the oldest page
                            {
                                maxTime = mainMemory[i].timer;
                                pageIndex = i;
                                targetPage = mainMemory[i].pageNumber;
                            }
                        }
                        
                        strcat(secMemory[targetPage].contents, mainMemory[pageIndex].contents); //Copy data to secMemory page

                        mainMemory[pageIndex].pageNumber = pageNum; //Change the target page number
                        strcpy(mainMemory[pageIndex].contents, secMemory[pageNum].contents); //Overwrite the contents with uploaded page's contents
                        mainMemory[pageIndex].timer = 0; //Set timer to 0

                        /*Print the contents of the uploaded page*/
                        printf("Page %d contains string '%s'\n", mainMemory[pageIndex].pageNumber, mainMemory[pageIndex].contents);
                    }
                }
                else
                {
                    /*Check if a free page exists in the main memory*/
                    for(i=0; i<mainMemSize; i++)
                    {
                        if(mainMemory[i].pageNumber == pageNum)
                        {
                            existingPageFlag = 1;
                            existingPageIndex = i;
                        }
                    }
                    
                    if(existingPageFlag) //Existing page modification
                    {
                        printf("Page %d contains string '%s'\n", existingPageIndex, mainMemory[existingPageIndex].contents);
                        mainMemory[existingPageIndex].secondChance = 1; //Set second chance counter to 1
                    }
                    else //Page replacement
                    {
                        /*Mark all max time pages with SC=1, since they're "saved" from erasing*/
                        for(j=0; j<mainMemSize; j++)
                        {
                            maxTime = 0;
                            for(i=0; i<mainMemSize; i++)
                            {
                                if(mainMemory[i].timer > maxTime)
                                {
                                    if(mainMemory[i].secondChance != 2)
                                    {
                                        maxTime = mainMemory[i].timer;
                                        pageIndex = i;
                                    }
                                }
                            }
                            /*Mark any SC page as "safe"*/
                            if(mainMemory[pageIndex].secondChance == 1)
                            {
                                mainMemory[pageIndex].secondChance = 2;
                            }
                            /*Find the first SC=0 page and save it as the target page*/
                            else if(mainMemory[pageIndex].secondChance == 0)
                            {
                                targetPage = mainMemory[pageIndex].pageNumber;
                                break;
                            }
                        }
                        
                        /*Reset all SC=2 pages to SC=0 since they're not safe anymore*/
                        for(i=0; i<mainMemSize; i++)
                        {
                            if(mainMemory[i].secondChance == 2)
                            {
                                mainMemory[i].secondChance = 0;
                            }
                        }

                        strcat(secMemory[targetPage].contents, mainMemory[pageIndex].contents); //Copy data to secMemory page

                        mainMemory[pageIndex].pageNumber = pageNum; //Change the target page number
                        strcpy(mainMemory[pageIndex].contents, secMemory[pageNum].contents); //Overwrite the contents with uploaded page's contents
                        mainMemory[pageIndex].timer = 0; //Set timer to 0
                        mainMemory[pageIndex].secondChance = 0; //Set second chance counter to 0
                        
                        /*Print the contents of the uploaded page*/
                        printf("Page %d contains string '%s'\n", targetPage, mainMemory[pageIndex].contents);
                    }
                }
                break;
            case 'w':
                for(i=0; line[i]!=' '; i++); //Skip all instruction letters
                i++;
                j=0;
                while (line[i]!=' ') param[j++] = line[i++];
                param[j] = '\0';
                pageNum = numFromString(param);
                c[0] = line[i+1];
                c[1] = '\0';

                /*Advance the timer on each existing page*/
                for(i=0; i<mainMemSize; i++)
                    if(mainMemory[i].pageNumber != -1)
                        mainMemory[i].timer++;

                /*Choose the correct strategy for writing to a page*/
                if(isLRU)
                {
                    /*Check if a free page exists in the main memory*/
                    for(i=0; i<mainMemSize; i++)
                    {
                        if(mainMemory[i].pageNumber == pageNum)
                        {
                            existingPageFlag = 1;
                            existingPageIndex = i;
                        }
                        if(mainMemory[i].pageNumber == -1)
                        {
                            if(!emptyPageFlag)
                            {
                                emptyPageFlag = 1;
                                emptyPageIndex = i;
                            }
                        }
                    }
                    
                    if(existingPageFlag) //Existing page modification
                    {
                        mainMemory[existingPageIndex].timer = 0;
                        strcat(mainMemory[existingPageIndex].contents, c);
                    }
                    else if(emptyPageFlag) //New page addition to free space
                    {
                        mainMemory[emptyPageIndex].timer = 0;
                        mainMemory[emptyPageIndex].pageNumber = pageNum;
                        strcat(mainMemory[emptyPageIndex].contents, c);
                    }
                    else //Page replacement
                    {
                        maxTime = 0;
                        for(i=0; i<mainMemSize; i++)
                        {
                            if(mainMemory[i].timer > maxTime) //Find the oldest page
                            {
                                maxTime = mainMemory[i].timer;
                                pageIndex = i;
                                targetPage = mainMemory[i].pageNumber;
                            }
                        }

                        strcat(secMemory[targetPage].contents, mainMemory[pageIndex].contents); //Copy data to secMemory page

                        mainMemory[pageIndex].pageNumber = pageNum; //Change the target page number
                        strcpy(mainMemory[pageIndex].contents, secMemory[pageNum].contents); //Overwrite the contents with uploaded page's contents
                        mainMemory[pageIndex].timer = 0; //Set timer to 0
                        strcat(mainMemory[pageIndex].contents, c); //Append data to the page
                    }
                    
                }
                else
                {
                    /*Check if a free page exists in the main memory*/
                    for(i=0; i<mainMemSize; i++)
                    {
                        if(mainMemory[i].pageNumber == pageNum)
                        {
                            existingPageFlag = 1;
                            existingPageIndex = i;
                        }
                        if(mainMemory[i].pageNumber == -1)
                        {
                            if(!emptyPageFlag)
                            {
                                emptyPageFlag = 1;
                                emptyPageIndex = i;
                            }
                        }
                    }
                    
                    if(existingPageFlag) //Existing page modification
                    {
                        strcat(mainMemory[existingPageIndex].contents, c);
                        mainMemory[existingPageIndex].secondChance = 1; //Set second chance counter to 1
                    }
                    else if(emptyPageFlag) //New page addition to free space
                    {
                        mainMemory[emptyPageIndex].timer = 0;
                        mainMemory[emptyPageIndex].pageNumber = pageNum;
                        mainMemory[emptyPageIndex].secondChance = 0; //Set second chance counter to 1
                        strcat(mainMemory[emptyPageIndex].contents, c);
                    }
                    else //Page replacement
                    {
                        /*Mark all max time pages with SC=1, since they're "saved" from erasing*/
                        for(j=0; j<mainMemSize; j++)
                        {
                            maxTime = 0;
                            for(i=0; i<mainMemSize; i++)
                            {
                                if(mainMemory[i].timer > maxTime)
                                {
                                    if(mainMemory[i].secondChance != 2)
                                    {
                                        maxTime = mainMemory[i].timer;
                                        pageIndex = i;
                                    }
                                }
                            }
                            /*Mark any SC page as "safe"*/
                            if(mainMemory[pageIndex].secondChance == 1)
                            {
                                mainMemory[pageIndex].secondChance = 2;
                            }
                            /*Find the first SC=0 page and save it as the target page*/
                            else if(mainMemory[pageIndex].secondChance == 0)
                            {
                                targetPage = mainMemory[pageIndex].pageNumber;
                                break;
                            }
                        }
                        
                        /*Reset all SC=2 pages to SC=0 since they're not safe anymore*/
                        for(i=0; i<mainMemSize; i++)
                        {
                            if(mainMemory[i].secondChance == 2)
                            {
                                mainMemory[i].secondChance = 0;
                            }
                        }

                        strcat(secMemory[targetPage].contents, mainMemory[pageIndex].contents); //Copy data to secMemory page

                        mainMemory[pageIndex].pageNumber = pageNum; //Change the target page number
                        strcpy(mainMemory[pageIndex].contents, secMemory[pageNum].contents); //Overwrite the contents with uploaded page's contents
                        mainMemory[pageIndex].timer = 0; //Set timer to 0
                        mainMemory[pageIndex].secondChance = 0; //Set second chance counter to 0
                        strcat(mainMemory[pageIndex].contents, c); //Append data to the page
                    }
                }
                break;
            case 'p':
                printMem(secMemory, secMemSize, outFile);
                break;
            default:
                printf("Unknown instruction\n");
        }
    }

    fclose(inFile);
    fclose(outFile);
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
        strcpy((*memory)[i].contents, "\0");
        (*memory)[i].pageNumber = -1;
        (*memory)[i].secondChance = 0;
        (*memory)[i].timer = 0;
    }
}

void printMem(mempage* memory, int size, FILE* file)
{
    int i;
    fprintf(file, "secondaryMemory=[");
    /*secMem printing goes here*/
    for(i=0; i<size; i++)
    {
        if(i!=size-1) fprintf(file, "%s, ", memory[i].contents);
        else fprintf(file, "%s", memory[i].contents);
    }
    /*End secMem printing*/
    fprintf(file, "]\n");
}