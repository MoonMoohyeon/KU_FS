#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "ku_fs_input.h"

int main(int argc, char* argv[])
{	
	if(argc != 3)
	{
		printf("not valid argc\n");
		return -1;	
	}

	char* temp = argv[2];
	for(int i=0; i<strlen(argv[2]); i++)
	{
		if(!isdigit(temp[i]))
		{
			printf("not valid argv\n");
			return -1;
		}
	}

	int pipefd[2];	
	char* target = argv[1];
	int processNum = atoi(argv[2]);
	int targetLength = strlen(target);
	int startingInterval = MAXS / processNum;
	int dividedLength = startingInterval + targetLength;
	int mod = MAXS - processNum * startingInterval;
//	pid_t pid[MAXS];
	char text[dividedLength + 1 + mod];

	int readArray[MAXS];
	int readValue;
	int count = 0;


	if(targetLength > MAXS-1 || processNum > MAXS || processNum <= 0)
	{
		printf("not valid argv\n");
		return -1;
	}
	
//	printf("MAXS is %d\n", MAXS);
//	printf("target is %s\n", target);
//	printf("processNum is %d\n",processNum);
//	printf("targetLength is %d\n", targetLength);
//	printf("startingInterval is %d\n", startingInterval);	
//	printf("dividedLength is %d\n", dividedLength);
//	printf("mod is %d\n\n", mod);

//	if(targetLength > dividedLength)
//	{
//		printf("not valid pnum");
//		return -1;
//	}
	if(pipe(pipefd) == -1)
	{
		printf("pipe error\n");
		return -1;
	}

	for(int i=0; i<processNum; i++)
	{
		if((fork()) == 0)
		{
			if(i == processNum-1)
			{
//				printf("enter!!\n");
				strncpy(text, input+startingInterval*i, dividedLength + mod);
				text[dividedLength+mod] = '\0';
			}
			else
			{	
				strncpy(text, input+startingInterval*i, dividedLength);
				text[dividedLength] = '\0';
			}
			// targetLength
			int dlen = strlen(text); // dividedLength + (mod)
//			printf("text = %s, dividedLength = %d, dlen = %d, startingInterval*i = %d\n", text, dividedLength, dlen, startingInterval*i);
			for(int j=0; j<=dlen-targetLength; j++) // naive searching
			{
				for(int k=0; k<targetLength; k++)
				{
			//		assert(j+k < dlen);
					if(text[j+k] != target[k]) break;
					if(k == targetLength - 1)
					{
						int temp = j + i * startingInterval;
						write(pipefd[1], &temp, sizeof(int));
					}
				}	
			}
			close(pipefd[1]);
			close(pipefd[0]);
			exit(0);
		} // child process terminated
	}
	
	close(pipefd[1]);
	while(read(pipefd[0], &readValue, sizeof(int)) > 0)	// parent process receives
	{
//		printf("read value is %d\n", readv);
		readArray[count++] = readValue;
	}
	close(pipefd[0]);

	for(int i=0; i<count; i++) // selection sort
	{
		int minindex = i;
		for(int j=i+1; j<count; j++)
		{
			if(readArray[j] < readArray[minindex])
				minindex = j;
		}
		if(minindex != i)
		{
			int temp = readArray[minindex];
			readArray[minindex] = readArray[i];
			readArray[i] = temp;
		}
	}

	if(count != 0) // print
	{	
		printf("%d\n", readArray[0]);
		for(int i=1; i<count; i++)
		{
			if(readArray[i-1] == readArray[i]) continue;
			printf("%d\n",readArray[i]);
		}
	}

	return 0;
}