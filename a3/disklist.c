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



void get_os(char* mmappedped, char* os_name){
     //This function finds the os name from the +3 mod and then assigns the pre
    //allocated malloc memory for os_name to the corresponding OS Name
    
    for (int i=0; i < 8; i++){
        os_name[i] = mmappedped[i+3];
    }
}
void print_date_time(char * directory_entry_startPos, int type_of_file){
    /*
    This was given to us
    */
	if (type_of_file=='D'){
        printf("\n");
        return;
    }
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

int calc_file_size(char* mmappedped, int mod){
    long int first_file_size = ((mmappedped[mod+28] & 0xf0 >> 4) + (mmappedped[mod+28] & 0x0f << 4));
    long int second_file_size = ((mmappedped[mod+29] & 0xf0 >> 4) + (mmappedped[mod+29] & 0x0f << 4)) << 8;
    long int third_file_size = ((mmappedped[mod+30] & 0xf0 >> 4) + (mmappedped[mod+30] & 0x0f << 4)) << 16;
    long int fourth_file_size = ((mmappedped[mod+31] & 0xf0 >> 4) + (mmappedped[mod+31] & 0x0f << 4)) << 24;
    long int total_size = first_file_size+second_file_size+third_file_size+fourth_file_size;
    return total_size;
}

void ascii_main(int a, int b, int c, int d, int e, int f, int g, int h){
     //Copied this from https://stackoverflow.com/questions/18330970/how-to-convert-decimal-value-to-character-in-c-language 
    char bytes[8]={a,b,c,d,e,f,g,h};  
    int i=0;

     //Print The Equivalent Character Values 
    for (i=0; i<8; i++)
    {   
        printf ("%c", bytes[i]);
    }   
}






void recur_print_disklist(char* mmapped, int mod, char* dir_name){
    /*
    This is a recursive function for printing out the list details
    Depending on if its a File or Subdirectory, the function will recurse on subdirectory
    Until reaching the end/no more logical clusters
    */

    int max_mod = 33*512;
    int type_of_file;
    int size_of_file = 0;
    char name_of_file[15];
    char name_of_ext[4];
    int direct_exist = 0;
    int len_file = 0;
    int counter = 1;


    int sub_count = 0;

    
    if(mod == -1){      // Enter here if we are the first call
        mod = 512*19;   // to recursive function
        printf("%s\n", dir_name);
        printf("==================\n");
    }else{          // otherwise we aren't root here
        printf("/%s\n", dir_name);
        printf("==================\n");
    }


   int init_mod = mod;
    while(mmapped[mod] != 0){
        if((mmapped[mod+11] & 0x2) == 0 && (mmapped[mod+11] & 0x8) == 0){
            len_file = 0;
            for (int i=0; i<8; i++){
                if (mmapped[mod+i]==' '){
                    break;
                }
                name_of_file[i] = mmapped[mod+i];
                len_file++;
            }

            if((mmapped[mod+11]&0x10) == 0x10){
                sub_count++;
                type_of_file = 'D';
                direct_exist = 1;
                name_of_ext[0] = 0;
            }else{
                type_of_file = 'F';

                name_of_file[len_file] = '.';
                len_file++;
                
                for (int i=0; i<3; i++){
                    if (mmapped[mod+i+8]==' '){
                        break;
                    }
                    name_of_ext[i] = mmapped[mod+i+8];
                    name_of_file[i+len_file] = mmapped[mod+i+8];
                }
            }

            size_of_file = calc_file_size(mmapped, mod);


            printf("%c ", type_of_file);
            printf("%10d ", size_of_file);
            printf("%20s ", name_of_file);
            print_date_time(mmapped+mod, type_of_file);


        }
        mod += 32;
    }

    if (direct_exist == 0){  // If File, return
        return;
    }

    while(counter <= sub_count){
        mod = init_mod;
        int cur = 1;
        while(mmapped[mod] != 0){
            if((mmapped[11 + mod] & 0x2) == 0 && (mmapped[11 + mod] & 0x8) == 0){

                if((mmapped[mod+11]&0x10) == 0x10){

                        for(int i = 0;i < 8;i++){
                            if(mmapped[mod+i] == ' '){
                                name_of_file[i] = '\0';
                                break;
                            }
                            name_of_file[i] = mmapped[mod+i];
                            }
                        for(int i = 0;i < 3;i++){
                            if(mmapped[mod+i] == ' '){
                                name_of_ext[i] = '\0';
                                break;
                            }
                            name_of_ext[i] = mmapped[mod+i];
                            }
                    if(cur == counter){
                        cur++;
                        break;
                    }
                }
                    
            }
            mod += 32;
        }

        int next_cluster = mmapped[mod+26] + (mmapped[mod+27] << 8);
        recur_print_disklist(mmapped, (31+next_cluster) * 512 + counter*64, name_of_file);
        counter++;
    }
}

int main(int argc, char *argv[]){
    /* 
    This function lists the details of a floppy disk image
    It prints the contents of ROOT as well as the contents of each subdirectory
    The subdirectories do not have a date as per: https://bright.uvic.ca/d2l/le/247651/discussions/threads/678373/View
    Each file has it's name and size shown, with subdirectories having a size of 0
    Each directory other than ROOT is indicated with a /DIRECTORY_NAME
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
    char* init_dir = "ROOT";

    recur_print_disklist(mmapped, -1, init_dir); 


    munmap(mmapped, file_stats.st_size);

    close(opened_disk);
    return 0;
}