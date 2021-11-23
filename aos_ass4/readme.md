# Linux File System

As the program starts the following menu is displayed

1. Create Disk: It is used to create disk of 500MB.
2. Mount Disk: It is used to mount disk.
3. Exit: It exits from program

when the disk is mounted the following menu is displayed
1. Create file: Creates a file in the system.
2. Open file: It takes file name as input and opens an existing file if its exists, allows to choose between read, write and append modes for it and prints the assigned file descriptor along with the mode it is opened in.
3. Read file: File descriptor is taken as input of a file opened in read mode and all its contents are printed on screen
4. Write file: File descriptor is taken as input of a file opened in write mode and then user is prompted to enter the text that should be written into the file. If file already has data in it, it is truncated and overwritten. User can enter `:q` in a new line to end text entry.
5. Append file:  File descriptor is taken as input  of a file opened in append mode, user is prompted to enter the text that should be appended to the end of the file. User can enter `:q` in a new line to end text entry.
6. Close file: Closes file corresponding to the given descriptor
7. Delete file: Filename is taken as input and it is deleted. If file is open, it cannot be deleted.
8. List of files: Lists all files that exist in disk which is mounted along with their inode numbers.
9. List of opened files: Lists all files that are currently open along with their mode and file descriptors.
10. Unmount: Unmounts disk
