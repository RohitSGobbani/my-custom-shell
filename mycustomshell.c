#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<limits.h>
#include<fcntl.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAX 100
#define MAX_HISTORY 100

int main(){
    
    int pid;
    // char input[MAX];
    char *args[10];
    char *home = getenv("HOME");
    char cwd[PATH_MAX] ;
    // char *history[MAX_HISTORY];
    // int history_count = 0;
    char prompt[PATH_MAX + 50];
    
    read_history("/tmp/.myshell_history");
    stifle_history(100);   // keep only last 100 commands


    while(1){

      //print command prompt
      if(getcwd(cwd,sizeof(cwd)) != NULL){
        if(strncmp(cwd,home,strlen(home))==0){
          snprintf(prompt, sizeof(prompt), "[mysh ~%s]# ",cwd + strlen(home));
        }else{
          snprintf(prompt, sizeof(prompt),"[mysh %s]# ",cwd);
        }
      }else{
        snprintf(prompt, sizeof(prompt), "[mysh]# ");
      }
      
      fflush(stdout);
      
      char *input = readline(prompt);
      if(!input) break;

      if(*input){
        add_history(input);
      }
      
      // //for ctrl+d
      // if(fgets(input,sizeof(input),stdin) == NULL){
      //   break;
      // }

      // input[strcspn(input,"\n")] = '\0';

      // //history
      // if(history_count < MAX_HISTORY){
      //   if(!(strcmp(input,"") == 0)){
      //     history[history_count] = strdup(input);
      //     history_count++;
      //   }
      // }

      // if(strcmp(input,"history") == 0){
      //   for(int i = 0; i<history_count; i++){
      //     printf("%d %s\n", i+1, history[i]);
      //   }
      // }

      //token/split the input
      char *token = strtok(input," ");
      int i=0;
      while(token!=NULL && i<9){
        args[i++]=token;
        token = strtok(NULL," ");
      }
      args[i]=NULL;

      //enter
      if(args[0] == NULL){
        free(input);
        continue;
      }

      //exit
      if(strcmp(args[0],"exit")==0){
        break;
      }

      //cd
      if(strcmp(args[0],"cd") == 0){
        if(args[1] == NULL){
            chdir(home);
        }else{
          chdir(args[1]);
        }
        free(input);
        continue;
      }

      //history
      if(strcmp(args[0],"history") == 0){
        HIST_ENTRY **list = history_list();
        if(list) {
          for(int j = 0; list[j]; j++){
            printf("%d %s\n", j+1, list[j]->line);
          }
        }
        free(input);
        continue;
      }

      //pipe
      int pipe_index = -1;
      for(int j = 0; args[j] != NULL; j++){
        if(strcmp(args[j],"|") == 0){
          pipe_index = j;
          break;
        }
      }

      if(pipe_index != -1){
        //left command
        char *left[10];
        for(int j = 0; j < pipe_index; j++){
          left[j] = args[j];
        }
        left[pipe_index] = NULL;

        //right command
        char *right[10];
        int k = 0;
        for(int j = pipe_index + 1; args[j] != NULL; j++){
          right[k++] = args[j];
        }
        right[k] = NULL;

        //creating two childs
        int fd[2];
        pipe(fd);

        if(fork() == 0){
          dup2(fd[1], STDOUT_FILENO);
          close(fd[0]);
          close(fd[1]);
          execvp(left[0],left);
        }

        if(fork() == 0){
          dup2(fd[0], STDIN_FILENO);
          close(fd[1]);
          close(fd[0]);
          execvp(right[0],right);
        }
        close(fd[0]);
        close(fd[1]);
        wait(NULL);
        wait(NULL);

        free(input);
        continue;
      }

      //child and parent processes
      pid = fork();
      
      if(pid == 0){

      //redirect >
      // for(int j=0; args[j] != NULL; j++)
        for(int j=0; args[j] != NULL; j++){
        if(strcmp(args[j],">") == 0){
          int fd = open(args[j+1], O_WRONLY | O_CREAT | O_TRUNC , 0644);
          if(fd < 0){
            perror("open");
            exit(1);
          }
          dup2(fd, STDOUT_FILENO);
          close(fd);

          args[j] = NULL; //cut the command from >
          break;
        }
      }
        
    

      //redirect <
      for(int j=0; args[j] != NULL; j++){
        if(strcmp(args[j],"<") == 0){
          int fd = open(args[j+1], O_RDONLY);
          if(fd < 0){
            perror("open");
            exit(1);
          }
          dup2(fd, STDIN_FILENO);
          close(fd);

          args[j] = NULL; //cut the command from >
          break;
        }
      }

        execvp(args[0],args);
        if(!(strcmp(args[0], "history") == 0)){
          perror("command not found");
        }
        
        exit(1);
      }else if(pid>0){
        wait(NULL);
      }else{
        perror("fork error");
      }
      free(input);
    }

    write_history("/tmp/.myshell_history");
    printf("GoodBye!!\n");
    return 0;
}

