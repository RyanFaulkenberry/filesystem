#include "shell.h"

int main() {

FS* fs = createFS();

char str[PATH_LENGTH_MAX + COMMAND_LENGTH_MAX];
char strCommand[COMMAND_LENGTH_MAX];
char strPath[PATH_LENGTH_MAX];
char strTempCommand[COMMAND_LENGTH_MAX];
char strTempPath[PATH_LENGTH_MAX];
memset(str, '\0', sizeof(str));
memset(strCommand, '\0', sizeof(strCommand));
memset(strPath, '\0', sizeof(strPath));
memset(strTempCommand, '\0', sizeof(strTempCommand));
memset(strTempPath, '\0', sizeof(strTempPath));

char quitStr[5] = "quit";
char lsStr[3] = "ls";
char pwdStr[4] = "pwd";
char cdStr[3] = "cd";
char mkdirStr[6] = "mkdir";
char touchStr[6] = "touch";
char rmStr[3] = "rm";


	while (fgets(str, PATH_LENGTH_MAX + COMMAND_LENGTH_MAX + 1, stdin)) { 
		// loop until quit command

		// Remove newline
		char* newlinePtr = strchr(str, '\n');
		*newlinePtr = '\0';

		// Look for space char
		char* ptr = strchr(str, 32);

		// check for absence of space char (indicates pathless command)
		if (ptr == NULL) {
		    	// Pathless commands
	    
	   		if (strcmp(str, quitStr) == 0)
				return 0;
	
	   		else if (strcmp(str, pwdStr) == 0)
				pwd(fs->CWD);

	    		else if (strcmp(str, lsStr) == 0)
				ls(fs->CWD, NULL);
			else
				printf("Command not found\n");
		}	


		//Commands followed by paths
		else {
			//separate command and path
			char* a = &str[0];
			memcpy(strCommand, a, ptr - a);
			memcpy(strPath, ptr+1, (a + sizeof(str)) - ptr);
	
	        	if (strcmp(strCommand, lsStr) == 0) {
				if (strPath[0] == '/')
					ls(fs->root, strPath+1);
				else
					ls(fs->CWD, strPath);
		        }
	        	else if (strcmp(strCommand, cdStr) == 0) {
				if (strPath[0] == '/')
					fs->CWD = cd(fs->root, strPath+1);
				else
					fs->CWD = cd(fs->CWD, strPath);
	        	}
		    	else if (strcmp(strCommand, touchStr) == 0) { // touch
				if (strPath[0] == '/')
			    		fs->CWD = mkdir(fs->root, strPath+1, 0);
				else
					fs->CWD = mkdir(fs->CWD, strPath, 0);
	    		} 
		    	else if (strcmp(strCommand, mkdirStr) == 0) { // mkdir
				if (strPath[0] == '/')
			    		fs->CWD = mkdir(fs->root, strPath+1, 1);
				else
					fs->CWD = mkdir(fs->CWD, strPath, 1);
	    		}
		    	else if (strcmp(strCommand, rmStr) == 0) {
				if (strPath[0] == '/')
					fs->CWD = rm(fs->root, strPath+1);
				else
					fs->CWD = rm(fs->CWD, strPath);
	    		}

			else {
				printf("%s %s\n", strCommand, strPath);
				printf("Command not found\n");
			}
		}
		memset(strCommand, '\0', sizeof(strCommand));
		memset(strPath, '\0', sizeof(strPath));
    	}
return -1;
}
