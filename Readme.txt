V_number: V00972159
Section: A01, T02
Name Jack Dewey

How to compile and run your code:
Save main.c, linked_list.c and Makefile to same directory.
In linux terminal at specified directory, enter 'make'
It creates a program named pman.
Run the program by entering './pman'
You may call the bg function by using a process, for example, named test, by entering:
"bg ./test"
The bglist function is called by "bglist"
The bgstop function is called by "bgstop <pid>"
The bgstart function is called by "bgstart <pid>"
The pstat function is called by "pstat <pid>"

The error handling is minimal at best:

It can handle an incorrect pid given to pstat.

It can handle bglist being called with no active processes.

It can handle a call to bgkill in which the pid does not exist.


It will not kill processes created outside this instance of PMan.

It cannot indicate when a process is killed outside of PMan, there is a vestige of my attempt
to implement a signal function at the top of the file.

The bg function will add a process to the list if called with e.g <test2>,
despite test2 not actually being an active process or real program.

