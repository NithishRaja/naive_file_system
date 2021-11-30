/****
  * File containing code to implement a naive filesystem
  *
  */

// Dependencies
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <utility>

// Set namespace
using namespace std;

#define DISK_SIZE 500*1024*1024
#define BLOCK_SIZE 4*1024
#define FILE_NAME_SIZE 100
#define SUPER_START 1
#define SUPER_END 8000
#define INODE_START 8001
#define INODE_END 28000
#define BLOCK_START 28001
#define BLOCK_END 128000


struct file_info {
  char file_name[FILE_NAME_SIZE];
  int inode_pos;
};

struct open_file_info {
  char file_name[FILE_NAME_SIZE];
  int inode_pos;
  int fd;
  int mode;
  int write_status;
};

struct inode_data {
  int block_pos;
  int block_filled;
};

// Declare global variables
FILE *fp;
vector<struct file_info> file_list;
vector<struct open_file_info> open_file_list;
int file_descriptor_count;

/*
 * Function to create an empty disk.
 */
int create_disk(char* disk_name){
  if(access(disk_name, F_OK) == 0){
    return 0;
  }
  // string disk_name = "disk_"+to_string(disk_count);
  int fd = open(disk_name, O_CREAT, 0666);
  close(fd);
  FILE *fp = fopen(disk_name, "r+b");
  if(fp == NULL){
    return -1;
  }
  fseek(fp, DISK_SIZE-1, 0);
  fputc('\0', fp);
  fclose(fp);

  return 1;
}

int get_empty_inode(){
  int pos = -1;
  for(int i=INODE_START;i<=INODE_END;++i){
    fseek(fp, (i-1)*BLOCK_SIZE, 0);
    int check;
    fread(&check, sizeof(check), 1, fp);
    if(check == 0){
      pos = i;
      break;
    }
  }

  return pos;
}

int get_empty_block(){
  int pos = -1;
  for(int i=BLOCK_START;i<=BLOCK_END;++i){
    fseek(fp, (i-1)*BLOCK_SIZE, 0);
    int check;
    fread(&check, sizeof(check), 1, fp);
    if(check == 0){
      pos = i;
      break;
    }
  }

  return pos;
}

/*
 * Function to read super block and get list of files.
 */
void get_files_in_disk(){
  fseek(fp, 0, 0);
  int file_count;
  fread(&file_count, sizeof(file_count), 1, fp);
  cout<<"no of files: "<<file_count<<endl;
  for(int i=0;i<file_count;++i){
    struct file_info temp;
    fread(&temp, sizeof(temp), 1, fp);
    file_list.push_back(temp);
  }
}

/*
 * Function to write filenames and corresponding inode position to super block of disk.
 */
void update_super_block(){
  fseek(fp, 0, 0);
  int count = file_list.size();
  fwrite(&count, sizeof(count), 1, fp);
  for(int i=0;i<file_list.size();++i){
    fwrite(&file_list[i], sizeof(file_list[i]), 1, fp);
  }
}

/*
 * Function to create file with given filename on disk.
 * Parameters:
 * file_name -- char array
 */
int add_file_to_disk(char* file_name){
  // Check if file exists
  for(int i=0;i<file_list.size();++i){
    if(strcmp(file_list[i].file_name, file_name) == 0){
      return 0;
    }
  }
  // Initialise struct to hold file info
  struct file_info temp;
  strcpy(temp.file_name, file_name);
  // Get memory for file
  int inode_pos = get_empty_inode();
  int block_pos = get_empty_block();

  temp.inode_pos = inode_pos;
  // Check if memory was obtained
  if(inode_pos < 0 || block_pos < 0){
    return -1;
  }

  // Add placeholder on block
  fseek(fp, (block_pos-1)*BLOCK_SIZE, 0);
  int placeholder = 1;
  fwrite(&placeholder, sizeof(placeholder), 1, fp);
  // Write block info to inode
  int block_count = 1;
  struct inode_data data;
  data.block_pos = block_pos;
  data.block_filled = 0;
  fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
  fwrite(&block_count, sizeof(block_count), 1, fp);
  fwrite(&data, sizeof(data), 1, fp);

  // cout<<"file name: "<<file_name<<" inode: "<<inode_pos<<" block: "<<block_pos<<endl;

  // Add file to file list
  file_list.push_back(temp);
  // Write data to super block
  update_super_block();
  return 1;
}

