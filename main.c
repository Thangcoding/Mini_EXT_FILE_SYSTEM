#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "block_layer/disk.h"
#include "block_layer/inode.h"

#include "directory/directory.h"
#include "directory/path.h"

#include "file_operator/file.h"

#define MAX_CMD_LEN 512
#define MAX_ARG_LEN 256
#define MAX_PATH_LEN 1024
#define ROOT_INODE  1


/* ===== CLI PATH HELPER ===== */
static void path_join(char *cwd, const char *arg)
{
    if (arg[0] == '/') {
        strcpy(cwd, arg);
        return;
    }

    if (strcmp(arg, ".") == 0)
        return;

    if (strcmp(arg, "..") == 0) {
        if (strcmp(cwd, "/") == 0)
            return;

        char *last = strrchr(cwd, '/');
        if (last && last != cwd)
            *last = '\0';
        else
            strcpy(cwd, "/");
        return;
    }

    if (strcmp(cwd, "/") != 0)
        strcat(cwd, "/");

    strcat(cwd, arg);
}


int main()
{
    char cmdline[MAX_CMD_LEN];
    char *cmd, *arg;

    uint32_t cwd_inode = ROOT_INODE;
    char cwd_path[MAX_PATH_LEN] = "/";

    if (disk_open("disk.img") < 0) {
        printf("Failed to open disk.img\n");
        return 1;
    }

    printf("Mini EXT File System CLI\n");
    printf("Type 'exit' to quit\n");

    while (1) {
        printf("mini-ext:%s$ ", cwd_path);
        fflush(stdout);

        if (!fgets(cmdline, sizeof(cmdline), stdin))
            break;

        cmdline[strcspn(cmdline, "\n")] = 0;
        if (strlen(cmdline) == 0)
            continue;

        cmd = strtok(cmdline, " ");
        arg = strtok(NULL, " ");

        /* ===== EXIT ===== */
        if (strcmp(cmd, "exit") == 0)
            break;

        /* ===== LS ===== */
        else if (strcmp(cmd, "ls") == 0) {
            directory_list(cwd_inode);
        }

        /* ===== CD ===== */
        else if (strcmp(cmd, "cd") == 0) {
            if (!arg) {
                printf("cd: missing operand\n");
                continue;
            }

            uint32_t inode;
            inode_t ino;

            if (path_resolve(arg, cwd_inode, &inode) < 0 ||
                inode_read(inode, &ino) < 0 ||
                ino.file_type != INODE_DIR) {
                printf("cd: not a directory\n");
                continue;
            }

            cwd_inode = inode;
            path_join(cwd_path, arg);
        }

        /* ===== PWD ===== */
        else if (strcmp(cmd, "pwd") == 0) {
            printf("%s\n", cwd_path);
        }

        /* ===== MKDIR ===== */
        else if (strcmp(cmd, "mkdir") == 0) {
            if (!arg) {
                printf("mkdir: missing operand\n");
                continue;
            }

            uint32_t parent;
            char name[MAX_ARG_LEN];

            if (path_parent(arg, cwd_inode, &parent, name) < 0 ||
                directory_create(parent, name) < 0) {
                printf("mkdir: failed\n");
            }
        }

        /* ===== CREATE FILE ===== */
        else if (strcmp(cmd, "cf") == 0) {
            if (!arg) {
                printf("cf: missing operand\n");
                continue;
            }

            uint32_t parent;
            char name[MAX_ARG_LEN];

            if (path_parent(arg, cwd_inode, &parent, name) < 0 ||
                file_create(parent, name) < 0) {
                printf("cf: failed\n");
            }
        }

        /* ===== WRITE FILE ===== */
        else if (strcmp(cmd, "wf") == 0) {
            char *flag = arg;
            char *path = strtok(NULL, " ");

            int mode = FILE_WRITE_OVERWRITE;

            if (!flag) {
                printf("wf: missing operand\n");
                continue;
            }

            /* wf file  → overwrite */
            if (flag[0] != '-') {
                path = flag;
            }
            else {
                if (!path) {
                    printf("wf: missing file operand\n");
                    continue;
                }

                if (strcmp(flag, "-a") == 0)
                    mode = FILE_WRITE_APPEND;
                else if (strcmp(flag, "-o") == 0)
                    mode = FILE_WRITE_OVERWRITE;
                else {
                    printf("wf: unknown option %s\n", flag);
                    continue;
                }
            }

            uint32_t inode;
            inode_t ino;

            if (path_resolve(path, cwd_inode, &inode) < 0 ||
                inode_read(inode, &ino) < 0 ||
                ino.file_type != INODE_FILE) {
                printf("wf: not a file\n");
                continue;
            }

            char buffer[1024];
            printf("Enter content: ");
            fgets(buffer, sizeof(buffer), stdin);

            file_write(inode,
                       buffer,
                       strlen(buffer),
                       mode);
        }

        /* ===== READ FILE ===== */
        else if (strcmp(cmd, "rf") == 0) {
            if (!arg) {
                printf("rf: missing operand\n");
                continue;
            }

            uint32_t inode;
            inode_t ino;

            if (path_resolve(arg, cwd_inode, &inode) < 0 ||
                inode_read(inode, &ino) < 0 ||
                ino.file_type != INODE_FILE) {
                printf("rf: not a file\n");
                continue;
            }

            char buffer[1024] = {0};
            int bytes = file_read(inode, buffer, sizeof(buffer));

            if (bytes > 0)
                printf("%s\n", buffer);
        }

        /* ===== REMOVE ===== */
        else if (strcmp(cmd, "rm") == 0) {
            if (!arg) {
                printf("rm: missing operand\n");
                continue;
            }

            uint32_t parent, inode;
            char name[MAX_ARG_LEN];
            inode_t ino;

            if (path_parent(arg, cwd_inode, &parent, name) < 0 ||
                path_resolve(arg, cwd_inode, &inode) < 0 ||
                inode_read(inode, &ino) < 0) {
                printf("rm: invalid path\n");
                continue;
            }

            if (ino.file_type == INODE_FILE)
                file_delete(parent, name);
            else if (ino.file_type == INODE_DIR)
                directory_delete(parent, name);
            else
                printf("rm: unknown type\n");
        }

        else {
            printf("Unknown command: %s\n", cmd);
        }
    }

    disk_close();
    return 0;
}
