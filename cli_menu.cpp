/****
  * File containing code for a CLI menu for file system.
  *
  */

// Dependencies
#include <iostream>
// Local Dependencies
#include "filesystem.cpp"

// Set namespace
using namespace std;

// Declare global variables
FileSystem fs;

/*
 * Function to get user input and perform actions on files
 */
/*
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
*/

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
      int res = fs.create_disk(disk_name);
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
      int res = fs.mount_disk(disk_name);
      if(res == 0){
        cout<<"Disk mounted\n";
      }else{
        cout<<"Failed to mount disk\n";
      }
      // file_REPL();
      res = fs.mount_disk(disk_name);
      if(res == 0){
        cout<<"Disk unmounted\n";
      }else{
        cout<<"Failed to unmount disk\n";
      }
    }else if(inp==3){
      cout<<"Exit\n";
      break;
    }else{
      cout<<"Not recognised\n";
    }
  }
}

int main(){
  disk_REPL();
  return 0;
}
