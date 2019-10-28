#include <mpi.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
  int n,pid; //Number of processes or nodes
  MPI_Init(&argc, &argv);  //Initialize MPI
  MPI_Comm_size(MPI_COMM_WORLD, &n); //Get number of Process
  MPI_Comm_rank(MPI_COMM_WORLD, &pid); //Get each process ID

  //Main Code for Merge Sort
 
  //
  MPI_Finalize();
  if(pid == 0){
   cout<< "I am the host process:"<<pid<<endl;
  }
  else{
   cout<<"I am Process:"<<pid<<endl;
  }
  return 0;
}
