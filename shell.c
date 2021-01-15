#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#define Pids_total 15
#define HIST_total 15
#define BUFFERSIZE 256
#define WHITESPACE " \t\n" 
int process_count = 0  ;
int pids[15] ;              //Child_pid for every new child process
int history_count = 0 ;  
int suspended = -999;        //Store the suspended child process and run it in the background when needed
int recent_child = -999 ;    //Store the recent child pid so that if it will get suspended then we can store in suspended pid
char historyArr[20][BUFFERSIZE];
int cnt = 0;
char *str[1000]; 
int commandIteration = 0;
int batchMode=0;
void do_pipe(char *buffer);
void pwd();
void execute_commands(char *token);
void removeSpaces(char *s);
int search(char *token, char character);
void inbuilt(char *cmd_str);
void do_cd(char* buffer);
void F_redirect(char *buffer);
static void handle_signal (int sig);


static void handle_signal (int sig)
{
  /*
    Handling signals Ctrl-C, Ctrl-Z, and Background the suspended processes
  */
   if(sig == 20)
   {
       suspended = recent_child; //The child who is suspended should be stored in suspended i.e. recent child should be stored in suspended
   //     printf("The process you are suspending is %d", suspended);
   }
   
   if( sig !=2 && sig !=20)
   {
      printf("Unable to determine the signal\n");
   }
   printf("\n");
}

static void print_pids()
{
  int i;
  for( i = 0; i < process_count; i++)
  {
     printf("%d: %d\n", i, pids[i]);
  }
}

//Function to print history
static void print_history(char ***history)
{
  int i;
  for( i = 0; i < history_count; i++)
  {
     printf("%d: %s\n", i, (*history)[i]);
  }
}


//Storing the pids everytime when there is new process start
static void manage_pid(int pid)
{
  // recent_child = pid;
  int lm;
  if(process_count < Pids_total)           
  {
     pids[process_count] = pid;
     process_count+=1;
  }
  else
  {
      for(lm = 0; lm < (Pids_total-1); lm++)
      {
          pids[lm] = pids[lm+1];
      }
      pids[14] = pid;
      process_count = 15;
  }
}

void printHistory()
{
    printf("Shell history:\n");
    int i;
    int j = 1;
    int hCount = cnt;

    for(i = 0; i < 20; i++)
    {
        printf("%d. ",hCount);
        while(historyArr[i][j] != '\n' && historyArr[i][j] != '\0')
        {
            printf("%c", historyArr[i][j]);
            j++;
        }
        printf("\n");
        j = 0;
        hCount--;
        if(hCount == 0)
        {
            break;
        }
    }
    printf("\n");

}



/*
add executing function and individual portions
*/
void pwd() {
	char path_name[BUFFERSIZE];

	if (getcwd(path_name, sizeof(path_name)) != NULL) {
			printf("%s\n", path_name);
		}
	 else {
			perror("Error printing working directory...\n");
			exit(EXIT_FAILURE);
			} // end if-else
}

//--------------------------search function for characters in tokens--------------------------
int search(char *token, char character){
	
    int i = 0;
    while(i < strlen(token)){
        if(character == token[i]){
            return 0;
        }
        i++;
    }
    return -1;

}

//-----------------------executing all the commands from the passed tokens-------------------------------------*/
void execute_commands(char *token){


				//	printf("the command is %s \n", token);
	strcpy(historyArr[0], token);
	cnt++;

	for(int i = 19; i > 0; i--)
		{
		  strcpy(historyArr[i], historyArr[i-1]);
		}
	if(cnt == 20)
		{cnt = 20;}

	if (strcmp(token, "path") == 0) {
		// print working directory
		pwd();
	}
	else if (strcmp(token, "cd")==0){
		//the cd function goes here
		//printf("\n CD\n");
		do_cd(token);
	}
	else if (strcmp(token,"myhistory")==0){
		//history function goes here
		printHistory();
	}	
	
	else if (search(token, '>')==0){
		//printf("\nfound your thing > \n");
		}
//	else if (search(token, '>>'))==0{
//		printf("\nfound your thing >> \n");
//		}
	else if (search(token, '<')==0){
	//	printf("\nfound another thing < \n");
		}
	else if(search(token, '|')==0){
		printf("\nfound another thing | \n");
		do_pipe(token);
		}
	
	else
		//send it to the function to execute commands
	
		inbuilt(token);
}


