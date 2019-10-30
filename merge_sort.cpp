#include <mpi.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

void process_nodes(int n,int pid);

int main(int argc, char *argv[])
{
  int n,pid; //Number of processes or nodes
  MPI_Init(&argc, &argv);  //Initialize MPI
  MPI_Comm_size(MPI_COMM_WORLD, &n); //Get number of Process
  MPI_Comm_rank(MPI_COMM_WORLD, &pid); //Get each process ID

  //Main Code for Merge Sort
  process_nodes(n,pid);
  //
  MPI_Finalize();

  //cout<<"I am Process:"<<pid<<endl;

  return 0;
}

void merge(int *a,int low,int mid,int high)
{
 int i,j,k;
 int n1=mid-low+1;
 int n2=high-mid;
 int L[n1],R[n2];

 for(i=0;i<n1;i++)
   L[i]=a[low+i];
 for(j=0;j<n2;j++)
   R[j]=a[mid+1+j];

 i=j=0;
 k=low;
 while(i<n1 && j<n2)
 {
  if(L[i]<=R[j])
  {
   a[k]=L[i];
   i++;
  }
  else
  {
   a[k]=R[j];
   j++;
  }
  k++;
 }

 while(i<n1)
  a[k++]=L[i++];

 while(j<n2)
  a[k++]=R[j++];  
}

void merge_sort(int *a,int low,int high)
{
 int mid;
 if(low<high)
  {
    mid=(low+high)/2;
    merge_sort(a,low,mid);
    merge_sort(a,mid+1,high);
    merge(a,low,mid,high);
  }
}

void process_nodes(int n,int pid)
{
MPI_Status status;
int a[100],a1[100];
int *b = (int *)malloc(2*n*sizeof(int));
int low,high,mid,low1,high1;
int size;
int source,dest;
 if(pid==0)
  {
   int piece_size,extra_piece;
   cout<<"Enter size of array"<<endl;
   cin>>size;
   cout<<"Enter array:";
   for(int i=0;i<size;i++)
    {
     cin>>a[i];
    }
   if(size>n)
    {
     piece_size = size/n; //size of array each node can carry
     extra_piece = size%n; //goes to last process
     for(int i=0;i<n;i++)
      {
	      *(b+i)=i*piece_size;
	      *(b+n+i)=*(b+i)+piece_size-1; 
      }
     *(b+2*n-1) += extra_piece;

      dest=1;
      MPI_Send(b,2*n,MPI_INT,dest,0,MPI_COMM_WORLD);
      MPI_Send(&size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
      MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
      
      if(n>2)
       {
        dest=n-1;
        MPI_Send(b,2*n,MPI_INT,dest,0,MPI_COMM_WORLD);
        MPI_Send(&size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
        MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
       }

      low=*(b+pid);	
      high=*(b+n+pid);
      merge_sort(a,low,high);

      MPI_Recv(a1,size,MPI_INT,1,0,MPI_COMM_WORLD,&status);
      mid=high;
      high=*(b+n+n/2);
      for(int i=mid+1;i<=high;i++){
        a[i]=a1[i];
      }

      merge(a,low,mid,high);

      MPI_Recv(a1,size,MPI_INT,n-1,0,MPI_COMM_WORLD,&status);
      mid=high;
      high=*(b+2*n-1);
      for(int i=mid+1;i<=high;i++){
        a[i]=a1[i];
      }
      merge(a,low,mid,high);
    
      cout<<"Sorted Array:";
      for(int i=low;i<=high;i++){
        cout<<a[i]<<" ";
      }
      cout<<endl;
    }
   else
   {
    merge_sort(a,0,size-1);
    for(int i=0;i<size;i++)
     cout<<a[i]<<" "<<endl;
    exit(0); 
   }
  }

 else if(pid==n/2)
  {
    source=(n/2)-1;

    MPI_Recv(b,2*n,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(a,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);
 
    low=*(b+pid);
    high=*(b+n+pid);
    merge_sort(a,low,high);
   
    dest=source;
    MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
  }

 else if(pid == (n/2)+1)
  {
   source=((n/2) + 2)%n;
   MPI_Recv(b,2*n,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   MPI_Recv(&size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   MPI_Recv(a,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   low=*(b+pid);
   high=*(b+n+pid);
   merge_sort(a,low,high);

   dest=source;
   MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
  }

 else if(pid>0 && pid<n/2)
  {
    source = pid-1;
    MPI_Recv(b,2*n,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(a,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);     

   dest=pid+1;
   MPI_Send(b,2*n,MPI_INT,dest,0,MPI_COMM_WORLD);
   MPI_Send(&size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
   MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);

   low=*(b+pid);
   high=*(b+n+pid);
   merge_sort(a,low,high);

   int temp=source;
   source=dest;
   dest=temp;

   MPI_Recv(a1,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   mid=high;
   high=*(b+n+n/2);
   for(int i=mid+1;i<=high;i++){
     a[i]=a1[i];
   }
   merge(a,low,mid,high);

   MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
  }

 else{
 if(pid>(n/2 +1) && pid<=(n-1))
  {
   source=(pid+1)%n;
   MPI_Recv(b,2*n,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   MPI_Recv(&size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   MPI_Recv(a,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);

   dest=pid-1;
   MPI_Send(b,2*n,MPI_INT,dest,0,MPI_COMM_WORLD);
   MPI_Send(&size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
   MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);

   low=*(b+pid);
   high=*(b+n+pid);
   merge_sort(a,low,high);

   int temp=source;
   source=dest;
   dest=temp;

   MPI_Recv(a1,size,MPI_INT,source,0,MPI_COMM_WORLD,&status);
   mid=*(b+n+source);
   low=*(b+n/2+1);
   for(int i=low;i<=mid;i++){
     a[i]=a1[i];
   }
   merge(a,low,mid,high);
   
   MPI_Send(a,size,MPI_INT,dest,0,MPI_COMM_WORLD);
  }
 }
}
