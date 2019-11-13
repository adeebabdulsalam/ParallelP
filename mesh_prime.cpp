#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

void process_nodes_mesh(int n,int pid);
// int prime(int* arr, int size);
int main(int argc, char *argv[])
{
  int n,pid; //Number of processes or nodes
  MPI_Init(&argc, &argv);  //Initialize MPI
  MPI_Comm_size(MPI_COMM_WORLD, &n); //Get number of Process
  MPI_Comm_rank(MPI_COMM_WORLD, &pid); //Get each process ID

  //Main Code for Merge Sort
  process_nodes_mesh(n,pid);
  //
  MPI_Finalize();

  //cout<<"I am Process:"<<pid<<endl;

  return 0;
}

void prime(int arr[],int size,int* p)
{
  p[0]=0;
  for(int j=0;j<size;j++)
  {
    int m=arr[j]/2;
    int flag=0;
    for(int i = 2; i <= m; i++)
    {
     if(arr[j] % i == 0)
      {
        flag=1;
        break;
      }
    }
    if(flag==0){
        p[0]++;
    }
  }
//   cout<<"count:"<<p[0]<<endl;
//  return p;
}

void process_nodes_mesh(int n,int pid)
{
MPI_Status status;
int *b,*c,*d,*e;
int low,high,mid;
int size,chunk,size_in;
int size1,size2;
int* a;
int* a1;
int* a2;
int* a3;
int *p=new int[1];
int q,total=0;
int to_kid2,to_kid1; 
int piece_size,extra_piece;
int source,dest;
int row,col;
int root = sqrt(n);
row = pid/root;
col = pid%root;
if(row==0)
  {
   if(col==0)
   { 
   cout<<"Enter size of array"<<endl;
   cin>>size;
   cout<<"Enter array:";
   a = new int[size];
   for(int i=0;i<size;i++)
    {
     cin>>a[i];
    }
   if(size>=n)
    {
     int to_next,to_root; 
     piece_size = size/n; //size of array each node can carry
     extra_piece = size%n; //goes to last process
     chunk = root*piece_size+extra_piece;

     to_next=(root/2)*root*piece_size;
     dest=col+1;
     MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
     MPI_Send(&to_next,1,MPI_INT,dest,0,MPI_COMM_WORLD);
     MPI_Send(&a[chunk],to_next,MPI_INT,dest,0,MPI_COMM_WORLD);
      
     if(n>4){
       dest=root-1;
       to_root=((root/2)-1)*root*piece_size;
       MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
       MPI_Send(&to_root,1,MPI_INT,dest,0,MPI_COMM_WORLD);
       MPI_Send(&a[chunk+to_next],to_root,MPI_INT,dest,0,MPI_COMM_WORLD);
    }
    else{
      to_root=0;
    }
     //To kids
    low=0;
    high=piece_size+extra_piece-1;
    //distributing to kids below
    dest=1*root+col;
    to_kid1 = (root/2)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid1,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1],to_kid1,MPI_INT,dest,0,MPI_COMM_WORLD);

    if(n>4){
    dest=(root-1)*root+col;
    to_kid2 = (root/2-1)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);
    }
    else{
      to_kid2=0;
    }
    //parent distributed to their kids
    //parent nodes will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    // cout<<"here"<<endl;
    prime(a,(high+1),p);
    // cout<<"why?"<<endl;
    total+=*p;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    // cout<<"is it"<<endl;
    // cout<<"In process";
     
    //given to kids
    //take back from siblings
    source=col+1;
    //a1=new int[to_next];
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total +=q;  
    // cout<<"From:"<<source<<":"<<q;
    if(n>4){
      source=root-1;
      //a2=new int[to_root];
      MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
      total+=q;
    //   cout<<"From:"<<source<<":"<<q;
    }
    //parent recvd from siblings and merged
    source=1*root+col;

    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    // cout<<"From:"<<source<<":"<<q;

    if(n>4){
    source=(root-1)*root+col;
    
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    // cout<<"From:"<<source<<":"<<q;
    }
 
    cout<<"Total primes: "<<total<<endl;
    }
   else
   {
    //  cout<<"cheat";  
     prime(a,size,p);
     cout<<"Total prime: "<<*p;
    exit(0); 
   }
  }
  //j==root/2
  else if(col==root/2){
    source=col-1;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a = new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=piece_size*root;
    //parent node
    low=0;
    high=piece_size-1;
    //distributing to kids below
    dest=1*root+col;
    to_kid1 = chunk/2;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid1,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1],to_kid1,MPI_INT,dest,0,MPI_COMM_WORLD);

    dest=(root-1)*root+col;
    if(n>4){
    to_kid2 = (root/2-1)*piece_size; //same as (root/2-1)*piece_size
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);
    }
    else{
      to_kid2=0;
    }
    //all parents distributed to their kids
    //parent nodes will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    // cout<<p;
    total+=*p;
    

    source=1*root+col;

    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    source=(root-1)*root+col;
    if(n>4){
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    }
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    dest=col-1;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
  }
  else if(col==(root/2+1)){
    source=(col+1)%root;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=piece_size*root;
    //parent node
    low=0;
    high=piece_size-1;
    //distributing to kids below
    dest=1*root+col;
    to_kid1 = chunk/2;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid1,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1],to_kid1,MPI_INT,dest,0,MPI_COMM_WORLD);
   // if(n>4){
    dest=(root-1)*root+col;
    to_kid2 = (root/2-1)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);

    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    

    source=1*root+col;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    source=(root-1)*root+col;
    total+=q;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;

    dest=(col+1)%root;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
  }
  else if(col>0 && col<int(root/2)){
    source=col-1;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=root*piece_size;

    dest=col+1;
    int to_next=((root/2)-col)*root*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_next,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[chunk],to_next,MPI_INT,dest,0,MPI_COMM_WORLD);
 
    low=0;
    high=piece_size-1;
    //distributing to kids below
    dest=1*root+col;
    to_kid1 = chunk/2;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid1,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1],to_kid1,MPI_INT,dest,0,MPI_COMM_WORLD);

    dest=(root-1)*root+col;
    to_kid2 = (root/2-1)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);
    //all parents distributed to their kids
    //parent nodes will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    
    //to rcv from sibling
    source=col+1;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;

    source=1*root+col;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;

    source=(root-1)*root+col;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    dest=col-1;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
  }
  else if(root>4){
    source=(col+1)%root;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=root*piece_size;

    dest=col-1;
    int to_next=((root/2-1)-(root-col))*root*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_next,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[chunk],to_next,MPI_INT,dest,0,MPI_COMM_WORLD);
    //parent node
    low=0;
    high=piece_size-1;
    //distributing to kids below
    dest=1*root+col;
    to_kid1 = chunk/2;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid1,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1],to_kid1,MPI_INT,dest,0,MPI_COMM_WORLD);

    dest=(root-1)*root+col;
    to_kid2 = chunk-(high+1);
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);
    //all parents distributed to their kids
    //parent nodes will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    

    source=col-1;
    //a3=new int[to_next];
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    //to rcv from children
    source=1*root+col;
    //a1=new int[to_kid1];
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    source=(root-1)*root+col;
    //a2=new int[to_kid2];
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    dest=(col+1)%root;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
  }
 }
 //Column Phase over//
