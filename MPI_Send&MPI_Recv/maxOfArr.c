#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

void getMax(int *arr, int size, int *maxVal, int *maxIdx) {
    *maxVal = arr[0];
    *maxIdx = 0;
    for(int i = 1; i < size; ++i) {
        if(arr[i] > *maxVal) {
            *maxVal = arr[i];
            *maxIdx = i;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n, *arr = NULL, *pArr = NULL;
    int pSize, pMax, pMaxIdx;

    if(rank == 0) { // Master
        printf("Enter array size: ");
        fflush(stdout);
        scanf("%d", &n);

        arr = (int*)malloc(n * sizeof(int));
        printf("Enter array elements: ");
        fflush(stdout);
        for(int i = 0; i < n; ++i) {
            scanf("%d", &arr[i]);
        }

        // distribute work among slaves
        pSize = n / size; // size of each slave's array
        for (int i = 1; i < size; ++i) {
            MPI_Send(&pSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(arr + i * pSize, pSize, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // master's task
        getMax(arr, pSize, &pMax, &pMaxIdx);

        // collect results
        int finalMax = pMax, finalMaxIdx = pMaxIdx;
        for(int i = 1; i < size; ++i) {
            int tmpMax, tmpMaxIdx;
            MPI_Recv(&tmpMax, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&tmpMaxIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if(tmpMax > finalMax) {
                finalMax = tmpMax;
                finalMaxIdx = tmpMaxIdx + i * pSize;
            }
        }
        printf("Max element: %d\nMax element index: %d\n", finalMax, finalMaxIdx);
        fflush(stdout);
        free(arr); // free allocated memory for whole array

    } else { // Slaves
        MPI_Recv(&pSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recieve size of partition
        pArr = (int*)malloc(pSize * sizeof(int)); // allocate memory with recieved size
        MPI_Recv(pArr, pSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //recieve values of partition in the allocated memory

        getMax(pArr, pSize, &pMax, &pMaxIdx); // slave's task

        MPI_Send(&pMax, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send max value
        MPI_Send(&pMaxIdx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send max index

        free(pArr); // free allocated memory for partition
    }

    MPI_Finalize();
    return 0;
}
