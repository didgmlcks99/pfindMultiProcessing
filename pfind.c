#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <string.h>

int main(int argc, char** args){
	//check # of arguments 
	int argsNum = 0;
	do{
		argsNum++;
	}while(args[argsNum]);
	
	//check # of option 
	int numOpt = 0;
	int numProc = 0;
	for(int i = 0; i < argsNum; i++){
		if(args[i][0] == '-'){
			if(args[i][1] != 'p' && args[i][1] != 'c' && args[i][1] != 'a'){
				printf("Error : no such option as %s\n", args[i]);
				printf("!Existing options : -p <N>, -c, -a\n");
				printf("Terminating Program\n");
				exit(1);
			}else{ numOpt++; }
		}
		if(args[i][1] == 'p'){
			numProc = args[i+1][0] - '0';
			if(numProc < 8 && numProc > 0){
				numOpt++;
			}else{
				printf("Error : not by rule\n");
				printf("!In this limit : 0 < N < 8\n");
				printf("Terminating Program\n");
				exit(0);
			}
		}
	} 

	//save address # for directory and start of keyword
	int numDir = numOpt + 1;
	int numKey = numDir + 1;

	//printf("dir : %d\nkey : %d\n", numDir, numKey);

	//handle error when input is wrong
	if(args[numDir] == NULL || args[numKey] == NULL || argsNum == 1){
		printf("Error : invalid input\n");
		printf("!Include both : directory and keyword\n");
		printf("Terminating Program\n");
		exit(0);
	}

	//test for process using array
/*	pid_t process[3];
	process[0] = fork();
	if(process[0] == 0){
		printf("process[0] working\n");
		exit(1);
	}else{
		process[1] = fork();
		if(process[1] == 0){
			printf("process[1] working\n");
			exit(1);
		}else{
			printf("parent process will wait\n");
			wait(0x0);
			wait(0x0);
			kill(process[0], SIGCHLD);
			kill(process[1], SIGCHLD);
			printf("all process done in parent\n");
		}
	}	
*/
	return 0;
}
