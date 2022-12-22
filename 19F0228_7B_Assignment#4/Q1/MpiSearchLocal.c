#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

void findNumber(int*recvBuff,int recvCount,int key_value,int rank)
{
	int check = 0;
	for(int i=0;i<recvCount;i++)
	{
		if(recvBuff[i]==key_value)
		{
			printf("Process: %d found its occurrence at index: %d\n",rank,i);
			check = 1;
		}
	}
	if(!check)
		printf("No Key Value Found at Process:%d\n",rank);
	
}

int main(int argv, char *argc[])
{
	int np,rank;
	MPI_Init(&argv,&argc);
	
	MPI_Comm_size(MPI_COMM_WORLD,&np);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	
	int size = atoi(argc[1]);
	int *recvBuff = NULL,*sendCount = NULL,*displs = NULL,*localarr = NULL;
	int recvCount = size/np;
	if(size%np!=0 && rank==np-1)
	{
		recvCount+=size%np;
	}
	recvBuff = (int*)malloc(recvCount *sizeof(int));

	if(rank==0)
	{
		
		FILE *file;
		if(size>100)
			size = 100;
		localarr = (int*)malloc(size*sizeof(int));
		if((file = fopen(argc[2],"r"))!=NULL)
		{
			int num=0;
			for(int i=0;i<size;i++)
			{
				fscanf(file,"%d",&num);
				localarr[i] = num;		
			}
				
			sendCount = (int*)malloc(np *sizeof(int));
			displs = (int*)malloc(np *sizeof(int));
			for(int i=0;i<np;i++)
			{
				if(size%np!=0&&i==np-1)
				{
					sendCount[i] = size/np;
					sendCount[i] += size%np; 
				}
				else
				{
					sendCount[i] = size/np;
				}
			}
			displs[0]=0;
			for(int i=1;i<np;i++)
			{
				displs[i] = displs[i-1] + sendCount[i-1];

			}

			printf("INITIAL VALUES = ");
			for(int i=0;i<size;i++)
				printf("%d, ",localarr[i]);
		}
		else
		{
			printf("File Not Found!!!\n");
		}
	}

	MPI_Scatterv(localarr,sendCount,displs,MPI_INT,recvBuff,recvCount,MPI_INT,0,MPI_COMM_WORLD);			
	MPI_Barrier(MPI_COMM_WORLD);
	printf("\nI AM RANK = %d WITH VALUES = ",rank);
	for(int i =0;i<recvCount;i++)
	{
		printf("%d, ",recvBuff[i]);
	}
	printf("\n");
	findNumber(recvBuff,recvCount,atoi(argc[3]),rank);

	MPI_Finalize();
}