/*
 * Function to delete given file from disk.
 * Parameters:
 * file_name -- char array
 */
int remove_file_from_disk(char* file_name){
  // Initialise flag
  int flag = 0;
  int empty = 0;
  // Check if file exists
  for(int i=0;i<file_list.size();++i){
    if(strcmp(file_list[i].file_name, file_name) == 0){
      // Get inode position
      int inode_pos = file_list[i].inode_pos;
      // Get block positions
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      int block_count;
      fread(&block_count, sizeof(block_count), 1, fp);
      // Get list of blocks
      vector<struct inode_data> data_list;
      for(int j=0;j<block_count;++j){
        struct inode_data data;
        fread(&data, sizeof(data), 1, fp);
        data_list.push_back(data);
      }

      // Free blocks
      for(int j=0;j<data_list.size();++j){
        fseek(fp, (data_list[j].block_pos-1)*BLOCK_SIZE, 0);
        fwrite(&empty, sizeof(empty), 1, fp);
      }
      // Free inode
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      fwrite(&empty, sizeof(empty), 1, fp);

      file_list.erase(file_list.begin()+i);
      flag = 1;
      break;
    }
  }

  update_super_block();
  return flag;
}

/*
 * Function to open file and assign a file descriptor.
 * Parameters:
 * file_name -- char array
 * mode -- int
 */
int open_file(char* file_name, int mode){
  int fd = -1;
  if(mode != 1 && mode != 2 && mode != 3){
    return fd;
  }
  for(int i=0;i<file_list.size();++i){
    if(strcmp(file_list[i].file_name, file_name) == 0){
      struct open_file_info temp;
      strcpy(temp.file_name, file_name);
      temp.inode_pos = file_list[i].inode_pos;
      temp.mode = mode;
      temp.write_status = 0;
      temp.fd = file_descriptor_count;
      open_file_list.push_back(temp);
      fd = file_descriptor_count;
      ++file_descriptor_count;
      break;
    }
  }
  return fd;
}

/*
 * Function to check if file was opened with given mode.
 */
int check_file_mode(int fd, int mode){
  int flag = 0;
  for(int i=0;i<open_file_list.size();++i){
    if(open_file_list[i].fd == fd){
      if(open_file_list[i].mode == mode){
        flag = 1;
      }
      break;
    }
  }
  return flag;
}

/*
 * Function to print contents of file.
 * It is assumed that all checks (file exists and opened in read mode) have been done.
 */
void display_file(int fd){
  for(int i=0;i<open_file_list.size();++i){
    if(open_file_list[i].fd == fd){
      int inode_pos = open_file_list[i].inode_pos;
      // Read from inode
      vector<struct inode_data> inode_data_list;
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      int block_count;
      fread(&block_count, sizeof(block_count), 1, fp);
      for(int j=0;j<block_count;++j){
        struct inode_data data;
        fread(&data, sizeof(data), 1, fp);
        inode_data_list.push_back(data);
      }
      // Read from block
      for(int j=0;j<inode_data_list.size();++j){
        int block_pos = inode_data_list[j].block_pos;
        int block_filled = inode_data_list[j].block_filled;
        fseek(fp, (block_pos-1)*BLOCK_SIZE, 0);
        // Print contents of block
        for(int k=0;k<block_filled;++k){
          char ch;
          fread(&ch, sizeof(ch), 1, fp);
          cout<<ch;
        }
      }
      cout<<endl;
      break;
    }
  }
}

