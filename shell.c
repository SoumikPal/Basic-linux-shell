#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#define MAX 1000
int pfd[10][2];  
// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J")
void openHelp() { 
	printf("\n***HELP***\nList of Commands supported:\n>cd\n>ls\n>exit\n>all other general commands available in UNIX shell\n>pipe handling\n>improper space handling"); 
	return; 
} 
int ownCmdHandler(char** parsed) { 
	int NoOfOwnCmds = 3, i, switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	char* username; 
	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "help"; 
	for (i=0; i<NoOfOwnCmds; i++) { 
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
			switchOwnArg = i+1; 
			break; 
		} 
	} 
	switch(switchOwnArg){ 
	case 1: 
		printf("Goodbye,take care\n"); 
		exit(0); 
	case 2: 
		chdir(parsed[1]); 
		return 0; 
	case 3: 
		openHelp(); 
		return 0; 
	default: 
		break; 
	} 
	return 1; 
}  
int parsepipe(char* str, char** strpiped) { 
	int i; 
	for (i = 0; i < MAX; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 
	return i;
}


// function for parsing command words 

int parseor(char* str, char** parsed) {
     char res[100]="";
     int c=0; 
	for (int i = 0;i<strlen(str);i++)
	{
		if(str[i]=='|'&&str[i+1]=='|')
		{
			//printf("%s\n",res);
			char *rr=(char *)malloc(100*sizeof(char));
			strcpy(rr,res);
			parsed[c]=rr;
			//printf("%s\n",parsed[c]);
			c++;
			strcpy(res,"");
			i++;
		}
		else
			{
			    char c[2];
			    c[0]=str[i];
			    c[1]='\0';
			    strcat(res,c);
			}
	}
	//printf("%s\n",res);
	parsed[c]=res;
	//printf("%sPP\n",parsed[c]);
	parsed[c+1]=NULL;
	return c+1;
}
int parseand(char* str, char** parsed) { 
	int i; 
	for (i = 0; i < MAX; i++) { 
		parsed[i] = strsep(&str, "&&");
		if (parsed[i] == NULL) //parsing complete 
			break;
		if (strlen(parsed[i]) == 0) //parsing complete 
			{i--;
		    continue;
		}
		//printf("%s %lud and \n",parsed[i],strlen(parsed[i]));
		  
	}
	return i; 
	//parsed[i]=NULL;
}
void parseSpace(char* str, char** parsed) { 
	int i; 
	for (i = 0; i < MAX; i++) { 
		parsed[i] = strsep(&str, " "); 
		if (parsed[i] == NULL) //parsing complete 
			break; 
		if (strlen(parsed[i]) == 0) //redundat space avoided
			i--; 
	} 
	//parsed[i]=NULL;
}
// Function where the system command is executed
int  execArgs(char** parsed) {

	// Forking a child
    
	pid_t pid = fork(); 
	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return 0; 
	} 
	else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command..\n");
			exit(1); 
		} 
		//exit(0); 
	} 
	else { 
		// waiting for child to terminate 
		int status;
		wait(&status);
		printf("Exit status: %d\n", WEXITSTATUS(status));  
		return WEXITSTATUS(status); 
	} 
}
int execArgsPiped(char** parsed, char** parsedpipe) { 
	// 0 is read end, 1 is write end 
	int pipefd[2]; 
	pid_t p1, p2; 
	if (pipe(pipefd) < 0) { 
		printf("\nPipe could not be initialized"); 
		return 1; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return 1; 
	} 
	if (p1 == 0) { 
		// Child 1 executing.. 
		// It only needs to write at the write end 
		close(pipefd[0]);//close read end 
		dup2(pipefd[1], 1); 
		close(pipefd[1]); 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command 1.."); 
			exit(1); 
		} 
	} 
	else { 
		// Parent executing 
		int stat;
		wait(&stat);
		if(WEXITSTATUS(stat))
		{
			printf("Exit status: %d\n", WEXITSTATUS(stat));  
		    return WEXITSTATUS(stat);
		} 
		p2 = fork(); 
		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return 1; 
		} 
		// Child 2 executing.. 
		// It only needs to read at the read end 
		close(pipefd[1]); 
		if (p2 == 0) { 
			
			dup2(pipefd[0], 0); 
			close(pipefd[0]); 
			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				printf("\nCould not execute command 2.."); 
				exit(1); 
			} 
		} 
		else { 
			// parentxecuting, waiting for two children 
			int status;
		    wait(&status);
		    printf("Exit status: %d\n", WEXITSTATUS(status));  
		    return WEXITSTATUS(status); 

			//wait(p1); 
			//dup2( 0,STDIN_FILENO); 
			//dup2( 1,STDOUT_FILENO);
		} 
	} 
}
int execArgsPipedM(char** inputCommands,int st,int ls,int ci) {
     char* ic[MAX];
     parseSpace(inputCommands[ci], ic);
	if(ci==st){
	if (pipe(pfd[st]) < 0) { 
		printf("\nPipe could not be initialized"); 
		return 1; 
	}
	//printf("%d bb %d\n",pfd[st][0],pfd[st][1]); 
	pid_t p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return 1; 
	} 
	if (p1 == 0) { 
		// Child 1 executing.. 
		// It only needs to write at the write end 
		//printf("Executing 1st pipe\n");
		close(pfd[st][0]);//close read end 
		dup2(pfd[st][1], 1);
		 //printf("Executing 11st pipe\n");
		close(pfd[st][1]); 
		if (execvp(ic[0], ic) < 0) { 
			printf("\nCould not execute command 1.."); 
			exit(1); 
		} 
	}
	else
	{
		//Parent executing 
		int stat;
		wait(&stat);
		if(WEXITSTATUS(stat))
		{
			printf("Exit status: %d\n", WEXITSTATUS(stat));  
		    return WEXITSTATUS(stat);
		}
		//printf("Executing 1stp pipe\n");
		int cc=execArgsPipedM(inputCommands,st,ls,ci+1);
		return cc;

	}
} 
	else if(ci==ls){ 
		// Parent executing 
		//int stat;
		//wait(&stat);
		//if(WEXITSTATUS(stat))
		//{
			//printf("Exit status: %d\n", WEXITSTATUS(stat));  
		    //return WEXITSTATUS(stat);
		//}
		//printf("%d  l %d\n",pfd[ci-1][0],pfd[ci-1][1]); 

		pid_t p2 = fork(); 
		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return 1; 
		} 
		// Child 2 executing.. 
		// It only needs to read at the read end 
		close(pfd[ci-1][1]); 
		if (p2 == 0) { 
			
			dup2(pfd[ci-1][0], 0); 
			close(pfd[ci-1][0]); 
			//close(pfd[ci-1][1]);
			//printf("executing last pipe\n"); 
			if (execvp(ic[0], ic) < 0) { 
				printf("\nCould not execute command 2.."); 
				exit(1); 
			} 
		}
		else
		{

			int stat;
		    wait(&stat);
		    //close(pfd[ci-1][1]);
		    //printf("Done\n");
			printf("Exit status: %d\n", WEXITSTATUS(stat));  
		    return WEXITSTATUS(stat);

             
		} 
	}
			else { 
			
			if (pipe(pfd[ci]) < 0) { 
			printf("\nPipe could not be initialized"); 
			return 1; 
			}
			//printf("%d mb %d\n",pfd[ci-1][0],pfd[ci-1][1]);
			//printf("%d mn %d\n",pfd[ci][0],pfd[ci][1]);
			pid_t p3=fork();
			close(pfd[ci-1][1]);
  			//close(pfd[ci][0]);

			if(p3==0){

			dup2(pfd[ci-1][0], 0);
            //printf("executing middle pipe\n");
  			dup2(pfd[ci][1], 1);
            //printf("executing middle pipe\n");
  			close(pfd[ci-1][0]);
  			//close(pfd[ci-1][1]);
  			close(pfd[ci][0]);
  			close(pfd[ci][1]);

              if(execvp(ic[0], ic) < 0) { 
				printf("\nCould not execute command 2.."); 
				exit(1); 
			} 
  			
		}
		else
		{
			int stat;
		wait(&stat);
		//printf("progressing\n");
		if(WEXITSTATUS(stat))
		{
			printf("Exit status: %d\n", WEXITSTATUS(stat));  
		    return WEXITSTATUS(stat);
		}
		int cc=execArgsPipedM(inputCommands,st,ls,ci+1);
		return cc;
             
		}
		} 
	
} 
 
