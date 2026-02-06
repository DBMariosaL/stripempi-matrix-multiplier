// BSC4-DS Assignment 03 - Matrix Helper File
// Use this file to generate two matrices (A and B) to be multiplied
// in parallel in your assignment03

#include <stdio.h>
#include <iostream>
#include <cstdlib>
using namespace std;

void printMatrix()
{
    int matrix[64];
    FILE *input = freopen("matA.dat", "r", stdin);
    if (!input)
    {
        return;
    }
    size_t mat = fread(matrix, sizeof(int), 64, input);
    fclose(input);
    for (size_t i = 0; i < 64; i++)
    {
        std::string s = std::to_string(matrix[i]);
        char const *pchar = s.c_str();
        std::cout << pchar << std::endl;
    }
}

// function that generates a matrix in binary form on disk
void generateMatrixFile()
{
    // generate a matrix of values that need to be written to disk in the form of a one dimensional array this will write out an 8x8 matrix
    int matrix[64];

    srand(time(NULL));

    for (int i = 0; i < 64; i++)
    {
        matrix[i] = rand() % 10;
    }
    for (size_t i = 0; i < 64; i++)
    {
        std::string s = std::to_string(matrix[i]);
        char const *pchar = s.c_str();
        std::cout << pchar << std::endl;
    }

    // open up a file for writing
    // TODO: You may want to change the name of the matrix for A and B (eg "matA.dat", "matB.dat")
    FILE *output = freopen("matB.dat", "wb", stdout);

    if (!output)
    {
        return;
    }

    // do a simple fwrite to write the matrix to file
    fwrite(matrix, sizeof(int), 64, output);

    // close the file when we are finished writing
    fclose(output);

    printMatrix();
}

// the main function of the program
int main(int argc, char **argv)
{
    generateMatrixFile();
}