/*
 * Function to write a character to a file.
 * It is assumed that all checks (file exists and opened in write mode) have been done.
 */
void write_to_file(int fd, char* buffer, int buffer_size){
  for(int i=0;i<open_file_list.size();++i){
    if(open_file_list[i].fd == fd){
      int inode_pos = open_file_list[i].inode_pos;
      // Read from inode
      vector<struct inode_data> inode_data_list;
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      int block_count;
      fread(&block_count, sizeof(block_count), 1, fp);
      for(int j=0;j<block_count;++j){
        struct inode_data data;
        fread(&data, sizeof(data), 1, fp);
        inode_data_list.push_back(data);
      }

      // Reset data in previous blocks
      for(int j=0;j<inode_data_list.size();++j){
        inode_data_list[j].block_filled = 0;
      }

      // Write buffer to file
      int max_block_count = inode_data_list.size();
      int current_block_counter = 0;
      int current_block_pos = inode_data_list[0].block_pos;
      int current_block_filled = inode_data_list[0].block_filled;
      fseek(fp, (current_block_pos-1)*BLOCK_SIZE, 0);
      for(int j=0;j<buffer_size;++j){
        // Check if current block is filled
        if(current_block_filled == BLOCK_SIZE){
          // If next block exists, move to next block
          if(current_block_counter < max_block_count-1){
            // Update current block
            ++current_block_counter;
            current_block_pos = inode_data_list[current_block_counter].block_pos;
            current_block_filled = inode_data_list[current_block_counter].block_filled;
            fseek(fp, (current_block_pos-1)*BLOCK_SIZE, 0);
          }else{
            // Add new block
            int res = get_empty_block();
            if(res < 0){
              break;
            }else{
              struct inode_data temp;
              temp.block_pos = res;
              temp.block_filled = 0;
              inode_data_list.push_back(temp);
              // Update current block
              max_block_count = inode_data_list.size();
              ++current_block_counter;
              current_block_pos = inode_data_list[current_block_counter].block_pos;
              current_block_filled = inode_data_list[current_block_counter].block_filled;
              fseek(fp, (current_block_pos-1)*BLOCK_SIZE, 0);
            }
          }
        }
        fwrite(&buffer[j], sizeof(buffer[j]), 1, fp);
        ++inode_data_list[current_block_counter].block_filled;
      }
      // Update inode data
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      ++current_block_counter;
      fwrite(&current_block_counter, sizeof(current_block_counter), 1, fp);
      for(int j=0;j<current_block_counter;++j){
        fwrite(&inode_data_list[j], sizeof(inode_data_list[j]), 1, fp);
      }
      break;
    }
  }
}

/*
 * Function to append a character to a file.
 * It is assumed that all checks (file exists and opened in append mode) have been done.
 */