//---------------------- test the inbuilt comands using execv and managae background processes and also the pid's-----------------------------------
void inbuilt(char *cmd_str){

//					printf("the command is %s \n", cmd_str);

					char *token[1000];

            int   token_count = 0;                                 
                                                           
            // Pointer to point to the token
            // parsed by strsep
            char *arg_ptr;                                         
                                                           
            char *working_str  = strdup( cmd_str );                

            // we are going to move the working_str pointer so 
            // keep track of its original value so we can deallocate
            // the correct amount at the end
            char *working_root = working_str;

            // Tokenize the input stringswith whitespace used as the delimiter
            while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
               (token_count<1000))
             {
                token[token_count] = strndup( arg_ptr, 1000 );
                if( strlen( token[token_count] ) == 0 )
                {
                    token[token_count] = NULL;
                }
                token_count++;
             
            }
            
            //Exit the program with status 0 if user put "exit" or "quit"
              if( !strcmp(token[0], "quit") || !strcmp(token[0], "exit"))
              {
                     exit(0);
              }
               if( !strcmp(token[0], "bg")) //running the suspended program into background
               {
                     if(suspended != -999)
                     {
                         printf("BG %d\n", suspended );
                         kill(suspended, SIGCONT);
                         suspended = -999;
                     }
                     else {printf("There is no process to run in background\n");} //If there is no process which is suspended then user will get this message
                    
               }
               else if( !strcmp(token[0], "listpids"))
               {
                      print_pids(); 
               }
               
               else
               {
                    //executing command using execv
                    //Creating a child process first 
                    pid_t child_pid= fork();
                    if(child_pid !=0)
		    { recent_child = child_pid; }
		    else
                    { recent_child = getpid(); }
             
                    int status;
      
          
                   //If it is an child process then execute the command 
                  if( child_pid == 0 )
                  {
                        //If failure to find command then print command not find
                        if( execv(token[0], token ) == -1)
                        {  
                             char addon[256] = "/usr/local/bin/";
                             strcat(addon,token[0]);
                             if( execv(addon, token ) == -1)
                             {
                                 char addonn[256] = "/usr/bin/";
                                 strcat(addonn,token[0]);
                                 if( execv(addonn, token ) == -1)
                                 {
                                     char addonnn[256] = "/bin/";
                                     strcat(addonnn,token[0]);
                                     if(execv(addonnn, token ) == -1)
                                     {
                                         printf("%s : Command not found\n", token[0]);
                                     }
                                  }
                              }
                           } 
                           exit( EXIT_SUCCESS );
                    }
                  
                    free( working_root );
                   
                  waitpid( child_pid, &status, 0 );     

                   

				 manage_pid(child_pid); //Storing the child pid
               }    
                   
                       
}
        

void do_cd(char* buffer) {
    char path[100];
    // no argument is passed
    // the command will change the current working directory 
    // to the user’s HOME directory
    if (strcmp(buffer, "cd") == 0) {
        chdir(getenv("HOME"));
        return;
    }

    // changes the current working directory to that directory
    sscanf(buffer, "cd %s", path);
    int ret = chdir(path);
    if (ret == 0) {
        return;
    }
    if (errno == ENOENT) {
        fprintf(stderr, "cd: %s: No such file or directory\n", path);
        return;
    }
    if (errno == ENOTDIR) {
        fprintf(stderr, "cd: %s: Not a directory\n", path);
        return;
    }

    fprintf(stderr, "failed to cd: %s\n", path);
}


void do_pipe(char *buffer) {
    char arr_cmd[3][300];
    int num_cmd = sscanf(buffer, "%[^|] | %[^|] | %[^|]", 
        arr_cmd[0], arr_cmd[1], arr_cmd[2]);

    // create two pipes
    int pipefd_1[2];
    if (pipe(pipefd_1) < 0) {
        printf("pipe error\n");
        exit(-1);
    }
    int pipefd_2[2];
    if (pipe(pipefd_2) < 0) {
        printf("pipe error\n");
        exit(-1);
    }
    int pid = 0;
    int status;
    // run each sub command one by one.
    for (int i = 0; i < num_cmd; i++) {
        // wait for previous child process to finish.
        if (i != 0) {
            waitpid(pid, &status, 0);
            // printf("wait %d ok\n", pid);
        }

        // close output
        if (i == 1) {
            close(pipefd_1[1]);
        }
        // close output
        if (i == 2) {
            close(pipefd_2[1]);
        }
        // printf("sub cmd: %s\n", arr_cmd[i]);
        // use fork-execvp to execute sub command.
        pid = fork();
        if (pid < 0) {
            printf("fork error\n");
            exit(-1);
        }
        if (pid > 0) {
            // parent
           // printf("forked %d\n", pid);
        }
        if (pid == 0) {
            // child
            if (i == 0) {
                // output to pipefd_1
                dup2(pipefd_1[1], 1);
            }
            if (i == 1) {
                // input from pipefd_1
                dup2(pipefd_1[0], 0);
                // last sub command should output to stdout
                if (i != num_cmd - 1) {
                    // output to pipefd_2
                    dup2(pipefd_2[1], 1);
                }
            }
            if (i == 2) {
                // input from pipefd_2
                dup2(pipefd_2[0], 0);
            }

            // parse params from sub command
            // for example: 
            // sub command is : wc -l
            // arr_params[0] is wc
            // arr_params[1] is -l
            // arr_params[2] is NULL
            char* token;
            int j = 0;
            char *arr_params[10];
            const char s[2] = " ";
            token = strtok(arr_cmd[i], s);
            while (token) {
                arr_params[j] = token;
                j++;
                token = strtok(NULL, "|");
            }
            arr_params[j] = NULL;
            // printf("executing: %s\n", arr_params[0]);
            if (execvp(arr_params[0], arr_params) < 0) {
                printf("execvp error\n");
                exit(-1);
            }
        }
    }
    // wait for the last pid
    close(pipefd_2[0]);
    close(pipefd_2[1]);
    waitpid(pid, &status, 0);
}

