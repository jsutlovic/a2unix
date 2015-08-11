#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0
#define READ_SIZE 1024

#define REPLACE 0xD
#define NEWLINE 0xA

void usage(char *name) {
    char *usage_doc = "Usage: %s\n"
                      "  This is a utility to convert all line endings to unix newlines\n";
    printf(usage_doc, name);
    exit(EXIT_FAILURE);
}

void find_file(char *filename) {
    struct stat stat_buffer;
    if (stat(filename, &stat_buffer) == -1) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    if (!S_ISREG(stat_buffer.st_mode)) {
        printf("%s is not a file\n", filename);
        exit(EXIT_FAILURE);
    }

#if (DEBUG)
    printf("Found file: %s\n", filename);
#endif
}

FILE *open_file(char *filename) {
    FILE *stream;
    stream = fopen(filename, "r+");
    if (stream == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    return stream;
}

void rewrite_newlines(FILE *stream) {
    char rbuf[READ_SIZE];
    size_t read_size;
    long read_pos = 0, write_pos = 0;

    // Forever loop
    for (;;) {
        fseek(stream, read_pos, SEEK_SET);
        read_size = fread(rbuf, sizeof *rbuf, READ_SIZE, stream);
        read_pos += read_size;

        // Replace bad chars
        for (int i = 0; i < read_size; i++) {
#if (DEBUG)
            printf("i: %d\n", i);
#endif
            if (rbuf[i] == REPLACE) {
#if (DEBUG)
                printf("found repl\n");
#endif
                // If there is already a '\n', delete the '\r'.
                // Otherwise replace the '\r' with a '\n'.
                if ((i > 0 && rbuf[i-1] == NEWLINE) || (i < (read_size) && rbuf[i+1] == NEWLINE)) {
                    read_size--;
#if (DEBUG)
                        printf("read_size: %ld\n", read_size);
#endif
                    for (int j = i; j < read_size; j++) {
#if (DEBUG)
                        printf("j: %d\n", j);
#endif
                        rbuf[j] = rbuf[j+1];
                    }
                } else {
                    rbuf[i] = NEWLINE;
                }
            }
        }

        if (read_size < READ_SIZE) {
            if (feof(stream)) {
#if (DEBUG)
                printf("EOF found\n");
#endif
                break;
            } else if (ferror(stream)) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
        }

#if (DEBUG)
        printf("fseek\n");
#endif
        fseek(stream, write_pos, SEEK_SET);
#if (DEBUG)
        printf("fwrite\n");
#endif
        write_pos += fwrite(rbuf, sizeof *rbuf, read_size, stream);
    }

    // Cleanup
#if (DEBUG)
    printf("fseek\n");
#endif
    fseek(stream, write_pos, SEEK_SET);
#if (DEBUG)
    printf("fwrite\n");
#endif
    write_pos += fwrite(rbuf, sizeof *rbuf, read_size, stream);

    // Truncate file to total write length
#if (DEBUG)
    printf("ftruncate\n");
    printf("write_pos: %ld\n", write_pos);
#endif
    if (ftruncate(fileno(stream), (off_t) write_pos)) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }

#if (DEBUG)
    printf("fflush\n");
#endif
    // Flush to disk (userspace)
    if (fflush(stream)) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }

#if (DEBUG)
    printf("fsync\n");
#endif
    // Flush to disk (kernel space)
    if (fsync(fileno(stream))) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }

#if (DEBUG)
    printf("fclose\n");
#endif
    // Close file descriptor
    if (fclose(stream)) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        usage(argv[0]);
    } else {
        for (int fc = 1; fc < argc; fc++) {
#if (DEBUG)
            printf("fc: %d\n", fc);
#endif
            find_file(argv[fc]);
            rewrite_newlines(open_file(argv[fc]));
        }
    }
    return 0;
}