void append_to_file(int fd, char* buffer, int buffer_size){
  for(int i=0;i<open_file_list.size();++i){
    if(open_file_list[i].fd == fd){
      int inode_pos = open_file_list[i].inode_pos;
      // Read from inode
      vector<struct inode_data> inode_data_list;
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      int block_count;
      fread(&block_count, sizeof(block_count), 1, fp);
      for(int j=0;j<block_count;++j){
        struct inode_data data;
        fread(&data, sizeof(data), 1, fp);
        inode_data_list.push_back(data);
      }

      // Write buffer to file
      int max_block_count = inode_data_list.size();
      int current_block_counter = max_block_count-1;
      int current_block_pos = inode_data_list[current_block_counter].block_pos;
      int current_block_filled = inode_data_list[current_block_counter].block_filled;
      fseek(fp, (current_block_pos-1)*BLOCK_SIZE+current_block_filled, 0);
      for(int j=0;j<buffer_size;++j){
        // Check if current block is filled
        if(current_block_filled == BLOCK_SIZE){
          // If next block exists, move to next block
          if(current_block_counter < max_block_count-1){
            // Update current block
            ++current_block_counter;
            current_block_pos = inode_data_list[current_block_counter].block_pos;
            current_block_filled = inode_data_list[current_block_counter].block_filled;
            fseek(fp, (current_block_pos-1)*BLOCK_SIZE, 0);
          }else{
            // Add new block
            int res = get_empty_block();
            if(res < 0){
              break;
            }else{
              struct inode_data temp;
              temp.block_pos = res;
              temp.block_filled = 0;
              inode_data_list.push_back(temp);
              // Update current block
              max_block_count = inode_data_list.size();
              ++current_block_counter;
              current_block_pos = inode_data_list[current_block_counter].block_pos;
              current_block_filled = inode_data_list[current_block_counter].block_filled;
              fseek(fp, (current_block_pos-1)*BLOCK_SIZE, 0);
            }
          }
        }
        fwrite(&buffer[j], sizeof(buffer[j]), 1, fp);
        ++inode_data_list[current_block_counter].block_filled;
      }

      // Update inode data
      fseek(fp, (inode_pos-1)*BLOCK_SIZE, 0);
      ++current_block_counter;
      fwrite(&current_block_counter, sizeof(current_block_counter), 1, fp);
      for(int j=0;j<current_block_counter;++j){
        fwrite(&inode_data_list[j], sizeof(inode_data_list[j]), 1, fp);
      }
      break;
    }
  }
}

/*
 * Function to close file corresponding to file descriptor.
 * Parameters:
 * fd -- int
 */
int close_file(int fd){
  int flag = 0;
  for(int i=0;i<open_file_list.size();++i){
    if(open_file_list[i].fd == fd){
      open_file_list.erase(open_file_list.begin()+i);
      flag = 1;
      break;
    }
  }
  return flag;
}

/*
 * Function to get user input and perform actions on files
 */
