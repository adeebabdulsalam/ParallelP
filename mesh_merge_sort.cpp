#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

void process_nodes_mesh(int n,int pid);

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
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;  
    //given to kids
    //take back from siblings
    source=col+1;
    a1=new int[to_next];
    MPI_Recv(a1,to_next,MPI_INT,source,0,MPI_COMM_WORLD,&status);
 
    if(n>4){
      source=root-1;
      a2=new int[to_root];
      MPI_Recv(a2,to_root,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    }

    b=new int[to_root+to_next];
    copy(a1,a1+to_next,b);
    if(n>4)
      copy(a2,a2+to_root,b+to_next);

    merge(b,0,to_next-1,(to_root+to_next-1));
    //parent recvd from siblings and merged
    source=1*root+col;
    a1 = new int[to_kid1];
    MPI_Recv(a1,to_kid1,MPI_INT,source,0,MPI_COMM_WORLD,&status);

    if(n>4){
    source=(root-1)*root+col;
    a2=new int[to_kid2];
    MPI_Recv(a2,to_kid2,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    }
    //parent rcvd from kids and merged
    c=new int[to_kid2+to_kid1];

    copy(a1,a1+to_kid1,c);
    if(n>4)
      copy(a2,a2+to_kid2,c+to_kid1);
    merge(c,0,to_kid1-1,(to_kid2+to_kid1-1));
    //kids merged
    d=new int[high+1+to_kid2+to_kid1];

    copy(a,a+high+1,d);
    copy(c,c+to_kid1+to_kid2,d+high+1);
    merge(d,0,high,(to_kid1+to_kid2+high));
    //parent and kids merged
    e=new int[to_kid1+to_kid2+high+1+to_root+to_next];

    size1=to_kid1+to_kid2+high+1;
    size2=to_next+to_root;
    copy(d,d+size1,e);
    copy(b,b+size2,e+size1);
    merge(e,0,(to_kid1+to_kid2+high),(to_next+to_root+to_kid1+to_kid2+high));
    //everyone merged
    cout<<"Final Merge sorted array:";
    for(int i=0;i<size;i++){
      cout<<e[i]<<" ";
    }
    cout<<endl;
    }
   else
   {
    merge_sort(a,0,size-1);
    cout<<"Final Sorted array:";
    for(int i=0;i<size;i++)
     cout<<a[i]<<" ";
    cout<<endl; 
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
    merge_sort(a,low,high);

    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    source=1*root+col;
    a1 = new int[to_kid1];
    MPI_Recv(a1,to_kid1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    source=(root-1)*root+col;
    if(n>4){
    a2=new int[to_kid2];
    MPI_Recv(a2,to_kid2,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    }
    b=new int[to_kid1+to_kid2];

    size1=to_kid1;
    size2=to_kid2;
    copy(a1,a1+size1,b);
    if(n>4)
      copy(a2,a2+size2,b+size1);
    merge(b,0,to_kid1-1,(to_kid1+to_kid2-1));

    c=new int[to_kid1+to_kid2+high+1];
    
    size1=high+1;
    size2=to_kid1+to_kid2;
    copy(a,a+size1,c);
    copy(b,b+size2,c+size1);
    merge(c,0,high,(to_kid1+to_kid2+high));

    dest=col-1;
    MPI_Send(c,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
  
    dest=(root-1)*root+col;
    to_kid2 = (root/2-1)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);

    //all parents distributed to their kids
    //parent nodes will merge their share
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    source=1*root+col;
    a1=new int[to_kid1];
    MPI_Recv(a1,to_kid1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    source=(root-1)*root+col;
    a2=new int[to_kid2];
    
    MPI_Recv(a2,to_kid2,MPI_INT,source,0,MPI_COMM_WORLD,&status);

    b=new int[to_kid1+to_kid2];
   
    size1=to_kid1;
    size2=to_kid2;
    copy(a1,a1+size1,b);
    copy(a2,a2+size2,b+size1);
    merge(b,0,to_kid1-1,(to_kid1+to_kid2-1));

    c=new int[to_kid1+to_kid2+high+1];
    
    size1=high+1;
    size2=to_kid1+to_kid2;
    copy(a,a+size1,c);
    copy(b,b+size2,c+size1);
    merge(c,0,high,(to_kid1+to_kid2+high));

    dest=(col+1)%root;
    MPI_Send(c,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
    to_kid2 = (root/2-1)*piece_size;
    MPI_Send(&piece_size,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&to_kid2,1,MPI_INT,dest,0,MPI_COMM_WORLD);
    MPI_Send(&a[high+1+to_kid1],to_kid2,MPI_INT,dest,0,MPI_COMM_WORLD);
    //all parents distributed to their kids
    //parent nodes will merge their share
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;
    //to rcv from sibling
    source=col+1;
    a3=new int[to_next];
    MPI_Recv(a3,to_next,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    //to rcv from children
    source=1*root+col;
    a1=new int[to_kid1];
    MPI_Recv(a1,to_kid1,MPI_INT,source,0,MPI_COMM_WORLD,&status); 
    source=(root-1)*root+col;
    a2=new int[to_kid2];
    MPI_Recv(a2,to_kid2,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    
    
    b=new int[to_kid1+to_kid2];

    size1=to_kid1;
    size2=to_kid2;
    copy(a1,a1+size1,b);
    copy(a2,a2+size2,b+size1);

    merge(b,0,to_kid1-1,(to_kid1+to_kid2-1));

    c=new int[high+1+to_kid1+to_kid2];

    size1=high+1;
    size2=to_kid1+to_kid2;
    copy(a,a+size1,c);
    copy(b,b+size2,c+size1);
    merge(c,0,high,(to_kid1+to_kid2+high));
    //parent and children merged
    d=new int[to_kid1+to_kid2+high+1+to_next];

    size1=to_kid1+to_kid2+high+1;
    size2=to_next;
    copy(c,c+size1,d);
    copy(a3,a3+size2,d+size1);
    merge(d,0,(to_kid1+to_kid2+high),(to_kid1+to_kid2+to_next+high));

    dest=col-1;
    MPI_Send(d,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;
    //to rcv from sibling
    source=col-1;
    a3=new int[to_next];
    MPI_Recv(a3,to_next,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    //to rcv from children
    source=1*root+col;
    a1=new int[to_kid1];
    MPI_Recv(a1,to_kid1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    source=(root-1)*root+col;
    a2=new int[to_kid2];
    MPI_Recv(a2,to_kid2,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    b=new int[to_kid1+to_kid2];
    
    size1=to_kid1;
    size2=to_kid2;
    copy(a1,a1+size1,b);
    copy(a2,a2+size2,b+size1);
    merge(b,0,to_kid1-1,(to_kid1+to_kid2-1));
    c=new int[high+1+to_kid1+to_kid2];
    
    size1=high+1;
    size2=to_kid1+to_kid2;
    copy(a,a+size1,c);
    copy(b,b+size2,c+size1);
    merge(c,0,high,(to_kid1+to_kid2+high));
    //parent and children merged
    d=new int[to_kid1+to_kid2+high+1+to_next];
   
    size1=to_kid1+to_kid2+high+1;
    size2=to_next;
    copy(c,c+size1,d);
    copy(a3,a3+size2,d+size1);
    merge(d,0,(to_kid1+to_kid2+high),(to_kid1+to_kid2+to_next+high));

    dest=(col+1)%root;
    MPI_Send(d,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
    merge_sort(a,low,high);

    cout<<"In node:["<<pid<<"]: ";
    for(int i=0;i<size_in;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    dest=source;
    MPI_Send(a,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
}
else if(row==(root/2+1)){
    source=((row+1)%root)*root + col;
    MPI_Recv(&piece_size,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&size_in,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    a=new int[size_in];
    MPI_Recv(a,size_in,MPI_INT,source,0,MPI_COMM_WORLD,&status);
 
    low=0;
    high=piece_size-1;
    merge_sort(a,low,high);
    cout<<"In node:["<<pid<<"]: ";
    for(int i=0;i<size_in;i++)
      cout<<a[i]<<" ";
    cout<<endl;

    dest=source;
    MPI_Send(a,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;
    //to rcv from sibling
    source=dest;
    a1=new int[to_next];
    MPI_Recv(a1,to_next,MPI_INT,source,0,MPI_COMM_WORLD,&status);
    b=new int[high+1+to_next];
    
    size1=high+1;
    size2=to_next;
    copy(a,a+size1,b);
    copy(a1,a1+size2,b+size1);
 
    merge(b,0,high,(to_next+high));

    dest=(row-1)*root+col;
    MPI_Send(b,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
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
    merge_sort(a,low,high);
    cout<<"In node ["<<pid<<"]: ";
    for(int i=low;i<=high;i++)
      cout<<a[i]<<" ";
    cout<<endl;
    //to rcv from sibling
    source=dest;
    a1=new int[to_next];
    MPI_Recv(a1,to_next,MPI_INT,source,0,MPI_COMM_WORLD,&status);  

    b=new int[high+1+to_next];
    
    size1=high+1;
    size2=to_next;
    copy(a,a+size1,b);
    copy(a1,a1+size2,b+size1);
    merge(b,0,high,(high+to_next));

    dest=((row+1)%root)*root+col;
    MPI_Send(b,size_in,MPI_INT,dest,0,MPI_COMM_WORLD);
}
}