/* Jack Dewey
V00972159
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void get_os(char* mmappped, char* os_name){
    /* This function finds the os name from the +3 offset and then assigns the pre
    allocated malloc memory for os_name to the corresponding OS Name
    */
    for (int i=0; i < 8; i++){
        os_name[i] = mmappped[i+3];
    }
}

int calculate_free_size(char* mmappped, int total_size){
    /*Calculates and returns the amount of free size on the disk
    by determining the number of sectors that are free and then multiplying
    that number by the amount of bytes per sector
     */
    int free_sectors = 0;
    int loop_num = total_size/512;

    for (int i=2; i < loop_num-31; i++ ){
        int result;
        if ((i%2)==0){
            int first = mmappped[1+(3*i/2) + 512] & 0x0f;
            int second =  mmappped[(3*i/2) + 512] & 0xff;
            result = (second + (first<<8));
        } else{
            int first =mmappped[(3*i/2) + 512] & 0xf0;
            int second = mmappped[1+(3*i/2) + 512] & 0xff;
            result = (second<<4) + (first>>4);
        }
        if(result==0){
            free_sectors++;
        }
    }
    int free_size = free_sectors*512;
    return free_size;
}

void print_date_time(char * directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(directory_entry_startPos + 14);
	date = *(unsigned short *)(directory_entry_startPos + 16);
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	
	printf("%d-%02d-%02d ", year, month, day);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d\n", hours, minutes);
	
	return;	
}

int num_files(char* mmappped){ 
    /* Returns the number of files in an image
    Takes the maximum number of sectors (14*16 = 224)
    and then subtract the number of directories that fulfill file conditions
    */
    int ret_val = 0 ;
    int mod = 0x2600;
    int mod_max = 33*512;
    //int count = 0;

    while(mod<mod_max){
        if (mmappped[mod]== 0 ||mmappped[mod]==0xe5){
            ret_val++;
        }
        else if ((mmappped[mod+11] & 0x10) == 0x01){
            ret_val++;
        }
        else if (mmappped[mod+26]==0 ||mmappped[mod+26]==0x01){
            ret_val++;
        }
        else if ((mmappped[mod+11]==0x0f)){
            ret_val++;
        }

        mod+=32;
    }
    return 224-ret_val;
}

void ascii_main(int a, int b, int c, int d, int e, int f, int g, int h){
    char bytes[8]={a,b,c,d,e,f,g,h};  
    int i=0;

    /* Print The Equivalent Character Values */
    for (i=0; i<8; i++)
    {   
        printf ("%c", bytes[i]);
    }   
}



char* label(char* mmappped, char* label_name){
    /* This function determines and returns the label of a disk
    by visiting the +0 - +11 offset after finding a 0x08 match
    */
    int mod = 19*512;
    int mod_max = 32*512;
    while(1){
        if(mmappped[mod+11]==0x08){
            for(int i=0; i<11;i++){
                label_name[i] = mmappped[mod+i];
            }
            break;
        }
        mod+=32;
        if (mod>mod_max){
            break;
        }
    }

    return label_name; 
}

int main(int argc, char *argv[]){
    /* 
    This function opens and then prints the required details about a floppy disk image
    The details in order are 
    OS NAME
    Label
    Total Size of Disk
    Free Size of Disk
    Number of files on Disk
    Number of FAT Copies
    Sectors per FAT
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
        printf("couldn't open disk image");
        exit(1);
    }

    char* os_name = malloc(sizeof(char));
    char* label_name = malloc(sizeof(char));
    struct stat file_stats;
    fstat(opened_disk, &file_stats);
    char* mmappped = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, opened_disk,0);


    get_os(mmappped, os_name);
    label_name = label(mmappped, label_name);
    int file_size = file_stats.st_size;
    int free_size = calculate_free_size(mmappped, file_size);
    int num_files_val = num_files(mmappped);
    
    printf("OS Name: %s \n", os_name);
    printf("Label of the disk: %s \n", label_name);

    //int file_size = file_stats.st_size;
    printf("Total size of the disk: %d bytes \n", file_size);
    printf("Free size of the disk: %d bytes \n", free_size);

    printf("============ \n");
    printf("Number of files on disk: %d \n", num_files_val);
    printf("============\n");

    int num_fats = mmappped[16];
    printf("Number of FAT copies: %d \n", num_fats);
    int num_sect = (mmappped[23]<<8)+mmappped[22];
    printf("Sectors per FAT: %d \n", num_sect);


    munmap(mmappped, file_stats.st_size);
    free(os_name);
    free(label_name);
    close(opened_disk);
    return 0;
}