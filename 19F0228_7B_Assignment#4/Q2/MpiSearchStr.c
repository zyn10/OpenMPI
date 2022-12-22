#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct patternStorage{
	int count;
	int *arr;
}patternStorage;

patternStorage findPattern(char*str,char*pattern,int sindex,int eindex,int patternlength)
{
	patternStorage obj;
	obj.arr = (int*)malloc(((sindex+eindex)/patternlength+1) * sizeof(int));
	int k=0;
	obj.count = 0;
	for (int i = sindex; i <= eindex; i++) 
	{

		int j;
		for (j = 0; j < patternlength; j++)
		    if (str[i + j] != pattern[j])
		        break;
	 
		if (j == patternlength) // if pat[0...M-1] = txt[i, i+1, ...i+M-1]
		{
			obj.arr[k] = i;
			obj.count += 1;
			k++;
		} 
    	}
		
    return obj;
}
int main(int argc,char *argv[])
{
	int np,rank;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&np);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	FILE *file;
	if((file = fopen("seq.txt","r"))!=NULL)
	{
		char str2[]="GC";
		char str1[1000];

		while(fscanf(file,"%s",str1)!=EOF);
		
		int wordlength = strlen(str1);
		int patternlength = strlen(str2);
		int sindex=0,eindex=0;
		int *recvBuff1,*recvBuff2;
		int globalcount = 0;
		int *displs;
		if(rank==np-1)//LAST PROCESS WILL WORK FOR MORE CHARACTERS THEN THE REST
		{
			sindex = rank*(wordlength/np);
			eindex = wordlength;
		}
		else
		{			
			sindex = rank*(wordlength/np);
			eindex = ((rank+1)*(wordlength/np)) - 1;
		}
		patternStorage obj = findPattern(str1,str2,sindex,eindex,patternlength);
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==0)
		{
			printf("SEQUENCE: %s\nPATTERN: %s\n",str1,str2);
			recvBuff2 = (int*)malloc(np*sizeof(int));
			MPI_Gather(&obj.count, 1, MPI_INT,recvBuff2, 1, MPI_INT, 0, MPI_COMM_WORLD);
			for(int i=0;i<np;i++)
				globalcount+=recvBuff2[i];
			printf("TOTAL TIMES THE SEQUENCE IS FOUND: %d\n",globalcount);
		}
		else
		{
			MPI_Gather(&obj.count, 1, MPI_INT,recvBuff2, 1, MPI_INT, 0, MPI_COMM_WORLD);			
		}
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==0)
		{
			recvBuff1 = (int*)malloc(globalcount*sizeof(int));
			displs = (int*)malloc(np*sizeof(int));
			displs[0]=0;
			for(int i=1;i<np;i++)
				displs[i] = displs[i-1] + recvBuff2[i-1]; 
			MPI_Gatherv(obj.arr,obj.count, MPI_INT,recvBuff1, recvBuff2,displs, MPI_INT, 0, MPI_COMM_WORLD);
			printf("POSITIONS FOR THE OCCURENCES ARE: ");
			for(int i=0;i<globalcount;i++)
				printf("%d,",recvBuff1[i]);
			printf("\n");

		}
		else
		{
			MPI_Gatherv(obj.arr,obj.count, MPI_INT,recvBuff1, recvBuff2,displs, MPI_INT, 0, MPI_COMM_WORLD);
		}
			
	}
	else
	{
		printf("FILE NOT FOUND!!!\n");
	}
	MPI_Finalize();
}

