#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fcntl.h"
#include "../user/user.h"
#include "../kernel/param.h"


int main(int argc, char *argv[])
{
	int x_argc;
	char *x_argv[MAXARG];
	char buf[512];
	char *start, *end;	//pointer that point in buffer

    if (argc <= 1){ //check argc
        fprintf(2, "Error: missing command. Usage: xargs <command> [arg ...]\n");
        exit(1);
    } 

	x_argc = argc - 1; //copy argv into x_argv
	for (int i = 1; i < argc; i++){
		x_argv[i - 1] = argv[i];
    }    
	start = buf;
    end = buf;

	while (read(0, end, sizeof(char))) { //read -> lưu vào đchi end
		if (*end == '\n') {
			//put in arg
			if (x_argc == MAXARG - 1) {	//if full
				fprintf(2, "xargs: too many arguments\n");
				start = end = buf;
				*end = '\0';

				while (*end != '\n') {
                    read(0, end, sizeof(char));
                }    
			} else {			// not full
				*end = '\0';
				x_argv[x_argc++] = start;
				end++;
				start = end;
			}

			//fork and exec
			if (fork() == 0) {
				exec(x_argv[0], x_argv);
			}

			//empty buffer and recover x_argc = argc - 1
			start = end = buf;
			x_argc = argc - 1;

		} else if (*end == ' ') {
			if (x_argc == MAXARG - 1) {	//nếu đầy
				fprintf(2, "xargs: too many arguments\n");
				start = end = buf;
				*end = '\0';
				while (*end != '\n') {
                    read(0, end, sizeof(char));
                }    
			} else {			// else
				*end = '\0';
				x_argv[x_argc++] = start;
				end++;
				start = end;
			}


		} else {
			end++; 
		}
	}

	if (buf != end) {	//while finished, but not empty
		if (x_argc == MAXARG-1) {	//if full
			fprintf(2, "xargs: too many arguments\n");
			start = end = buf;
			*end = '\0';
			while (*end != '\n') {
                read(0, end, sizeof(char));
            }    
		} else {			// not full
			*end = '\0';
			x_argv[x_argc++] = start;
			end++;
			start = end;
		}
		
		if(fork() == 0) {
			exec(x_argv[0], x_argv);
		}
	}
	
	wait(0);
	exit(0);
}