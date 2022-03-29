#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

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
    if(str[0]=='-'){
        printf("dimenstion cant be negative");
        exit(5);
    }
    for (int i = 0;str[i] != '\0'; i++) {
        if(isdigit(str[i]))
            res = res * 10 + str[i] - '0';
        else{
            printf("string found in the dimensions");
            exit(6);
        }
    }
    return res;
}
int getElement(char* str){
    int res = 0;
    int i=0;
    if(str[i]=='-')
        i++;  //skip negative sign
    for (;str[i] != '\0' ; i++) {
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
    char* rowString= malloc(sizeof (char)*10);
    char* colString= malloc(sizeof (char)*10);
    if(fscanf(fp,"row=%s col=%s",rowString,colString)>0){
        *row= getDim(rowString);
        *column= getDim(colString);
    }
    else{
        printf("file : %s is empty ",filename);
    }
    fclose(fp);
    free(rowString);
    free(colString);
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
            }else{
                printf("dimesnions mismatch data");
                exit(2);
            }
        }
    }
    fclose(fp);
    return matrix;

}


void writeToFile(char* filename,int** matrixC,char* method , int row ,int column) {
    strcat(filename,method);
    FILE *fp = fopen(filename, "w");
    fprintf(fp,"%s\n",filename);
    fprintf(fp,"row=%d col=%d\n",row,column);

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

    matricesData.matrixA= (int***)malloc(sizeof (int **));
    matricesData.matrixB= (int***)malloc(sizeof (int **));
    matricesData.ra= (int*)malloc(sizeof (int ));
    matricesData.ca= (int*)malloc(sizeof (int ));
    matricesData.rb= (int*)malloc(sizeof(int ));
    matricesData.cb= (int*)malloc(sizeof (int ));

    getDimensions(filenames[0],&ra,&ca);
    *matricesData.ra=ra;
    *matricesData.ca=ca;
    *matricesData.matrixA = readFromFile(filenames[0],ra,ca);
    getDimensions(filenames[1],&rb,&cb);
    *matricesData.rb=rb;
    *matricesData.cb=cb;
    *matricesData.matrixB = readFromFile(filenames[1],rb,cb);
    matricesData.elementToWorkOn=0;
    matricesData.rowToWorkOn=0;

    if(ca!=rb)
    {
        printf("cant multiply these matrices");
        exit(3);
    }

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
clearMemory(char** filenames,MatricesData* matricesData){


    for(int i=0;i<4;i++)
        free(filenames[i]);
    free(filenames);

    for(int i=0; i< *matricesData->ra;i++) {
        free(matricesData->matrixA[0][i]);
    }
    free(matricesData->matrixA[0]);
    free(matricesData->matrixA);
    for (int i = 0; i < *matricesData->rb; i++) {
        free(matricesData->matrixB[0][i]);
    }
    free(matricesData->matrixB[0]);
    free(matricesData->matrixB);
    for (int i = 0; i < *matricesData->ra; i++) {
        free(matrixCWhole[i]);
        free(matrixCRowThread[i]);
        free(matrixCElementThread[i]);
    }


    free(matricesData->ra);
    free(matricesData->ca);
    free(matricesData->cb);
    free(matricesData->rb);
    free(matrixCWhole);
    free(matrixCElementThread);
    free(matrixCRowThread);


}
void print(MatricesData matricesData){
    printf("results one thread :////////////////////\n");
    printMatrixC(matrixCWhole,matricesData);
    printf("results row thread :////////////////////\n");
    printMatrixC(matrixCRowThread,matricesData);
    printf("results element thread :////////////////////\n");
    printMatrixC(matrixCElementThread,matricesData);
}
MatricesData* geDataArray(MatricesData matricesData,int size){
    MatricesData* matricesDataArr= malloc(sizeof (matricesData)*size);
    for(int i=0;i<size;i++)
    {
        matricesDataArr[i]=matricesData;
    }
    return matricesDataArr;
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
    MatricesData* matricesDataArr;
    matrixCWhole= allocateMatrixC(matricesData);
    matrixCRowThread= allocateMatrixC(matricesData);
    matrixCElementThread= allocateMatrixC(matricesData);

    int threadIndex=0;
    matricesDataArr=geDataArray(matricesData,ra*cb);


    struct timeval stop, start;

    //--per Matrix start
    gettimeofday(&start, NULL); //start checking time
    pthread_create(&threads[threadIndex++], NULL, multiplyWhole,(void*)&matricesDataArr[threadIndex]);
    threadIndex--;
    while (threadIndex>=0)
        pthread_join(threads[threadIndex--],NULL);
    gettimeofday(&stop, NULL); //end checking time
    printf("Thread per Matrix seconds taken %lu s\n", stop.tv_sec - start.tv_sec);
    printf("Thread per Matrix Microseconds taken: %lu microseconds\n", stop.tv_usec - start.tv_usec);
    threadIndex++;
    //--per Matrix End

    //--per Row start
    gettimeofday(&start, NULL); //start checking time
    while (threadIndex<ra){
        matricesDataArr[threadIndex].rowToWorkOn=threadIndex;
        pthread_create(&threads[threadIndex++], NULL, multiplyRow,(void*)&matricesDataArr[threadIndex]);
    }
    threadIndex--;
    while (threadIndex>=0)
        pthread_join(threads[threadIndex--],NULL);
    gettimeofday(&stop, NULL); //end checking time
    printf("Thread per Row Seconds taken %lu s\n", stop.tv_sec - start.tv_sec);
    printf("Thread per Row Microseconds taken: %lu microseconds\n", stop.tv_usec - start.tv_usec);
    threadIndex++;
    //--per Row End

    //--perElement start
    gettimeofday(&start, NULL); //start checking time
    matricesData.elementToWorkOn=0;
    while (threadIndex<ra*cb){
        matricesDataArr[threadIndex].elementToWorkOn=threadIndex;
        pthread_create(&threads[threadIndex++], NULL, multiplyElement,(void*)&matricesDataArr[threadIndex]);
    }
    threadIndex--;
    while (threadIndex>=0)
        pthread_join(threads[threadIndex--],NULL);
    gettimeofday(&stop, NULL); //end checking time
    printf("Thread per Element Seconds taken %lu s\n", stop.tv_sec - start.tv_sec);
    printf("Thread per Element Microseconds taken: %lu microseconds\n", stop.tv_usec - start.tv_usec);
    threadIndex++;
    //--perElement End


    strcpy(filenames[3],filenames[2]);
    writeToFile(filenames[2],matrixCWhole ,"_per_matrix",ra,cb);
    strcpy(filenames[2],filenames[3]);
    writeToFile(filenames[3],matrixCRowThread ,"_per_row",ra,cb);
    writeToFile(filenames[2],matrixCElementThread ,"_per_element",ra,cb);

    print(matricesData);

    free(matricesDataArr);
    clearMemory(filenames,&matricesData);



    return 0;
}