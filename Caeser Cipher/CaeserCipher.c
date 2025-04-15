#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void encrypt(char* str,int length, int shift){
    for (int i = 0; i < length; ++i){
        // printf("%c ",str[i]);
        // fflush(stdout);
        if (!isalpha(str[i]))
            continue;
        if (islower(str[i])){
            str[i] += shift;
            if (str[i] > 'z')
                str[i] -= 26;
        }
        else {
            str[i] += shift;
            if (str[i] > 'Z')
                str[i] -= 26;
        }
    }
}

void decrypt(char* str,int length, int shift){
    for (int i = 0; i < length; ++i){
        if (!isalpha(str[i]))
            continue;
        if (islower(str[i])){
            str[i] -= shift;
            if (str[i] < 'a')
                str[i] += 26;
        }
        else {
            str[i] -= shift;
            if (str[i] < 'A')
                str[i] += 26;
        }
    }
}

int getInputOption(){
    int option = 0;
    printf("Enter 1 for console input, 2 for file input: ");
    fflush(stdout);
    scanf("%d", &option);
    // fflush(stdin);
    while(option != 1 && option != 2){
        printf ("Invalid option. Enter 1 for console input, 2 for file input: ");
        fflush(stdout);
        scanf("%d", &option);
        fflush(stdin);
    }
    return option;
}

int getEncrytionOption(){
    int option = 0;
    printf("Enter 1 to encrypt, 2 to decrypt: ");
    fflush(stdout);
    scanf("%d", &option);
    // fflush(stdin);
    while(option != 1 && option != 2){
        printf ("Invalid option. Enter 1 to encrypt, 2 to decrypt: ");
        fflush(stdout);
        scanf("%d", &option);
        // fflush(stdin);
    }
    return option;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    int processRank, numOfProcessors;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcessors);
    MPI_Status status;
    char* str;
    int inputOption,encryptionOption,strLength,shift = 3;
    if (processRank == 0){
        inputOption = getInputOption();
        strLength = 0;
        if (inputOption == 1){
            encryptionOption = getEncrytionOption();
            printf("Enter the size of the string: ");
            fflush(stdout);
            scanf("%d", &strLength);
            str = (char*)malloc((strLength + 1) * sizeof(char));
            printf("Enter the string: ");
            fflush(stdout);
            scanf("%s", str);
            // fflush(stdin);
        }
        else {
            FILE* input = fopen("input.txt", "r");
            if (input == NULL){
                printf("Error opening file.\n");
                return 1;
            }
            fscanf(input, "%d", &encryptionOption);
            fscanf(input, "%d", &strLength);
            str = (char*)malloc((strLength + 1) * sizeof(char));
            fscanf(input, "%s", str);
            fclose(input);
        }

        int portionSize = strLength/numOfProcessors;

        fflush(stdout);
        for (int i = 1; i < numOfProcessors; ++i){
            MPI_Send(&encryptionOption,1,MPI_INT,i,0,MPI_COMM_WORLD);
            MPI_Send(&portionSize,1,MPI_INT,i,0,MPI_COMM_WORLD);
            MPI_Send(str + i * portionSize, portionSize,MPI_CHAR,i,0,MPI_COMM_WORLD);
        }

        if (encryptionOption == 1)
            encrypt(str, portionSize, shift);
        else
            decrypt(str, portionSize, shift);

        for (int i = 1; i < numOfProcessors; ++i) 
            MPI_Recv(str + portionSize * i, portionSize, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
        
        printf(encryptionOption == 1 ? "Encrypted string: %s\n" : "Decrypted string: %s\n", str);
    }
    else{
        MPI_Recv(&encryptionOption,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        MPI_Recv(&strLength,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
        str = (char*) malloc(strLength * sizeof(char));
        MPI_Recv(str,strLength,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);

        if (encryptionOption == 1)
            encrypt(str,strLength,shift);
        else
            decrypt(str,strLength,shift);
        
        MPI_Send(str,strLength,MPI_CHAR,0,0,MPI_COMM_WORLD);
    }
    free(str);
    MPI_Finalize();
}