int  executeo(char *str)
{
	//bprintf("%s\n",str);
	char* inputCommands[MAX];
	int total = parsepipe(str, inputCommands);

	//printf("%d Or Parsedpipe\n",total);
	if(total==1)
	{
		char* ic[MAX];
        parseSpace(inputCommands[0], ic);
		if (ownCmdHandler(ic))
		{
		   int c=execArgs(ic);
		   return c;
		}
		else return 0;	
	}
	/*else if(total==2)
	{
		char* ic[MAX];
        parseSpace(inputCommands[0], ic);
        char* bc[MAX];
        parseSpace(inputCommands[1], bc);
        int c=execArgsPiped(ic,bc);
        return c;

         
	}*/
	else
	{
		int i=0;
		int c=execArgsPipedM(inputCommands,0,total-1,i);
		return c;
	}
	return 1;
}
int executea(char *str)
{
	//printf("%s\n",str);
	char* inputCommands[MAX];
	int total = parseor(str, inputCommands);

	//printf("%d Or Parsed\n",total);
	for(int j=0;j<total;j++)
	{
		int c=executeo(inputCommands[j]);
		if(c==0)
        {
        	//printf("ExecutedOr\n");
        	return 0;
        }
	}
	return 1;




}

void execute(char *str)
{
	//printf("%s\n",str);
	char* inputCommands[MAX];
	int total = parseand(str, inputCommands);
	//printf("%d And Parsed\n",total);
	for(int j=0;j<total;j++)
	{
		int c=executea(inputCommands[j]);
		if(c==1){
		printf("Command not found\n");
		break;
		}
	}




}
void setup(){
	clear();
	//printf("---------------------------is shell me aapka swagat hai-----------\n");
	//char* user = getenv("USER");
	//printf("Maalik  = %s : \n",user);
	//sleep(2);
	clear();
}
// Help command builtin 

