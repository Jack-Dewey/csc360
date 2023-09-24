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

 
Node * add_newNode(Node* head, pid_t new_pid, char * new_path){
	// Adds a new node to the LinkedList
	struct Node *newNode;
	newNode = malloc(sizeof(struct Node));
	newNode->pid = new_pid;
	newNode->path = new_path;
	newNode->next = head;
	return(newNode);

}


Node * deleteNode(Node* head, pid_t pid){
	// Deletes a node from the LinkedList
	Node * save_head = head;
	if (head == NULL){
		return head;
	}
	if(head->next == NULL){
		if (head->pid == pid){
			head = head->next;
			head = NULL;
			return NULL;
		}
	} else if(head->pid == pid){
		save_head = head;
		*head = *head->next;
		return save_head;
	} else{
		Node *cur_node = head;
		while (cur_node != NULL){
			if (cur_node->next->pid == pid){
				save_head = cur_node->next;
				cur_node->next = cur_node->next->next;
				
				return save_head;
			}else {cur_node = cur_node->next;
			}
		return save_head;
		}
	
	}
}

void printList(Node *node){
	// Prints a list of all nodes in LinkedList
	int len = 0;
	while (node != NULL){
		printf("%d: ", node->pid);
		printf("%s \n\n", node->path);
		node = node->next;
		len ++;
	}
	printf("Total Background Jobs: %d \n", len);
}


int PifExist(Node *node, pid_t pid){
	// Returns 1 if node exists in LinkedList, 0 otherwise
	while (node != NULL){
		if(node->pid == pid){
			return 1;
		}
		else{
			node = node->next;
		}
	}
  	return 0;
}