// Function to remove all spaces from a given string 
void removeSpaces(char *s) {
char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;
    memmove(s, p, l + 1);
}



int main(int argc, char *argv[]) 
{
    char buffer[1000];
    char *cmds[1000];
    char* batch_file = NULL;
    FILE* fp;
    char **arguments;
    int status;
    char in[100]="PROMPT> ";
    char pro;
    
    struct sigaction act;
	
		/*
       Set the handler to use the function handle_signal()
    */ 
     act.sa_handler = &handle_signal;
 
     /* 
       Install the handler for SIGINT and SIGTSTP and check the 
       return value.*/
     
     if (sigaction(SIGINT , &act, NULL) < 0) 
     {
        perror ("sigaction: ");
        return 1;
     }
     if (sigaction(SIGTSTP , &act, NULL) < 0) 
     {
        perror ("sigaction: ");
        return 1;
     }
      
  
    if (argc == 1){
    		printf("Interactive mode....\n");
    		printf("Would you like to change your prompt(Y/N)? ");
    		scanf("%c", &pro);
    		if(pro=='y' || pro =='Y'){
    				printf("\nWhat would you like your prompt to be: ");
    				scanf("%s", in);
    		}
    		else if(pro=='n' || pro =='N'){
						printf("Starting interactive mode with inbuilt prompt\n.");
				}
				else{
						printf("Command not recognized. Starting shell with inbuilt prompt");
				}
		}
   
    if (argc > 2) {
        fprintf(stderr, "too many parameters.\n");
        return -1;
    }

    if (argc == 2) {
        // In batch mode, 
        // your shell is started by specifying a batch file on its command line
        batchMode = 1;
        batch_file = argv[1];
        fp = fopen(batch_file, "r");
        if (fp == NULL) {
            fprintf(stderr, "failed to open batch_file: %s.\n", batch_file);
            return -1;
        }
    }
    
    while (1) 
    {
         if (batchMode==0) {
            write(STDOUT_FILENO, &in, strlen(in));
            fgets(buffer, 1000, stdin);
        } else {
            // batchMode
            // should not display a prompt

            // get a line from file
            // Reading stops after an EOF or a newline
            // If a newline is read, it is stored into the buffer.
           		 fgets(buffer, 1000, fp);
            if (feof(fp)) {
                // printf("[DEBUG] batchMode reach the end of file.\n");
                break;
            }
        }

        if (batchMode) {
            // should echo each line you read from the batch file 
            // back to the user before executing it.
            printf("%s\n", buffer);
        //write(STDOUT_FILENO, buffer , strlen(buffer));
        }
        
        
					removeSpaces(buffer);
	
    	char *token = strtok(buffer, ";"); 

   	 int i= 0;
   	 int lm;
   	 while (token != NULL) { 
            str[i++] = token;            
     	   token = strtok(NULL, ";");
    }        
             int stringSize=0;
             //sending texts to execute
   					 if(i>0){
   					 for (int x = 0; x < i; ++x) {
    			     removeSpaces(str[x]);
    			     stringSize = (int)strlen(str[x]);
		  			   if ( stringSize > 0){
		    		   execute_commands(str[x]);
		    		  
		    		   for( lm = 0; lm <stringSize; lm++)
							 { 
							    str[x][lm] = '\0';
							 } 
       			 }
        }
        
        
}
       
		   for( lm = 0; lm <1000; lm++)
       { 
         buffer[lm]= '\0';
       } 

        
        buffer[0] = '\0';
         
    }
    return 0;
}
