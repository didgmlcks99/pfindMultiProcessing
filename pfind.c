#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int write_bytes(int fd, void * a, int len){
	char * s = (char *) a;

	int i = 0;
	while(i < len){
		int b;
		b = write(fd, s+i, len-i);
		if(b == 0) break;
		i += b;
	}
	return i;
}

int read_bytes (int fd, void * a, int len){
	char * s = (char *) a ;
	
	int i ;
	for (i = 0 ; i < len ; ) {
		int b ;
		b = read(fd, s+i, len-i) ;
		if (b == 0)
			break ;
		i += b ;
	}
	return i ; 
}

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
				exit(1);
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

//================================================================================

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

	//make workers in the user given amount
	pid_t workers[numProc];
	for(int i = 0; i < numProc; i++){
		workers[i] = fork();
		if(workers[i] == 0){
#ifdef DEBUG
			printf("==> WORKER #%d AT WORK...\n", i);
#endif
			int tasks_rec = open("tasks", O_RDONLY | O_SYNC);
			int results_send = open("results", O_WRONLY | O_SYNC);
			
			//wait for task in tasks_rec pipe
			while(1){
				char task_name[128];
				size_t len, bs;

				flock(tasks_rec, LOCK_EX);

				if(read_bytes(tasks_rec, &len, sizeof(len)) != sizeof(len)){
					printf("here break in task read\n");
					flock(tasks_rec, LOCK_UN);
					break;
				}

				bs = read_bytes(tasks_rec, task_name, len);
				task_name[bs] = 0x0;

				flock(tasks_rec, LOCK_UN);

#ifdef DEBUG
				printf("==> WORKER #%d RECEIVED TASK : %s...\n", i, task_name);		
#endif

				// for(int i = 0; i < bs; i++){
				// 	putchar(task_name[i]);
				// }printf("\n");

				//open the directory to be searched
				DIR *directory = opendir(task_name);
				if(directory == NULL){
					printf("ERROR : cannot open this directory '%s'\n", task_name);
					printf("Terminating Program\n");
					exit(0);
				}

#ifdef DEBUG
				printf("==> DIRECTORY '%s' IS SUCCESSFULLY OPENED\n", task_name);
#endif

//open pipe for sending data from linux command, file
						int pipes[2];
						if(pipe(pipes) != 0){
							perror("Error : cannot open pipe\n");
							printf("Terminating Program\n");
							exit(1);
						}

// #ifdef DEBUG
						printf("==> PIPE:FILE : [%d, %d]\n", pipes[0], pipes[1]);
// #endif

				//looks into each file of the given directory
				struct dirent *each_file;
				while((each_file = readdir(directory)) != NULL){
					char path_name[50];
					if(strcmp(each_file->d_name, "..") == 0 || strcmp(each_file->d_name, ".") == 0){}
					else{
						printf("%s\n", each_file->d_name);

						sprintf(path_name, "%s/%s", task_name, each_file->d_name);

// 						//open pipe for sending data from linux command, file
// 						int pipes[2];
// 						if(pipe(pipes) != 0){
// 							perror("Error : cannot open pipe\n");
// 							printf("Terminating Program\n");
// 							exit(1);
// 						}

// #ifdef DEBUG
// 						printf("==> PIPE:FILE : [%d, %d] : %s\n", pipes[0], pipes[1], path_name);
// #endif

						//executes Linux command : file <file>, sending result to pipe
						pid_t checker = fork();
						if(checker == 0){
							close(pipes[0]);
							dup2(pipes[1], 1);
							execlp("file", "file", path_name, NULL);
							exit(1);
						}wait(0x0);

						//check file type, from pipe
						pid_t maker = fork();
						if(maker == 0){
							close(pipes[1]);
							dup2(pipes[0], 0);

							char typeFL[50];

							scanf("%[^\n]s", typeFL);

#ifdef DEBUG
							printf("==> EXEC RESULT : %s\n", typeFL);
#endif

							if(strstr(typeFL, "ASCII") != NULL || strstr(typeFL, "text") != NULL){
								printf("FILE TYPE : ASCII or text\n");
							}else if(strstr(typeFL, "directory") != NULL){
#ifdef DEBUG
								printf("==> FILE TYPE : directory\n");
								printf("==> WORKER #%d SENT TASK : %s...\n", i, path_name);
#endif			/*
								size_t len = strlen(path_name);
								flock(results_send, LOCK_EX);
								if(write_bytes(results_send, &len, sizeof(len)) != sizeof(len)){}
								if(write_bytes(results_send, path_name, len) != len){
									printf("here break in result send\n");
									flock(results_send, LOCK_UN);
									break;
								}
								flock(results_send, LOCK_UN);
							*/}else{
								printf("FILE TYPE : not regular\n");
							}
							exit(1);
						}wait(0x0);
					}
				}
			}
			// close(tasks_rec);
			// close(results_send);
			exit(1);
			//kill(workers[i], SIGCHLD);
		}
	}

	char task[128];
	strcpy(task, args[numDir]);

	int tasks_send = open("tasks", O_WRONLY | O_SYNC);
	int results_rec = open("results", O_RDONLY | O_SYNC);
	while(1){
#ifdef DEBUG
		printf("==> MANAGER SENDING TASK : %s\n", task);
#endif
		//manager send task through tasks_send pipe
		size_t len = strlen(task);

		if(write_bytes(tasks_send, &len, sizeof(len)) != sizeof(len)){}
		if(write_bytes(tasks_send, task, len) != len){
			printf("here break in task send\n");
			break;
		}
			
		//wait for worker in results_rec pipe
		char result_name[128];
		size_t length, bs;

		// flock(results_rec, LOCK_EX);

		if(read_bytes(results_rec, &length, sizeof(length)) != sizeof(length)){
			printf("here break in result read\n");
			// flock(results_rec, LOCK_UN);
			// break;
		}

		bs = read_bytes(results_rec, result_name, length);
		result_name[bs] = 0x0;

		// flock(results_rec, LOCK_UN);

#ifdef DEBUG
		printf("==> MANAGER RECEIVED TASK : %s\n", result_name);
#endif

		strcpy(task, result_name);

		// for(int i = 0; i < bs; i++){
		// 	putchar(result_name[i]);
		// }printf("\n");
	}
	close(tasks_send);
	close(results_rec);

	//manager waiting for all worker to end
	// for(int i = 0; i< numProc; i++){
	// 	wait(0x0);
	// }

#ifdef DEBUG
	printf("==> ALL WORKERS DONE CHECKED BY MANAGER PROCESS...\n");
#endif

	return 0;
}
