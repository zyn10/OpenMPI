#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>

int roundOff(double num)
{
	double i;
	double fraction = modf(num,&i);
		if(fraction<0.5)
			return num;
		else
			return num+0.5;
}

int *countBins(int*recvBuff,int recvCount,int *bins)
{
	for(int i=0;i<recvCount;i++)
	{
		if(recvBuff[i] >=0 && recvBuff[i] <=33)
		{
			bins[0]+=1;
		}
		else if(recvBuff[i] >=34 && recvBuff[i] <=50)
		{
			bins[1]+=1;
		}
		else if(recvBuff[i] >=51 && recvBuff[i] <=75)
		{
			bins[2]+=1;
		}
		else if(recvBuff[i] >=76 && recvBuff[i] <=100)
		{
			bins[3]+=1;
		}
	}
	return bins;
}

int main(int argc,char *argv[])
{
	int np,rank;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&np);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	int recvCount,size;
	int *recvBuff=NULL,*sendBuff=NULL,*sendCount=NULL,*displs=NULL,*sendBins,*recvBins;
	
	sendBins = (int*)malloc(4*sizeof(int));
	recvBins = (int*)malloc(4*sizeof(int));
	for(int i=0;i<4;i++)
		sendBins[i] = recvBins[i] = 0;
	
	sendCount = (int*)malloc(np*sizeof(int));
	size = atoi(argv[1]);
	
	int tempRank = np;
	int tempSize = size;
	if(tempSize%tempRank!=0)
	{
		for(int i=tempRank;i>=1;i--)
		{			
			if(tempSize%i!=0)
			{	
				double num = (double)tempSize/i;
				int range = roundOff(num);
				tempSize -= range;
				sendCount[i-1] = range;	
			}
			else
			{
				sendCount[i-1] = tempSize/i;
			}
		}
	}
	else
	{
		for(int i=0;i<np;i++)
		{
			sendCount[i] = size/np;
		}
	}

	if(rank == 0)
	{

		srand(time(0));
		sendBuff = (int*)malloc(size*sizeof(int));
		for(int i=0;i<size;i++)
			sendBuff[i]=rand()%101;
		
		printf("SEND BUFFER VALUES = ");
		for(int i=0;i<size;i++)
			printf("%d, ", sendBuff[i]);
		printf("\n");
		

		displs = (int*)malloc(np*sizeof(int));
		displs[0] = 0;
		for(int i=0;i<np;i++)
			displs[i] = displs[i-1] + sendCount[i-1];
	}
	
	
	MPI_Barrier(MPI_COMM_WORLD);
	recvCount = sendCount[rank];
	recvBuff = (int*)malloc(recvCount*sizeof(int));
	MPI_Scatterv(sendBuff,sendCount,displs,MPI_INT,recvBuff,recvCount,MPI_INT,0,MPI_COMM_WORLD);
	

	sendBins = countBins(recvBuff,recvCount,sendBins);

	MPI_Reduce(sendBins,recvBins,4,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	if(rank==0)
	{
		printf("THE FINAL DISTRIBUTION AT P0\n");
		for(int i=0;i<4;i++)
		{
			if(i==0)
			{
				printf("BIN 1 (0-33): %d, \n",recvBins[i]);
			}
			else if(i==1)
			{
				printf("BIN 2 (34-50): %d, \n",recvBins[i]);			
			}
			else if(i==2)
			{
				printf("BIN 3 (51-75): %d, \n",recvBins[i]);
			}
			else
			{
				printf("BIN 4 (76-100): %d, \n",recvBins[i]);
			}
		}		
	}

	
	MPI_Finalize();
}

