CC = gcc
CFLAGS = -Wall -Wextra -g

# ===== FILE SYSTEM CLI =====
FS_SRC = \
main.c \
block_layer/disk.c \
block_layer/bitmap.c \
block_layer/super_block.c \
block_layer/group_desc.c \
block_layer/inode.c \
block_layer/inode_table.c \
directory/directory.c \
directory/path.c \
file_operator/file.c

FS_OBJ = $(FS_SRC:.c=.o)

# ===== MKFS =====
MKFS_SRC = \
mkfs.c \
block_layer/disk.c \
block_layer/bitmap.c \
block_layer/super_block.c \
block_layer/group_desc.c \
block_layer/inode.c \
block_layer/inode_table.c

MKFS_OBJ = $(MKFS_SRC:.c=.o)

all: fs mkfs

fs: $(FS_OBJ)
	$(CC) $(CFLAGS) -o fs $(FS_OBJ)

mkfs: $(MKFS_OBJ)
	$(CC) $(CFLAGS) -o mkfs $(MKFS_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f fs mkfs *.o block_layer/*.o directory/*.o file_operator/*.o
