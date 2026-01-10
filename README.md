# Mini-EXT File System

## 1. Introduction

Mini-EXT is a simplified filesystem inspired by EXT4, implemented for educational purposes in an Operating Systems course.  
The filesystem operates on a virtual disk image (`disk.img`), supports basic file and directory operations, provides a command-line interface (CLI), and includes a performance evaluation compared with EXT4.

---

## 2. Features

- Virtual block device abstraction
- Superblock and group descriptor
- Block bitmap and inode bitmap
- Inode-based file structure
- Directory entries with `.` and `..`
- Absolute and relative path resolution
- File operations:
  - Create
  - Read
  - Write (overwrite `-o`, append `-a`)
  - Delete
- Directory operations:
  - Create
  - List
  - Delete
- Filesystem formatter (`mkfs`)
- Performance evaluation against EXT4

---

## 3. Project Structure

Project_File_System/
    └── src/
    ├── main.c # CLI
    ├── mkfs.c # Filesystem formatter
    ├── eval.sh # Evaluation script
    │
    ├── block_layer/
    │ ├── disk.c / disk.h
    │ ├── bitmap.c / bitmap.h
    │ ├── super_block.c / super_block.h
    │ ├── group_desc.c / group_desc.h
    │ ├── inode.c / inode.h
    │ └── inode_table.c / inode_table.h  
    │
    ├── directory/
    │ ├── directory.c / directory.h
    │ ├── path.c / path.h
    │ └── dir_entry.h
    │
    └── file_operator/
    ├── file.c / file.h

---

## 4. Build Instructions


---

## 4. Build Instructions

Compile the project:

```bash
make

./mkfs
./fs


## 5. CLI Commands
Command	      Description
ls	               List directory contents
cd <path>	       Change directory
pwd	Print          current working directory path
mkdir <path>       Create a directory
cf <path>          Create a file
wf -o <path>	   Write file (overwrite)
wf -a <path>	   Write file (append)
rf <path>	       Read file
rm <path>	       Remove file or directory
exit	           Exit CLI
