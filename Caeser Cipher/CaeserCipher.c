#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Encrypts the given string using Caesar Cipher with the specified shift
void encrypt(char* str, int length, int shift) {
    for (int i = 0; i < length; ++i) {
        if (!isalpha(str[i])) // Skip non-alphabetic characters
            continue;
        if (islower(str[i])) { // Handle lowercase letters
            str[i] += shift;
            if (str[i] > 'z')
                str[i] -= 26;
        } else { // Handle uppercase letters
            str[i] += shift;
            if (str[i] > 'Z')
                str[i] -= 26;
        }
    }
}

// Decrypts the given string using Caesar Cipher with the specified shift
void decrypt(char* str, int length, int shift) {
    for (int i = 0; i < length; ++i) {
        if (!isalpha(str[i])) // Skip non-alphabetic characters
            continue;
        if (islower(str[i])) { // Handle lowercase letters
            str[i] -= shift;
            if (str[i] < 'a')
                str[i] += 26;
        } else { // Handle uppercase letters
            str[i] -= shift;
            if (str[i] < 'A')
                str[i] += 26;
        }
    }
}

// Prompts the user to choose between console input or file input
int getInputOption() {
    int option = 0;
    printf("Enter 1 for console input, 2 for file input: ");
    fflush(stdout);
    scanf("%d", &option);
    while (option != 1 && option != 2) { // Validate input
        printf("Invalid option. Enter 1 for console input, 2 for file input: ");
        fflush(stdout);
        scanf("%d", &option);
        fflush(stdin);
    }
    return option;
}

// Prompts the user to choose between encryption or decryption
int getEncrytionOption() {
    int option = 0;
    printf("Enter 1 to encrypt, 2 to decrypt: ");
    fflush(stdout);
    scanf("%d", &option);
    while (option != 1 && option != 2) { // Validate input
        printf("Invalid option. Enter 1 to encrypt, 2 to decrypt: ");
        fflush(stdout);
        scanf("%d", &option);
    }
    return option;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize MPI environment
    int processRank, numOfProcessors;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank); // Get the rank of the current process
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcessors); // Get the total number of processes
    MPI_Status status;
    char* str;
    int inputOption, encryptionOption, strLength, shift = 3;

    if (processRank == 0) { // Root process
        inputOption = getInputOption(); // Get input method
        strLength = 0;

        if (inputOption == 1) { // Console input
            encryptionOption = getEncrytionOption(); // Get encryption/decryption option
            printf("Enter the size of the string: ");
            fflush(stdout);
            scanf("%d", &strLength);
            str = (char*)malloc((strLength + 1) * sizeof(char)); // Allocate memory for the string
            printf("Enter the string: ");
            fflush(stdout);
            scanf("%s", str);
        } else { // File input
            FILE* input = fopen("input.txt", "r");
            if (input == NULL) {
                printf("Error opening file.\n");
                return 1;
            }
            fscanf(input, "%d", &encryptionOption); // Read encryption/decryption option
            fscanf(input, "%d", &strLength); // Read string length
            str = (char*)malloc((strLength + 1) * sizeof(char)); // Allocate memory for the string
            fscanf(input, "%s", str); // Read the string
            fclose(input);
        }

        int portionSize = strLength / numOfProcessors; // Divide the string into equal portions
        int rem=strLength%numOfProcessors;
        int prev=portionSize+(0<rem? 1 :0);
        // Send portions of the string and options to other processes
        for (int i = 1; i < numOfProcessors; ++i) {
            int currentPortionSize=portionSize+(i<rem? 1 :0);
            MPI_Send(&encryptionOption, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&currentPortionSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(str + prev, currentPortionSize, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            prev+=currentPortionSize;
        }

        prev=portionSize+(0<rem? 1 :0);

        // Process the first portion in the root process
        if (encryptionOption == 1)
            encrypt(str, prev, shift);
        else
            decrypt(str, prev, shift);

        // Receive processed portions from other processes
        for (int i = 1; i < numOfProcessors; ++i){
            int currentPortionSize=portionSize+(i<rem? 1 :0);
            MPI_Recv(str + prev,currentPortionSize, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
            prev+=currentPortionSize;
        }
        // Print the final result
        printf(encryptionOption == 1 ? "Encrypted string: %s\n" : "Decrypted string: %s\n", str);
    } else { // Worker processes
        MPI_Recv(&encryptionOption, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); // Receive encryption/decryption option
        MPI_Recv(&strLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); // Receive portion size
        str = (char*)malloc(strLength * sizeof(char)); // Allocate memory for the portion
        MPI_Recv(str, strLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status); // Receive the portion

        // Process the received portion
        if (encryptionOption == 1)
            encrypt(str, strLength, shift);
        else
            decrypt(str, strLength, shift);

        // Send the processed portion back to the root process
        MPI_Send(str, strLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    free(str); // Free allocated memory
    MPI_Finalize(); // Finalize MPI environment
}