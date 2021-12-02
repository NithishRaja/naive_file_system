# Naive File System

## Running code

* To start the menu interface for filesystem, run `g++ cli_menu.cpp -o cli_menu.out && ./cli_menu.out`

## Editing code

* The main logic is present inside `filesystem.cpp`

## Testing code

* To run the unittest, run `g++ unittest.cpp -o unittest.out -lcunit`

## Features

* Create, mount and delete operations can be performed on disk
* Once a disk is mounted, files can be opened in read, write and append mode
* Filesystem has a CLI through which users can interact
