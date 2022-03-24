#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>

int getDim(char* str)
{
    int res = 0;
    if(str[0]=='-')
        res = -1;
    for (int i = 0;str[i] != '\0'; i++) {
        if(isdigit(str[i]))
            res = res * 10 + str[i] - '0';
    }
    return res;
}
int getElement(char* str){
    int res = 0;
    for (int i = 0;str[i] != '\0' ; i++) {
        if(isdigit(str[i]))
            res = res * 10 + str[i] - '0';
       else {
            printf("data contains string");
            exit(1);
        }
    }
    if(str[0]=='-')
        res *= -1;
    return res;
}

void getDimensions(char* filename , int* row ,int* column) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
        exit(5);
    }
    const unsigned MAX_LENGTH = 1024;
    char buffer[MAX_LENGTH];
    if(fgets(buffer, MAX_LENGTH, fp)){

        char *token = strtok(buffer, " ");
        *row= getDim(token);

        token = strtok(NULL, " ");
        *column= getDim(token);
    }
    else{
        printf("file : %s is empty ",filename);
    }
    fclose(fp);
}
int** readFromFile(char* filename , int row ,int column){
    FILE *fp = fopen(filename, "r");
    const unsigned MAX_LENGTH = 1024;
    char buffer[MAX_LENGTH];

    int** matrix = (int**)malloc(row * sizeof(int*));
    for (int i = 0; i < row; i++)
        matrix[i] = (int*)malloc(column * sizeof(int));

    //move pointer (ignore row and column) already read it
    fgets(buffer, MAX_LENGTH, fp);
    for(int i=0;i<row;i++){
        for(int j=0;j<column;j++){
            if(fscanf(fp,"%s ", buffer)>0) {
               matrix[i][j]=getElement(buffer);
           }
       }
    }
    fclose(fp);
    return matrix;

}


int main(int argc, char **argv)
{
    if(argc>4)
    {
        printf("Error to many arguments");
        return 1;
    }
    char** filenames = malloc(sizeof (char *) *3);
    filenames[0]= malloc(sizeof (char)*128);
    filenames[1]= malloc(sizeof (char)*128);
    filenames[2]= malloc(sizeof (char)*128);
    strcpy(filenames[0],"a");
    strcpy(filenames[1],"b");
    strcpy(filenames[2],"c");
    for(int i=1;i<argc ;i++){
        strcpy(filenames[i-1],argv[i]);
    }
    for(int i=0;i<3;i++){
        strncat(filenames[i],".txt",4);
    }
    int ra;
    int ca;
    int rb;
    int cb;
    getDimensions(filenames[0],&ra,&ca);
    int** matrixA = readFromFile(filenames[0],ra,ca);

    for (int i = 0; i < ra; i++) {
        for (int j = 0; j < ca; j++)
            printf("%d ",matrixA[i][j]);
        printf("\n");
    }
    getDimensions(filenames[1],&rb,&cb);
    int** matrixB = readFromFile(filenames[0],rb,cb);

    for (int i = 0; i < rb; i++) {
        for (int j = 0; j < cb; j++)
            printf("%d ",matrixB[i][j]);
        printf("\n");
    }
    return 0;
}