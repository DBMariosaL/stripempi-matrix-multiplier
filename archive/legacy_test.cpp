// Multiplication of a matrix by a vector – stripe horizontal partitioning
// (the source and the result vectors are doubled amoung the processors)
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <mpi.h>

int ProcRank; // Rank of current process
int ProcNum;  // Number of processes

int *getMatrix(const char *file_name, int *&matrix, int &size)
{
    FILE *input = freopen(file_name, "r", stdin);
    if (!input)
    {
        return NULL;
    }
    fread(matrix, sizeof(int), size, input);
    fclose(input);
    return matrix;
}

void ProcessInitialization(int *&pMatrix, int *&pMatrixB, int *&pVector, int *&pResult, int *&pProcRows, int *&pProcResult, int &Size, int &RowNum)
{
    int RestRows; // Number of rows, that haven’t been distributed yet
    int i;        // Loop variable
    if (ProcRank == 0)
    {
        Size = 8;
    }
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    RestRows = Size;
    for (i = 0; i < ProcRank; i++)
        RestRows = RestRows - RestRows / (ProcNum - i);
    RowNum = RestRows / (ProcNum - ProcRank);
    pVector = new int[Size];
    pResult = new int[Size];
    pProcRows = new int[RowNum * Size];
    pProcResult = new int[RowNum];
    if (ProcRank == 0)
    {
        pMatrix = new int[Size * Size];
        pMatrix = getMatrix("matA.dat", pMatrix, Size);
        pMatrixB = new int[Size * Size];
        pMatrixB = getMatrix("matB.dat", pMatrix, Size);
    }
}

void DataDistribution(int *pMatrix, int *pProcRows, int *pVector, int Size, int RowNum)
{
    int *pSendNum;                                        // the number of elements sent to the process
    int *pSendInd;                                        // the index of the first data element sent to the process
    int RestRows = Size;                                  // Number of rows, that haven’t been distributed yet
    MPI_Bcast(pVector, Size, MPI_INT, 0, MPI_COMM_WORLD); // Alloc memory for temporary objects
    pSendInd = new int[ProcNum];
    pSendNum = new int[ProcNum]; // Define the disposition of the matrix rows for current process
    RowNum = (Size / ProcNum);
    pSendNum[0] = RowNum * Size;
    pSendInd[0] = 0;
    for (int i = 1; i < ProcNum; i++)
    {
        RestRows -= RowNum;
        RowNum = RestRows / (ProcNum - i);
        pSendNum[i] = RowNum * Size;
        pSendInd[i] = pSendInd[i - 1] + pSendNum[i - 1];
    } // Scatter the rows
    MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_INT, pProcRows, pSendNum[ProcRank], MPI_INT, 0, MPI_COMM_WORLD);
    // Free the memory
    delete[] pSendNum;
    delete[] pSendInd;
}

void ParallelResultCalculation(int *pProcRows, int *pVector, int *pProcResult, int Size, int RowNum)
{
    int i, j; // Loop variables
    for (i = 0; i < RowNum; i++)
    {
        pProcResult[i] = 0;
        for (j = 0; j < Size; j++)
            pProcResult[i] += pProcRows[i * Size + j] * pVector[j];
    }
}

void ResultReplication(int *pProcResult, int *pResult, int Size, int RowNum)
{
    int i;               // Loop variable
    int *pReceiveNum;    // Number of elements, that current process sends
    int *pReceiveInd;    /* Index of the first element from current process                          in result vector */
    int RestRows = Size; // Number of rows, that haven’t been distributed yet
    //Alloc memory for temporary objects
    pReceiveNum = new int[ProcNum];
    pReceiveInd = new int[ProcNum];
    //Define the disposition of the result vector block of current processor
    pReceiveInd[0] = 0;
    pReceiveNum[0] = Size / ProcNum;
    for (i = 1; i < ProcNum; i++)
    {
        RestRows -= pReceiveNum[i - 1];
        pReceiveNum[i] = RestRows / (ProcNum - i);
        pReceiveInd[i] = pReceiveInd[i - 1] + pReceiveNum[i - 1];
    } //Gather the whole result vector on every processor
    MPI_Allgatherv(pProcResult, pReceiveNum[ProcRank], MPI_INT, pResult, pReceiveNum, pReceiveInd, MPI_INT, MPI_COMM_WORLD);
    //Free the memory
    delete[] pReceiveNum;
    delete[] pReceiveInd;
}

int main(int argc, char *argv[])
{
    int *pMatrix; // The first argument - initial matrix
    int *pMatrixB;
    int *pVector; // The second argument - initial vector
    int *pResult; // Result vector for matrix-vector multiplication
    int Size;     // Sizes of initial matrix and vector
    int *pProcRows;
    int *pProcResult;
    int RowNum;
    int Start, Finish, Duration;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    ProcessInitialization(pMatrix, pMatrixB, pVector, pResult, pProcRows, pProcResult, Size, RowNum);
    DataDistribution(pMatrix, pProcRows, pVector, Size, RowNum);
    ParallelResultCalculation(pProcRows, pVector, pProcResult, Size, RowNum);
    ResultReplication(pProcResult, pResult, Size, RowNum);
    if (ProcRank == 0)
    {
        for (size_t i = 0; i < 8; i++)
        {
            std::cout << pResult[i] << " ";
        }
        std::cout << std::endl;
    }

    //ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);
    MPI_Finalize();
    return 0;
}