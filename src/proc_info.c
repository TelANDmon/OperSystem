#include "proc_info.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_info_init(ProcessInfo *info)
{
    info->pid = 0;
    info->parent_pid = 0;

    info->name[0] = '\0';
    info->state[0] = '\0';

    info->command_line = NULL;

    info->fd_count = 0;
}

void process_info_destroy(ProcessInfo *info)
{
    free(info->command_line);
    info->command_line = NULL;
}

static void remove_newline(char *text)
{
    size_t length = strlen(text);

    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

static const char *get_state_description(char code)
{
    switch(code) {
        case 'R':
            return "running";
        case 'S':
            return "sleeping";
        case 'D':
            return "disk sleep";
        case 'T':
            return "stopped";
        case 't':
            return "tracing stop";
        case 'Z':
            return "zombie";
        case 'I':
            return "idle";
        default:
            return "unknown";
    }
}

static int read_status(ProcessInfo *info, unsigned int pid)
{
    char path[256];
    char line[512];

    int name_found = 0;
    int state_found = 0;
    int pid_found = 0;
    int ppid_found = 0;

    int result = snprintf(
        path,
        sizeof(path),
        "/proc/%u/status",
        pid
    );

    if (result < 0 || (size_t)result >= sizeof(path)) {
        fprintf(stderr, "Failed to create status path\n");
        return 0;
    }

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        perror("Cannot open status");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {

        if (strncmp(line, "Name:", 5) == 0) {
            char *value = line + 5;

            while (*value == ' ' || *value == '\t') {
                value++;
            }

            remove_newline(value);

            snprintf(
                info->name,
                sizeof(info->name),
                "%s",
                value
            );

            name_found = 1;
        }


        else if (strncmp(line, "State:", 6) == 0) {
            char *value = line + 6;
            while (*value == ' ' || *value == '\t') {
                value++;
            }
            char state_code = *value;
            snprintf(
                info->state,
                sizeof(info->state),
                "%c (%s)",
                state_code,
                get_state_description(state_code)
            );
            
            state_found = 1;
}

        else if (strncmp(line, "Pid:", 4) == 0) {
            unsigned int value;

            if (sscanf(line + 4, "%u", &value) == 1) {
                info->pid = value;
                pid_found = 1;
            }
        }

        else if (strncmp(line, "PPid:", 5) == 0) {
            unsigned int value;

            if (sscanf(line + 5, "%u", &value) == 1) {
                info->parent_pid = value;
                ppid_found = 1;
            }
        }
    }

    fclose(file);

    if (!name_found || !state_found || !pid_found || !ppid_found) {
        fprintf(stderr, "Required fields not found in status\n");
        return 0;
    }

    return 1;
}

static int read_cmdline(ProcessInfo *info, unsigned int pid)
{
    char path[256];

    int result = snprintf(
        path,
        sizeof(path),
        "/proc/%u/cmdline",
        pid
    );

    if (result < 0 || (size_t)result >= sizeof(path)) {
        fprintf(stderr, "Failed to create cmdline path\n");
        return 0;
    }

    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        perror("Cannot open cmdline");
        return 0;
    }

    size_t capacity = 128;
    size_t length = 0;

    char *buffer = malloc(capacity);

    if (buffer == NULL) {
        fprintf(stderr, "Cannot allocate memory\n");
        fclose(file);
        return 0;
    }

    int ch;

    while ((ch = fgetc(file)) != EOF) {

        if (length + 1 >= capacity) {
            capacity *= 2;

            char *new_buffer = realloc(buffer, capacity);

            if (new_buffer == NULL) {
                fprintf(stderr, "Cannot allocate memory\n");
                free(buffer);
                fclose(file);
                return 0;
            }

            buffer = new_buffer;
        }

        if (ch == '\0') {
            buffer[length] = ' ';
        } else {
            buffer[length] = (char)ch;
        }

        length++;
    }

    if (ferror(file)) {
        perror("Cannot read cmdline");
        free(buffer);
        fclose(file);
        return 0;
    }

    fclose(file);

    while (length > 0 && buffer[length - 1] == ' ') {
        length--;
    }

    if (length == 0) {
        free(buffer);
        info->command_line = NULL;
        return 1;
    }

    buffer[length] = '\0';

    info->command_line = buffer;

    return 1;
}
static int count_fd(ProcessInfo *info, unsigned int pid)
{
    char path[256];

    int result = snprintf(
        path,
        sizeof(path),
        "/proc/%u/fd",
        pid
    );

    if (result < 0 || (size_t)result >= sizeof(path)) {
        fprintf(stderr, "Failed to create fd path\n");
        return 0;
    }

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Cannot open fd directory");
        return 0;
    }

    size_t count = 0;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        count++;
    }

    closedir(dir);

    info->fd_count = count;

    return 1;
}

int process_info_read(ProcessInfo *info, unsigned int pid)
{
    if (!read_status(info, pid)) {
        return 0;
    }

    if (!read_cmdline(info, pid)) {
        return 0;
    }

    if (!count_fd(info, pid)) {
        return 0;
    }

    return 1;
}