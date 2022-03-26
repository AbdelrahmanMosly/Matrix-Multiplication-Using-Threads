#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>


typedef struct matricesData {
    int*** matrixA;
    int*** matrixB ;

    int elementToWorkOn;
    int rowToWorkOn;
    int* ra;
    int* ca;
    int* rb;
    int* cb;
}MatricesData;

int** matrixCWhole;
int** matrixCRowThread;
int** matrixCElementThread;


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
int** readFromFile(char* filename , int row ,int column) {
    FILE *fp = fopen(filename, "r");
    const unsigned MAX_LENGTH = 1024;
    char buffer[MAX_LENGTH];

    int **matrix = (int **) malloc(row * sizeof(int *));
    for (int i = 0; i < row; i++)
        matrix[i] = (int *) malloc(column * sizeof(int));

    //move pointer (ignore row and column) already read it
    fgets(buffer, MAX_LENGTH, fp);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            if (fscanf(fp, "%s ", buffer) > 0) {
                matrix[i][j] = getElement(buffer);
            }
        }
    }
    fclose(fp);
    return matrix;

}


void writeToFile(char* filename,int** matrixC,char* method , int row ,int column) {
    strcat(filename,method);
    FILE *fp = fopen(filename, "w");

    for (int i = 0; i <row; i++) {
        for (int j = 0; j <column; j++)
            fprintf(fp,"%d ",matrixC[i][j]);
        fprintf(fp,"\n");
    }
    fclose(fp);
}



MatricesData generateMatricesData(char** filenames){

    int ra;
    int ca;
    int rb;
    int cb;
    MatricesData matricesData;

    matricesData.matrixA= malloc(sizeof (int **));
    matricesData.matrixB= malloc(sizeof (int **));
    matricesData.ra= malloc(sizeof (int ));
    matricesData.ca= malloc(sizeof (int ));
    matricesData.rb= malloc(sizeof(int ));
    matricesData.cb= malloc(sizeof (int ));

    getDimensions(filenames[0],&ra,&ca);
    *matricesData.ra=ra;
    *matricesData.ca=ca;
    *matricesData.matrixA = readFromFile(filenames[0],ra,ca);
    getDimensions(filenames[1],&rb,&cb);
    *matricesData.rb=rb;
    *matricesData.cb=cb;
    *matricesData.matrixB = readFromFile(filenames[0],rb,cb);
    matricesData.elementToWorkOn=0;
    matricesData.rowToWorkOn=0;

    return matricesData;
}
void printMatrixC(int** matrixC,MatricesData matricesData)
{
    for (int i = 0; i < *matricesData.ra; i++) {
        for (int j = 0; j < *matricesData.cb; j++)
            printf("%d ",matrixC[i][j]);
        printf("\n");
    }
}
int** allocateMatrixC(MatricesData matricesData) {
    int** arr = (int**)malloc(*matricesData.ra * sizeof(int*));
    for (int i = 0; i < *matricesData.ra; i++)
        arr[i] = (int*)malloc(*matricesData.cb * sizeof(int));
    return arr;
}

void* multiplyWhole(void * arg){
    MatricesData* matricesData=(MatricesData*) arg;
    for (int i = 0; i < *matricesData->ra; i++) {
        for (int j = 0; j <*matricesData-> cb; j++){
            matrixCWhole[i][j] = 0;

            for (int k = 0; k < *matricesData-> rb; k++)
                matrixCWhole[i][j] += (*matricesData->matrixA)[i][k]* (*matricesData->matrixB)[k][j];


        }
    }
}
void* multiplyRow(void * arg){
    MatricesData* matricesData=(MatricesData*) arg;
    int i=matricesData->rowToWorkOn;
    for (int j = 0; j < *matricesData-> cb; j++){
        matrixCRowThread[i][j] = 0;

        for (int k = 0; k < *matricesData-> rb; k++)
            matrixCRowThread[i][j] += (*matricesData->matrixA)[i][k] * (*matricesData->matrixB)[k][j];
    }
}
void* multiplyElement(void * arg){
    MatricesData* matricesData=(MatricesData*) arg;
    int i=matricesData->elementToWorkOn;
    int elementi=i/(*matricesData->ra);
    int elementj=i%(*matricesData->ra);

    matrixCElementThread[elementi][elementj] = 0;

    for (int k = 0; k < (*matricesData-> rb); k++)
        matrixCElementThread[elementi][elementj] += (*matricesData->matrixA)[elementi][k] * (*matricesData->matrixB)[k][elementj];

}


int main(int argc, char **argv)
{
    if(argc>4)
    {
        printf("Error to many arguments");
        return 1;
    }
    char** filenames = malloc(sizeof (char *) *4);
    filenames[0]= malloc(sizeof (char)*128);
    filenames[1]= malloc(sizeof (char)*128);
    filenames[2]= malloc(sizeof (char)*128);
    filenames[3]= malloc(sizeof (char)*128);
    strcpy(filenames[0],"a");
    strcpy(filenames[1],"b");
    strcpy(filenames[2],"c");
    for(int i=1;i<argc ;i++){
        strcpy(filenames[i-1],argv[i]);
    }
    for(int i=0;i<2;i++){
        strncat(filenames[i],".txt",4);
    }

    MatricesData matricesData = generateMatricesData(filenames);
    int ra=*matricesData.ra;
    int cb=*matricesData.cb;
    pthread_t threads[ra*cb+ra+1]; //each element +each row+whole
    MatricesData matricesDataArr[ra*cb+ra+1];
    matrixCWhole= allocateMatrixC(matricesData);
    matrixCRowThread= allocateMatrixC(matricesData);
    matrixCElementThread= allocateMatrixC(matricesData);

    int threadIndex=0;
    matricesDataArr[threadIndex]=matricesData;
    pthread_create(&threads[threadIndex++], NULL, multiplyWhole,(void*)&matricesDataArr[threadIndex]);

    while (threadIndex<ra+1){
        matricesDataArr[threadIndex]=matricesData;
        pthread_create(&threads[threadIndex++], NULL, multiplyRow,(void*)&matricesDataArr[threadIndex]);
        matricesData.rowToWorkOn++;
    }
    matricesData.elementToWorkOn=0;
    while (threadIndex<ra*cb+ra+1){
        matricesDataArr[threadIndex]=matricesData;
        pthread_create(&threads[threadIndex++], NULL, multiplyElement,(void*)&matricesDataArr[threadIndex]);
        matricesData.elementToWorkOn++;
    }
    threadIndex--;
    while (threadIndex>=0)
        pthread_join(threads[threadIndex--],NULL);


    strcpy(filenames[3],filenames[2]);
    writeToFile(filenames[2],matrixCWhole ,"_per_matrix",ra,cb);
    strcpy(filenames[2],filenames[3]);
    writeToFile(filenames[3],matrixCRowThread ,"_per_row",ra,cb);
    writeToFile(filenames[2],matrixCElementThread ,"_per_element",ra,cb);



    printf("results one thread :////////////////////\n");
    printMatrixC(matrixCWhole,matricesData);
    printf("results row thread :////////////////////\n");
    printMatrixC(matrixCRowThread,matricesData);
    printf("results element thread :////////////////////\n");
    printMatrixC(matrixCElementThread,matricesData);
    return 0;
}