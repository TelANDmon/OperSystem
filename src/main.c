#include "args.h"
#include "output.h"
#include "proc_info.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned int pid;
    ProcessInfo info;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 2;
    }

    if (!parse_pid(argv[1], &pid)) {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 2;
    }

    process_info_init(&info);

    if (!process_info_read(&info, pid)) {
        process_info_destroy(&info);
        return 1;
    }

    print_process_info(&info);

    process_info_destroy(&info);

    return 0;
}