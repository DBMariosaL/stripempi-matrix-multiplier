/*
    Name:   Lorenzo Mariosa
    N°:     3077735
*/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <mpi.h>

int world_size;
int world_rank;

//Get matrix from file
int *getMatrix(const char *file_name, int *&matrix, int size)
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

// Print the matrix
void printMatrix(int *&matrix, int size)
{
    int line = sqrt(size);

    for (size_t i = 0; i < size; i++)
    {
        std::cout << matrix[i] << "\t";
        if ((i + 1) % line == 0)
        {
            std::cout << std::endl;
        }
    }
}

//Row of matrix A * Column of matrix B
int dotProduct(int *rowA, int *colB, int resC, int size)
{

    for (size_t k = 0; k < size; k++)
    {
        resC += rowA[k] * colB[k];
    }
    return resC;
}

// Stripe (2 columns) of A * Matrix B
int *multiplyStripe(int **matrixB, int *stripe, int *resultC, int size, int rank)
{
    int loop_size = size * 2;

    // Stripe (column 1) of A * Matrix B
    for (int i = 0; i < size; i++)
    {
        resultC[i] = 0;
        for (int j = 0; j < size; j++)
        {
            resultC[i] += (*matrixB)[i * size + j] * stripe[j];
        }
    }
    // Stripe (column 2) of A * Matrix B
    for (int i = size; i < loop_size; i++)
    {
        resultC[i] = 0;
        for (int j = 0; j < size; j++)
        {
            resultC[i] += (*matrixB)[(i - size) * size + j] * stripe[j + size];
        }
    }
    return resultC;
}

int main(int argc, char const *argv[])
{
    // MPI initiation
    MPI_Init(NULL, NULL);

    //number of processes assignment
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //rank number assignment
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // node algorithm
    // initiation
    int size;
    int row_size;
    int stripe_size;
    int matrix_size;
    int node_rank;
    int *matrixA;
    int *matrixB;
    int message;
    int *result;
    int *revmatrixA;

    //matrix B memory allocation for all nodes (used in a Broadcast)
    matrixB = new int[matrix_size];

    //error handling
    if (world_rank == 0)
    {
        if (8 % atoi(argv[3]) != 0 || argc != 4)
        {
            message = 1;
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (message == 1)
    {
        std::cerr << "Unvalid Argument" << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
    }
    if (world_rank == 0)
    {

        try
        {
            //memory allocation for coordinator only
            size = atoi(argv[3]);
            row_size = atoi(argv[3]);
            stripe_size = size * (size / world_size);
            matrix_size = size * size;
            node_rank = world_rank * (size / world_size);
            message = 0;
            matrixA = new int[matrix_size];

            //getting matrix A and matrix B from files
            matrixA = getMatrix(argv[1], matrixA, matrix_size);
            matrixB = getMatrix(argv[2], matrixB, matrix_size);
            
            std::cout << "Lorenzo Mariosa 3077735" << std::endl;
            std::cout << "matA.dat matrix A:" << std::endl;
            printMatrix(matrixA, matrix_size);
            
            std::cout << "matB.dat matrix B:" << std::endl;
            printMatrix(matrixB, matrix_size);
            
            revmatrixA = new int[matrix_size];
            //revmatrixA is the tansposition of  matrix A (necessary to scatter the columns)
            for (size_t n = 0; n < matrix_size; n++)
            {
                int i = n / size;
                int j = n % size;
                revmatrixA[n] = matrixA[size * j + i];
            }
        }
        catch (const std::exception &e)
        {
            //error handling
            std::cerr << e.what() << '\n';
            message = 1;
        }
    }
    //error handling
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (message == 1)
    {
        std::cerr << "Unvalid Argument" << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
    }

    //Broadcasting all the usefull variables
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&row_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&stripe_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&node_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Declaring / Allocation memory for the stripe and it's result
    int *result_stripe = new int[stripe_size];
    int stripe[stripe_size];
    result = new int[matrix_size];

    //Broadcasting matrix B and scattering revmatrixA in stripe
    MPI_Bcast(matrixB, matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(revmatrixA, stripe_size, MPI_INT, stripe, stripe_size, MPI_INT, 0, MPI_COMM_WORLD);

    //performing the multiplication on matrix B
    result_stripe = multiplyStripe(&matrixB, stripe, result_stripe, size, node_rank);

    //gathering all the result_stripe in result
    MPI_Gather(result_stripe, stripe_size, MPI_INT, result, stripe_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        //Final print of matrix C
        std::cout << "Final result : Matrix C = B * A:" << std::endl;
        int *revmatrixB = new int[matrix_size];
       
        //revmatrixB is the tansposition of  result (necessary to reverse the columns and the rows)
        for (size_t n = 0; n < matrix_size; n++)
        {
            int i = n / size;
            int j = n % size;
            revmatrixB[n] = result[size * j + i];
        }
        printMatrix(revmatrixB, matrix_size);
        
        //deallocating all memory
        delete[] matrixA;
        delete[] revmatrixA;
    }
    
    //deallocating all memory
    delete[] result_stripe;
    delete[] matrixB;
    delete[] result;

    // MPI shutdown
    MPI_Finalize();
    return 0;
}