void file_REPL(){
  // Display menu
  cout<<"1) Create file\n2) Open file\n3) Read file\n4) Write file\n5) Append file\n6) Close file\n7) Delete file\n8) List all files\n9) List opened files\n10) Unmount\n";
  while(1){
    int inp;
    cin>>inp;
    if(inp == 1){ // Create file
      char file_name[FILE_NAME_SIZE];
      cout<<"Enter filename: ";
      cin>>file_name;
      int res = add_file_to_disk(file_name);
      if(res == 0){
        cout<<"File exists\n";
      }else if(res == -1){
        cout<<"Out of memory\n";
      }else{
        cout<<"File created\n";
      }
    }else if(inp == 2){ // Open file
      char file_name[FILE_NAME_SIZE];
      cout<<"Enter filename: ";
      cin>>file_name;
      int mode;
      cout<<"1) Read mode\n2) Write mode\n3) Append mode\n";
      cin>>mode;
      int res = open_file(file_name, mode);
      if(res >= 0){
        cout<<"File opened with file descriptor: "<<res<<endl;
      }else{
        cout<<"Failed to open file\n";
      }
    }else if(inp == 3){ // Read file
      int fd;
      cout<<"Enter file descriptor: ";
      cin>>fd;
      // Check if file was opened in read mode
      int mode_check = check_file_mode(fd, 1);
      if(mode_check){
        display_file(fd);
      }else{
        cout<<"File was not opened in read mode\n";
      }
    }else if(inp == 4){ // Write file
      int fd;
      cout<<"Enter file descriptor: ";
      cin>>fd;
      // Check if file was opened in read mode
      int mode_check = check_file_mode(fd, 2);
      if(mode_check){
        // Read in string to write to file
        char buffer[BLOCK_SIZE];
        int buffer_size = read(STDIN_FILENO, &buffer, BLOCK_SIZE);
        if((int)buffer[buffer_size-1] < 32){
          buffer[buffer_size-1] = '\0';
          --buffer_size;
        }
        int first_write = 1;
        for(int i=0;i<open_file_list.size();++i){
          if(open_file_list[i].fd == fd){
            if(open_file_list[i].write_status == 1){
              first_write = 0;
            }else{
              open_file_list[i].write_status = 1;
            }
            break;
          }
        }
        if(first_write){
          write_to_file(fd, buffer, buffer_size);
        }else{
          append_to_file(fd, buffer, buffer_size);
        }
        cout<<"Wrote file\n";
      }else{
        cout<<"File was not opened in write mode\n";
      }
    }else if(inp == 5){ // Append file
      int fd;
      cout<<"Enter file descriptor: ";
      cin>>fd;
      // Check if file was opened in read mode
      int mode_check = check_file_mode(fd, 3);
      if(mode_check){
        // Read in string to write to file
        char buffer[BLOCK_SIZE];
        int buffer_size = read(STDIN_FILENO, &buffer, BLOCK_SIZE);
        if((int)buffer[buffer_size-1] < 32){
          buffer[buffer_size-1] = '\0';
          --buffer_size;
        }
        append_to_file(fd, buffer, buffer_size);
        cout<<"Appended to file\n";
      }else{
        cout<<"File was not opened in append mode\n";
      }
    }else if(inp == 6){ // Close file
      int fd;
      cout<<"Enter file descriptor: ";
      cin>>fd;
      int res = close_file(fd);
      if(res){
        cout<<"File closed\n";
      }else{
        cout<<"No file open with given file descriptor\n";
      }
    }else if(inp == 7){ // Delete file
      char file_name[FILE_NAME_SIZE];
      cout<<"Enter filename: ";
      cin>>file_name;
      int res = remove_file_from_disk(file_name);
      if(res){
        cout<<"File deleted\n";
      }else{
        cout<<"No such file\n";
      }
    }else if(inp == 8){ // Display all files
      for(int i=0;i<file_list.size();++i){
        cout<<file_list[i].file_name<<" "<<file_list[i].inode_pos<<endl;
      }
    }else if(inp == 9){ // Display open files
      for(int i=0;i<open_file_list.size();++i){
        cout<<open_file_list[i].file_name<<" fd: "<<open_file_list[i].fd<<" mode: "<<open_file_list[i].mode<<endl;
      }
    }else if(inp == 10){
      open_file_list.clear();
      file_descriptor_count = 0;
      file_list.clear();
      cout<<"Umount successful\n";
      break;
    }else{
      cout<<"Not recognised\n";
    }
  }
}

/*
 * Function to get user input and perform actions on disks
 */
void disk_REPL(){
  // Display menu
  cout<<"1) Create disk\n2) Mount disk\n3) Exit\n";
  // Start REPL
  while(1){
    int inp;
    cin>>inp;
    if(inp==1){
      char disk_name[FILE_NAME_SIZE];
      cout<<"Enter disk name: ";
      cin>>disk_name;
      int res = create_disk(disk_name);
      if(res == 0){
        cout<<"Disk exists\n";
      }else if(res == 1){
        cout<<"Created disk\n";
      }else{
        cout<<"Failed to create disk\n";
      }
    }else if(inp==2){
      // Get disk name to open
      char disk_name[FILE_NAME_SIZE];
      cout<<"Enter disk name: ";
      cin>>disk_name;
      // Open corresponding file
      fp = fopen(disk_name, "r+b");
      // Check if file was opened successfully
      if(fp == NULL){
        cout<<"Failed to mount disk\n";
        continue;
      }
      get_files_in_disk();
      cout<<"Disk mounted\n";
      file_REPL();
      fclose(fp);
      cout<<"Disk unmounted\n";
      cout<<"1) Create disk\n2) Mount disk\n3) Exit\n";
    }else if(inp==3){
      cout<<"Exit\n";
      break;
    }else{
      cout<<"Not recognised\n";
    }
  }
}

int main(){
  file_descriptor_count = 0;
  disk_REPL();
  return 0;
}
