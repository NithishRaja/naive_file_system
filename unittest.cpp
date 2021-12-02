/****
  * File containing unittests.
  *
  */

// Dependencies
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
// Local Dependencies
#include "filesystem.cpp"

// Set namespace
using namespace std;

FileSystem fs;

void test_create_disk(void){
  char file_name[10];
  strcpy(file_name, "test_disk");
  // Test creating a new disk
  CU_ASSERT(fs.create_disk(file_name) == 1);
  CU_ASSERT(access(file_name, F_OK) == 0);
  FILE* fp = fopen(file_name, "r+b");
  fseek(fp, 0, SEEK_END);
  CU_ASSERT(ftell(fp) == DISK_SIZE);
  fclose(fp);
  // Test creating a disk with duplicate name
  CU_ASSERT(fs.create_disk(file_name) == 0);
  // Delete disk
  system("rm -rf test_disk");
}

void test_mount_disk(void){
  char file_name[10];
  strcpy(file_name, "test_disk");
  // Test creating a new disk
  CU_ASSERT(fs.create_disk(file_name) == 1);
  CU_ASSERT(fs.mount_disk(file_name) == 0);
  CU_ASSERT(fs.unmount_disk(file_name) == 0);
  // Delete disk
  system("rm -rf test_disk");
  CU_ASSERT(fs.mount_disk(file_name) == -1);
}

int main(){
  CU_pSuite pSuite = NULL;

  if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
  }

  pSuite = CU_add_suite("Suite_1", NULL, NULL);
  if(NULL == pSuite){
    CU_cleanup_registry();
    return CU_get_error();
  }

  if((NULL == CU_add_test(pSuite, "test creating disk", test_create_disk))
  || (NULL == CU_add_test(pSuite, "test mounting disk", test_mount_disk))){
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_console_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
