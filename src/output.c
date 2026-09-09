#include "output.h"
#include <stdio.h>

void print_process_info(const ProcessInfo *info)
{
    printf("Process information\n");
    printf("PID: %u\n", info->pid);
    printf("Name: %s\n", info->name);
    printf("State: %s\n", info->state);
    printf("Parent PID: %u\n", info->parent_pid);

    if (info->command_line == NULL || info->command_line[0] == '\0') {
        printf("Command line: [not available]\n");
    } else {
        printf("Command line: %s\n", info->command_line);
    }

    printf("Open file descriptors: %zu\n", info->fd_count);
}