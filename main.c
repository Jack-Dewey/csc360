#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"
#include "linked_list.c"
Node* head = NULL;
FILE *pf;

    // This is the vestige of my attempt to error handle when a process
    // Is terminated outside of PMan
    /* 
void death_alert(){
			// int opts = WNOHANG | WUNTRACED | WCONTINUED;
			int opts = WUNTRACED|WCONTINUED;
      int retVal;
	    int status;
			retVal = waitpid(-1, &status, opts);
			if (retVal == -1) { 
				perror("Fail at waitpid"); 
				exit(EXIT_FAILURE); 
			}
      printf("pid %d was terminated and removed from list", retVal);
			// Macros below can be found by "$ man 2 waitpid"
    			if (WIFEXITED(status)) {
     				printf("Normal, status code=%d\n", WEXITSTATUS(status));  // Display the status code of child process
    			} else if (WIFSIGNALED(status)) {
    				printf("killed by signal %d\n", WTERMSIG(status));
    			} else if (WIFSTOPPED(status)) {
    				printf("stopped by signal %d\n", WSTOPSIG(status));
    			} else if (WIFCONTINUED(status)) {    
      				printf("continued\n");   
   			}

}
*/

void func_BG(char **cmd){
  	// This function creates a new process in the background
    // To do so, when prompted by PMan enter
    // "bg ./test" to start the process ./test in the background
    char* fn_name = cmd[0];
    char* fn_tag = cmd[1];
    char* copied_tag;
    memcpy(copied_tag, fn_tag, sizeof(fn_tag));
    int pid = fork();               // Fork here and save the pid
    if (pid==0){            // If pid==0 do  execvp : Child process
      int program = execvp(cmd[1], cmd);
      printf("%d \n", program);
      if (program < 0){
        pid = 0;
        exit(-1);
        return;
      }
    } else if (pid>0) { // Else if pid >0 add it to the linked list as a node   : Parent
      char* fn_tag = cmd[1];
      head = add_newNode(head, pid, copied_tag); // Add a new node to linkedList
    }else{ // We should never get here
      printf("no fork\n");
    }
}


void func_BGlist(char **cmd){
  // This function prints processes explicitly created by this instance of PMan
  // Processes created prior to this PMan initialization are not shown
  if (head == NULL){
    printf("There are no active processes \n");
    return;
  }
  printf("Below is a list of processes created by this PMan \n");
  printf("PID ----- PATH \n");
  printf("----------------------- \n");
	printList(head);
}


void func_BGkill(char * str_pid){
  // This function kills a process with a given pid
  
  int pid_int = atoi(str_pid);
  if (PifExist(head, pid_int) == 0){
      printf("This process was either started outside this instance of PMan or does not exist. \n");
      return;
  } else{
    kill(pid_int, SIGTERM);
    head = deleteNode(head, pid_int);
    printf("Killed process %d \n", pid_int);
  }
  
}


void func_BGstop(char * str_pid){
  // This function temporarily pauses a process
	int pid_int = atoi(str_pid);
	kill(pid_int, SIGSTOP);
  printf("Paused process %d \n", pid_int);

}


void func_BGstart(char * str_pid){
  // This function resumes a paused process
	int pid_int = atoi(str_pid);
	kill(pid_int, SIGCONT);
  printf("Resumed process %d \n", pid_int);

}


void func_pstat(char * str_pid){
  // This function prints the Comm, State, UTime, STime, RSS, voluntary and unvoluntary ctxt switches
  // For a given pid
  int pid_int = atoi(str_pid);
  if (PifExist(head, pid_int) == 0){
    printf("The process %d either does not exist or was not created by this instance of PMan \n", pid_int);
    return;
  }

  printf("Below is the pstat for pid: %d \n", pid_int);
  // The format from this area came from:
  // https://stackoverflow.com/questions/33266678/how-to-extract-information-from-the-content-of-proc-files-on-linux-using-c
  char stat[1000];
  char status[1000];
  sprintf(stat, "/proc/%d/stat", pid_int); //read as a file
  FILE *f = fopen(stat, "r");

  int unused;
  char comm[1000];
  char state;
  long int rss;
  unsigned long utime;
  unsigned long stime; 
  unsigned long unused_long;          // The travesty below needs to find the 2nd, 3rd, 14th, 15th, 24th
  unsigned long long int unused_vlong;   // Entries in the proc/pid/stat file for comm, state, utime, stime
  
  fscanf(f, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld", &unused_long, comm, &state, &unused, &unused, &unused, &unused, &unused, &unused, &unused_long, &unused_long, &unused_long, &unused_long, &utime, &stime, &unused_long, &unused_long,  &unused_long,  &unused_long,  &unused_long,  &unused_long, &unused_vlong, &unused_long, &rss);
  
  fclose(f);
  // Print code block out to stdout
  printf("comm = %s\n", comm);
  printf("state = %c\n", state);
  printf("utime = %lu \n", utime);
  printf("stime = %lu \n", stime);
  printf("rss = %ld\n", rss);

  // This bit below is for printing the (un)voluntary ctxt switches
  char line[1000];
  sprintf(status, "/proc/%d/status", pid_int);
  FILE *f2 = fopen(status, "r");     
    if(f2 == NULL)
        return 1;
     
    while(fgets(line, sizeof(line), f2)){ 
        if (strstr(line, "voluntary_ctxt_switches:")){ // This strstr will get both
            printf(line);                       // Voluntary and unvoluntary
        }  
    }
    fclose(f2);


}

 
int main(){
  // The main PMan function.
  // Contains commands:
  // func_BG: Start a process in background
  // func_BGlist: View list of bg process
  // func_BGKill: Kill a process
  // func_BGStop: Pauses a process
  // func_BGStart: Resumes a process
  // func_pstat: Lists details about a given process
    char user_input_str[50];
    while (true) {
      printf("Pman: > ");
      fgets(user_input_str, 50, stdin);
      printf("User input: %s \n", user_input_str);
      char * ptr = strtok(user_input_str, " \n");
      if(ptr == NULL){
        continue;
      }
      char * lst[50];
      int index = 0;
      lst[index] = ptr;
      index++;
      while(ptr != NULL){
        ptr = strtok(NULL, " \n");
        lst[index]=ptr;
        index++;
      }
      if (strcmp("bg",lst[0]) == 0){
        func_BG(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        func_BGlist(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        func_BGkill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        func_BGstop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        func_BGstart(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        func_pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
        printf("Bye Bye \n");
        exit(0);
      } else {
        printf("Invalid input\n");
      }
    }

  return 0;
}

