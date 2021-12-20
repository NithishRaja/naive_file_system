# Naive File System

A simple file system that manages multiple disks and supports CRUD on text files

## Running code

* To start the menu interface for filesystem, run `g++ cli_menu.cpp -o cli_menu.out && ./cli_menu.out`

## Editing code

* The main logic is present inside `filesystem.cpp`

## Testing code

* To run the unittest, run `g++ unittest.cpp -o unittest.out -lcunit`

## Features

* Create, mount and delete operations can be performed on disk
* Once a disk is mounted, files can be opened in read(1), write(2) and append(3) mode
* Filesystem has a CLI through which users can interact
