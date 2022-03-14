/*
  "Hello World" MPI Test Program
*/
//https://en.wikipedia.org/wiki/Message_Passing_Interface#Example_program
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void printArray(int rank, int *array, int size){
    printf("rank: %d) ", rank);
    printf("[");
    for(int loop = 0; loop < size; loop++)
        printf("%d ", array[loop]);
    printf("]");
    printf("\n");
}



int main(int argc, char **argv)
{
    int my_rank, num_procs;
    /* Initialize the infrastructure necessary for communication */
    MPI_Init(&argc, &argv);

    /* Identify this process */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out how many total processes are active */
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* Until this point, all programs have been doing exactly the same.
       Here, we check the rank to distinguish the roles of the programs */

    int lamport_vec[num_procs];

    MPI_Request requests[num_procs];
    MPI_Status status[num_procs];
    int request_complete = 0;

    int flag = 0;

    //printf("You are in rank before: %i processes.\n", my_rank );
    if (my_rank == 0) {

        int num_received;
        int other_rank;
        int rsource, rdestination;
        printf("We have %i processes.\n", num_procs);

        memset( lamport_vec, 0, num_procs * sizeof(int) );
        lamport_vec[my_rank] = lamport_vec[my_rank] + 1; 

        for (other_rank = 0; other_rank < num_procs; other_rank++)
        {
            MPI_Isend(lamport_vec, num_procs, MPI_INT, other_rank, my_rank, MPI_COMM_WORLD, &requests[other_rank]);

            /* Receive message from any process */
            int ret = MPI_Irecv(lamport_vec, num_procs, MPI_INT, other_rank, 0, MPI_COMM_WORLD, &requests[other_rank]);

            if (ret == MPI_SUCCESS )
            {   
                MPI_Get_count(&status[other_rank], MPI_INT, &num_received);
                //MPI_Wait(&requests[other_rank], &status[other_rank]);         
                printArray(other_rank, lamport_vec, num_procs);
            }
        }

    } else {
        int num_received;
        /* Receive message from any process for telemetry */
        memset( lamport_vec, 0, num_procs * sizeof(int) );
        for (int other_rank = 0; other_rank < num_procs; other_rank++)
        { 
            /* Receive message from any process */

            int ret = MPI_Irecv(lamport_vec, num_procs, MPI_INT, other_rank, 0, MPI_COMM_WORLD, &requests[other_rank]);

            if (ret == MPI_SUCCESS )
            {
                MPI_Get_count(&status[other_rank], MPI_INT, &num_received);
                //MPI_Wait(&requests[other_rank], &status[other_rank]);
                lamport_vec[my_rank] = MAX(lamport_vec[my_rank], lamport_vec[other_rank]) + 1;             
                //printArray(other_rank, lamport_vec, num_procs);
                MPI_Isend(lamport_vec, num_procs, MPI_INT, other_rank, 0, MPI_COMM_WORLD, &requests[other_rank]);
            }
                          
        }
    }

    MPI_Waitall(num_procs, requests, status);

    /* Tear down the communication infrastructure */
    MPI_Finalize();
    return 0;
}


/**
MPI_Isend and MPI_Irecv MUST be followed at some point by MPI_Test and MPI_Wait. The process sending should never write in the buffer until the request has been completed. On the other hand, the process receiving should never read in the buffer before the request has been completed. And the only way to know if a request is completed, is to call MPI_Wait and MPI_Test.
*/
