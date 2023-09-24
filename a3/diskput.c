/* Jack Dewey
V00972159
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>



int main(int argc, char *argv[]){
    /* 
    
    */
    if(argv[1] == NULL){
        printf("Input a .IMA file");
        exit(1);
    }
    if (argc >2 ){
        printf("Too many arguments");
        exit(1);
    }

    int opened_disk = open(argv[1], O_RDWR);
    if (opened_disk < 0){
        printf("Couldn't open disk image");
        exit(1);
    }


    
    struct stat file_stats;

    fstat(opened_disk, &file_stats);
    char* mmapped = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, opened_disk,0);



    munmap(mmapped, file_stats.st_size);

    close(opened_disk);
    return 0;
}