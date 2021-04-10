#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int arc, char** args){

#ifdef DEBUG
	printf("==> THIS IS DEBUG MODE <==\n");
#endif

	//check # of arguments 
	int argsNum = 0;
	do{
		argsNum++;
	}while(args[argsNum]);

#ifdef DEBUG
	printf("==> NUMBER OF ARGS : %d\n", argsNum);
	printf("==> INPUT : ");
	for(int i = 0; i < argsNum; i++){
		printf("%s ", args[i]);
	}printf("\n");
#endif
	
	//check # of option + (number of worker) 
	int numOpt = 0;
	int numProc = 2;
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
			if(numProc <= 8 && numProc >= 1){
				numOpt++;
			}else{
				printf("Error : not by rule\n");
				printf("!In this limit : 1 <= N <= 8\n");
				printf("Terminating Program\n");
				exit(0);
			}
		}
	} 

#ifdef DEBUG
	printf("==> NUMBER OF OPTIONS : %d\n", numOpt);
	printf("==> OPTIONS : ");
	for(int i = 1; i <= numOpt; i++){
		printf("%s ", args[i]);
	}printf("\n");
	printf("==> # OF WORKERS : %d\n", numProc);
#endif

	//save space # for directory and start of keyword
	int numDir = numOpt + 1;
	int numKey = numDir + 1;
	int countKey = argsNum - numKey;

#ifdef DEBUG
	printf("==> DIR SPACE : %d\n", numDir);
	printf("==> KEY SPACE : %d\n", numKey);
	printf("==> # OF KEYWORDS : %d\n", countKey);
	printf("==> DIRECTORY NAME : %s\n", args[numDir]);
	printf("==> KEYWORD NAMES : ");
	for(int i = numKey; i < numKey + countKey; i++){
		printf("%s ", args[i]);
	}printf("\n");
#endif

	//handle error when input is wrong
	if(args[numDir] == NULL || args[numKey] == NULL || argsNum == 1){
		printf("Error : invalid input\n");
		printf("!Include both : directory and keyword\n");
		printf("Terminating Program\n");
		exit(0);
	}

	//open the directory to be searched
	DIR *directory = opendir(args[numDir]);
	if(directory == NULL){
		printf("ERROR : cannot open this directory '%s'\n", args[numDir]);
		printf("Terminating Program\n");
		exit(0);
	}

#ifdef DEBUG
	printf("==> DIRECTORY '%s' IS SUCCESSFULLY OPENED\n", args[numDir]);
#endif

	//looks into each file of the given directory
	struct dirent *each_file;
	while((each_file = readdir(directory)) != NULL){
		char path_name[100];
		if(strcmp(each_file->d_name, "..") == 0 || strcmp(each_file->d_name, ".") == 0){}
		else{
			sprintf(path_name, "%s/%s", args[numDir], each_file->d_name);
#ifdef DEBUG
			//executes Linux command : file <file>
			int file_type;
			pid_t checker = fork();
			if(checker == 0){
				file_type = open("fileType.txt", O_WRONLY | O_CREAT, 0644);
				if(file_type == -1){
					printf("Error : cannot open file\n");
					printf("Terminating Program\n");
					exit(1);
				}

				//dup standard output to file, result of Linux command : file
				dup2(file_type, 1);
				close(file_type);
				
				execlp("file", "file", path_name, NULL);
			}wait(0x0);

			//check file type
			checker = fork();
			if(checker == 0){
				file_type = open("fileType.txt", O_RDONLY | O_CREAT, 0644);
				if(file_type == -1){
					printf("Error : cannot open file\n");
					printf("Terminating Program\n");
					exit(0);
				}

				//dup standard input to file, scanf result of Linux command : file
				dup2(file_type, 0);
				close(file_type);
				
				char typeFL[50];
				scanf("%[^\n]s", typeFL);

				if(strstr(typeFL, "ASCII") != NULL || strstr(typeFL, "text") != NULL){
					printf("==> FILE TYPE : ASCII or text\n");
				}else if(strstr(typeFL, "directory") != NULL){
					printf("==> FILE TYPE : directory\n");	
				}else{
					printf("==> FILE TYPE : not regular");
				}

				exit(1);
			}wait(0x0);

			printf("==> FILE : %s\n", path_name);
#endif
		}
	}

/*	
	//make named pipe : tasks
	if (mkfifo("tasks", 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}
	
	//make named pipe : results
	if (mkfifo("results", 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}
*/

	//make workers in the user given amount
	pid_t workers[numProc];
	for(int i = 0; i < numProc; i++){
		workers[i] = fork();
		if(workers[i] == 0){
#ifdef DEBUG
			printf("==> WORKER #%d AT WORK...\n", i);
#endif
			exit(1);
		}
	}

	//manager waiting for all worker to end
	for(int i = 0; i< numProc; i++){
		wait(0x0);
	}

#ifdef DEBUG
	printf("==> ALL WORKERS DONE CHECKED BY MANAGER PROCESS...\n");
#endif

	return 0;
}
