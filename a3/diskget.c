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


int search_for(char* mmapped, char* argument){
    /*
    This function looks for a file with the specified name in the root directory
    If found, returns the address of file, otherwise 0;
    */
    int mod = 19*512;
    int max_mod = 33*512;


    while(mod < max_mod){
        int length_name = 0;
        char new_file_name[12];
        for (int i=0; i<8; i++){
            if(mmapped[mod+i] == ' '){
                break;
            } else{
                new_file_name[i] = mmapped[mod+i];
                length_name++;
            }
        }

        if (strncmp(new_file_name,argument, length_name)==0){
            return mod;
        }
        mod +=32;
        
    }
    return 0;
}

int FAT_formula(char* mmapped, int i){
    /*
    This is used because of the 12 vs 8 bit encoding used by FAT systems
    */
    int result = 0;
            if ((i%2)==0){
            int first = mmapped[1+(3*i/2) + 512] & 0x0f;
            int second =  mmapped[(3*i/2) + 512] & 0xff;
            result = (second + (first<<8));
        } else{
            int first =mmapped[(3*i/2) + 512] & 0xf0;
            int second = mmapped[1+(3*i/2) + 512] & 0xff;
            result = (second<<4) + (first>>4);
        }
        return result;
}

void write_file(char* mmapped, int mod, char* argument){
    /*
    This function writes the input file from the disk to the new file
    */
    
    FILE* new_file = fopen(argument, "wb");
    int bytes_copied = 0;
    int counter;

    int flc = mmapped[mod+26] + ((mmapped[mod+27 <<8]) & 0xff);
    int latter_flc = FAT_formula(mmapped, flc);

    long int first_file_size = ((mmapped[mod+28] & 0xf0 >> 4) + (mmapped[mod+28] & 0x0f << 4));
    long int second_file_size = ((mmapped[mod+29] & 0xf0 >> 4) + (mmapped[mod+29] & 0x0f << 4)) << 8;
    long int third_file_size = ((mmapped[mod+30] & 0xf0 >> 4) + (mmapped[mod+30] & 0x0f << 4)) << 16;
    long int fourth_file_size = ((mmapped[mod+31] & 0xf0 >> 4) + (mmapped[mod+31] & 0x0f << 4)) << 24;
    long int total_size = first_file_size+second_file_size+third_file_size+fourth_file_size;
    


    while (bytes_copied<=total_size-512){
        mod = (flc +31)*512;
        for (int i=0; i<512; i++){
        fputc(mmapped[mod+i], new_file);
       }
       bytes_copied+=512;
       counter++;
       flc = latter_flc;
       latter_flc = (FAT_formula(mmapped, flc));

    }

    int leftovers = total_size-(bytes_copied);
    int leftover_counter=0;
    mod = (31+flc) * 512;
    for (int i=0; i<leftovers;i++){
        fputc(mmapped[mod+i], new_file);
        leftover_counter++;
    }
    fclose(new_file);
}


int main(int argc, char *argv[]){
    /* 
    This function when called finds the specified file in the root directory and then copies it to
    current linux directory
    I don't know why it corrupts all PDFs and JPG, it has the correct byte number. Pls let me know I'm begging you literally crying.
    */
    if(argv[1] == NULL){
        printf("Input a .IMA file");
        exit(1);
    }
    if (argc >3 ){
        printf("Too many arguments");
        exit(1);
    }
    int opened_disk = open(argv[1], O_RDWR);
    if (opened_disk < 0){
        printf("couldn't open disk image");
        exit(1);
    }


    struct stat file_stats;
    fstat(opened_disk, &file_stats);
    char* mmapped = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, opened_disk,0);
    char *argument = argv[2];
    int search_result = 0;
    search_result = search_for(mmapped, argument);
    if(search_result>0){
        write_file(mmapped, search_result, argument);
    }else{
        printf("File not found.\n");
    }

    munmap(mmapped, file_stats.st_size);

    close(opened_disk);
    return 0;
}