void printDir() { 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("\033[0;32m");
	printf("\n%s$ ", cwd);
	printf("\033[0m"); 
} 
int takeInput(char *str){
	char buf[1000];
	fgets(buf,sizeof(buf),stdin);
	//printf("%d length %s\n",strlen(buf),buf);
	int fl=0;
	for(int i=0;i<strlen(buf);i++){
		if(buf[i]!=' '&&buf[i]!='\n'){
			fl=1; break;
		}
	}
	if(fl==0) return 1;
	if(strlen(buf)>0){
		//printf("\t\tzzz\n");
		int l=strlen(buf);
		buf[l-1]='\0';
		strcpy(str, buf);
		return 0;
	}
	//printf("%d length %s\n",strlen(buf),buf);
	return 1;
}




int stringToCommands(char* str, char** commands){
	int i;
	for(i=0; i<MAX; i++){
		commands[i]=strsep(&str, ";");
		if(commands[i]==NULL)
			break;
	}	
	return i;
} 

int main() { 
	char inputString[MAX];
	setup(); 
	while (1) {  
		printDir(); 
		if (takeInput(inputString)) 
			continue; 
		char* inputCommands[MAX];
		int total = stringToCommands(inputString, inputCommands);
		for(int i=0;i<total;i++){ 
            execute(inputCommands[i]);  
			printf("%d executed\n",i);
		}
	} 
	return 0; 
} 
