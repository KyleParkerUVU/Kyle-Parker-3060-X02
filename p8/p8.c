#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// Function prototype
void list_directory(const char *dir_path, int depth, long long *total_size);

int main(int argc, char *argv[]) {
    const char *start_dir;
    long long total_size = 0;

    // Determine starting directory
    if (argc == 1) {
        start_dir = ".";
        printf("dir .\n");
    } else {
        start_dir = argv[1];
        printf("dir %s\n", start_dir);
    }

    // Start recursive traversal
    list_directory(start_dir, 1, &total_size);

    // Display total file space used
    printf("\nTotal file space used:%lld\n", total_size);

    return 0;
}

void list_directory(const char *dir_path, int depth, long long *total_size) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char *full_path;

    // Open the directory
    dir = opendir(dir_path);
    if (dir == NULL) {
        // Silently ignore directories that can't be opened
        return;
    }

    // Read each entry
    while ((entry = readdir(dir)) != NULL) {
        // Skip entries starting with '.' (including "." and "..")
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path using asprintf (handy, as per hint)
        if (asprintf(&full_path, "%s/%s", dir_path, entry->d_name) == -1) {
            continue; // Memory allocation failed
        }

        // Use lstat to get file info (doesn't follow symlinks)
        if (lstat(full_path, &file_stat) == -1) {
            free(full_path);
            continue; // Skip if we can't stat
        }

        // Check if it's a regular file
        if (S_ISREG(file_stat.st_mode)) {
            // Print with indentation
            for (int i = 0; i < depth; i++) {
                printf("  ");
            }
            printf("%10lld:%s\n", (long long)file_stat.st_size, entry->d_name);
            *total_size += file_stat.st_size;
        }
        // Check if it's a directory (but not a symlink)
        else if (S_ISDIR(file_stat.st_mode)) {
            // Print directory header with indentation
            for (int i = 0; i < depth; i++) {
                printf("  ");
            }
            printf("dir %s\n", entry->d_name);

            // Recursively list subdirectory
            list_directory(full_path, depth + 1, total_size);
        }
        // Ignore other types (symlinks, sockets, devices, etc.)

        free(full_path);
    }

    closedir(dir);
}