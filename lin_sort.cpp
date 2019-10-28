# include <cstdlib>
# include <iomanip>
# include <iostream>
# include <mpi.h>
# include <string.h>

using namespace std;

int main ( int argc, char *argv[] );
void ring_io ( int p, int id );
void sort(int id, int* arr, int num, int size); 
void Merge(int *a, int low, int high, int mid);
void MergeSort(int *a, int low, int high);

int main ( int argc, char *argv[] )
{

  int error;
  int id;
  int p;

  if ( id == 0 )
  {
    cout << "\nprocessor no: "<<id;
  }

  //int size=num/p;
  //cout<<"size="<<size;


//
//  Initialize MPI.
//
  MPI_Init ( &argc, &argv );
//
//  Get the number of processes.
//
  MPI_Comm_size ( MPI_COMM_WORLD, &p );
//
//  Get the individual process ID.
//
  MPI_Comm_rank ( MPI_COMM_WORLD, &id );
  
  ring_io ( p, id );
//
//  Shut down MPI.
//
  MPI_Finalize ( );
//
//  Terminate.
//
  if ( id == 0 )
  {
    cout << "\nback to processor: "<<id;
  }
  return 0;
}


void ring_io ( int p, int id )

{
  int dest;
  int i;
  int j;
  int n=5;
  int test[5]={1,2,3,4,5};
  int n_test_num = 5;
  int source;
  MPI_Status status;
  string s;
  int num=16;
  int size=4;

  cout<<"maddalam"<<endl;
  int *rec;
//
//  Process 0 sends very first message, 
//  then waits to receive the "echo" that has gone around the world.
//
  if ( id == 0 )
  {

      
      int i=0;



      cout<<"Enter number of elements for the array"<<endl;
      cin>>*(&num);
      int* arr= new int[num];
      rec= new int[num];
      int* res= new int[num];

      for(i=0;i<num;i++)
      {
        cout<<"Enter "<<i<<"th value: "<<endl;
        cin>>arr[i];
        res[i]=arr[i];
        
      }

      size=num/p;
      cout<<"size="<<size;
      
      
      
      dest = 1;
      MPI_Send ( arr, num, MPI_INT, dest,   0, MPI_COMM_WORLD );

      if(p>2){
      dest = p-1;
      MPI_Send ( arr, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
      }
      
      
      source = 1;
      MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
      

      for(i=0;i<=(size-1)+(num/2);i++)
        res[i]=rec[i];
      //for(i=0;i<num;i++)
        //cout<<rec[i]<<"\nyeah\n";

      if(p>2){
      source = p - 1;
      MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );

      for(i=num-1;i>(size-1)+(num/2);i--)
        res[i]=rec[i];
      //for(i=0;i<num;i++)
        //cout<<rec[i]<<"\nyes\n";
      }     


      sort(id,res,num,size);
      cout<<"hi"<<num<<endl;
      for(i=0;i<num;i++)
        cout<<res[i];

      for(i=0;i<(num/size)-1;i++)
      {
         Merge(res,0,(i+2)*size,(((i+2)*size)/2)-1);
      }

      cout<<"final result:\n";
      for(i=0;i<num;i++)
        cout<<res[i];

    }

//
//  Worker ID must receive first from ID-1, then send to ID+1.
//
    else
    {
      if(id<=p/2)
      {
        
        rec= new int[num];
        if(id==p/2)
        {
         source=id-1;
         dest=id-1; 
         MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
         cout<<"size= "<<size;
         sort(id,rec,num,size);
         MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD ); 
        }
        else
        {
        source=id-1; 
        MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
        cout<<"size= "<<size;
        sort(id,rec,num,size);
        dest=id+1;
        MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
        //cout<<"hello from processor :"<<id<<" num="<<num<<" msg:"<<endl;
        //for(i=0;i<num;i++)
        //cout<<rec[i];

        source= id+1;
        MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
        cout<<"size= "<<size;
        dest=id-1;
        MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
        }
      }
      else
      {
         rec= new int[num];
         cout<<"num+"<<num<<"size+"<<size;
        if(id==p/2 +1)
        {
          source= (id+1)%p;
          dest=(id+1)%p;
          MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
          cout<<"size= "<<size;
          sort(id,rec,num,size);
          MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
          
        }
        else
        {
          source=(id+1)%p;
          dest=id-1;
          MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
          cout<<"size= "<<size;
          sort(id,rec,num,size);
          MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
          //cout<<"hello from processor :"<<id<<" num="<<num<<" msg:"<<endl;
          //for(i=0;i<num;i++)
          //cout<<rec[i];

          source= id-1;
          dest=(id+1)%p;
          MPI_Recv ( rec, num, MPI_INT, source, 0, MPI_COMM_WORLD, &status );
          cout<<"size= "<<size;
          MPI_Send ( rec, num, MPI_INT, dest,   0, MPI_COMM_WORLD );
        }
        }
      }
    

  return;
}

void sort(int id, int* arr, int num, int size)
{
  int* temp=new int[size];
  for(int i=0;i<size;i++)
  {
    //cout<<"processor: "<<id<<" partn: ";
    //cout<<arr[size*id+i];
    temp[i]=arr[size*id+i];
    
  }

  MergeSort(temp,0,size-1);
  for(int i=0;i<size;i++)
  {
    arr[size*id+i]=temp[i];
    //cout<<"\n\narr+"<<size*id+i<<"="<<arr[size*id+i]<<endl;
    //cout<<"temp+"<<i<<"="<<temp[i]<<endl;
  }
  
  cout<<"\n\n";
}


void Merge(int *a, int low, int high, int mid)
{
	// We have low to mid and mid+1 to high already sorted.
	int i, j, k, temp[high-low+1];
	i = low;
	k = 0;
	j = mid + 1;

	// Merge the two parts into temp[].
	while (i <= mid && j <= high)
	{
		if (a[i] < a[j])
		{
			temp[k] = a[i];
			k++;
			i++;
		}
		else
		{
			temp[k] = a[j];
			k++;
			j++;
		}
	}

	// Insert all the remaining values from i to mid into temp[].
	while (i <= mid)
	{
		temp[k] = a[i];
		k++;
		i++;
	}

	// Insert all the remaining values from j to high into temp[].
	while (j <= high)
	{
		temp[k] = a[j];
		k++;
		j++;
	}


	// Assign sorted data stored in temp[] to a[].
	for (i = low; i <= high; i++)
	{
		a[i] = temp[i-low];
	}
}

// A function to split array into two parts.
void MergeSort(int *a, int low, int high)
{
	int mid;
	if (low < high)
	{
		mid=(low+high)/2;
		// Split the data into two half.
		MergeSort(a, low, mid);
		MergeSort(a, mid+1, high);

		// Merge them to get sorted output.
		Merge(a, low, high, mid);
	}
}
   
