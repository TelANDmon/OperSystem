#ifndef PROC_INFO_H
#define PROC_INFO_H
#include <stddef.h>

typedef struct {
    unsigned int pid;
    unsigned int parent_pid;

    char name[256];
    char state[256];
    char *command_line;
    size_t fd_count;

} ProcessInfo;

void process_info_init(ProcessInfo *info);
void process_info_destroy(ProcessInfo *info);
int process_info_read(ProcessInfo *info, unsigned int pid);

#endif