//Start Row Phase//
else if(row==root/2){
    source=(row-1)*root + col;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
 
    low=0;
    high=piece_size-1;

    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;

    dest=source;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
}
else if(row==(root/2+1)){
    source=((row+1)%root)*root + col;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
 
    low=0;
    high=piece_size-1;

    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;

    dest=source;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
}
else if(row>0 && row<(root/2)){
    source=(row-1)*root+col;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=piece_size;

    dest=(row+1)*root+col;
    int to_next=((root/2)-row)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_next,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[chunk],to_next,MPI_INT,dest,0,MPI_COMM_WORLD);
    //parent node
    low=0;
    high=piece_size-1;
    //current node will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    

    source=dest;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    total+=q;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    dest=(row-1)*root+col;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
}
else if(root>4){
    source=((row+1)%root)*root+col;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    chunk=piece_size;

    dest=(row-1)*root+col;
    int to_next=((root/2-1)-(root-row))*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_next,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[chunk],to_next,MPI_INT,dest,0,MPI_COMM_WORLD);
    //parent node
    low=0;
    high=piece_size-1;
    //current nodes will merge their share
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    prime(a,high+1,p);
    total+=*p;
    
 
    source=dest;
    MPI_Recv(&q,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);  
    total+=q;
    cout<<"In process["<<pid<<"] total:"<<total<<endl;
    dest=((row+1)%root)*root+col;
    MPI_Send(&total,1,MPI_INT,dest,0,MPI_COMM_WORLD);
}
}