#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

#define MIN(a,b) ((a) < (b) ? (a) : (b))

int tail_file(FILE *, int);
int copy_to_end(FILE *, FILE *);
int write_buffer(char *, size_t , FILE *);

int main(int argc, char **argv) {

    // Tail each file from argument list one at a time
    while(*++argv) {

        // Output the header, but only if we are tailing more than one file
        if(argc > 2)
            printf("==> %s <==\n", *argv);

        // Open the file
        FILE *file;
        if(!(file = fopen(*argv, "r"))) {
            fprintf(stderr, "tail: %s: %s\n", *argv, strerror(errno));
            continue;
        }

        // tail the last ten lines of the file
        if(tail_file(file, 10)) {
            fprintf(stderr, "tail: %s: %s\n", *argv, strerror(errno));
        }

        fclose(file);
    }
}

// Output the last 'n' lines from 'file'
int tail_file(FILE *file, int n) {

    char buffer[BUFFER_SIZE];

    // Figure out the size of the file
    struct stat sb;
    if(fstat(fileno(file), &sb))
        return -1;

    // Read from the end of the file
    if(fseek(file, 0, SEEK_END))
        return -1;

    off_t bytes_left = sb.st_size;
    int bytes_to_read;
    int newline_count = 0;
    int bytes_read = 0;

    // Keep reading backwards until we have seen 'n' lines or we get to the
    // beginning of the file, whichever comes first.
    while(bytes_left && newline_count <= n) {

        // Get ready to read the next block
        bytes_to_read = MIN(bytes_left, BUFFER_SIZE);
        if(fseek(file, -(bytes_to_read + bytes_read), SEEK_CUR))
            return -1;

        // Read a block
        if(!(bytes_read = fread(buffer, sizeof(char), bytes_to_read, file)))
            return -1;
        bytes_left -= bytes_read;

        // Count the newlines
        for(char *p = buffer + bytes_read - 1; p >= buffer; p--) {
            if(*p == '\n' && ++newline_count > n) {
                // output the remaining buffer
                p++; // Skip the newline
                if(write_buffer(p, buffer + bytes_read - p, stdout))
                    return -1;
                break;
            }
        }
    }

    // Go to be beginning of the file if we couldn't find all the newlines.
    if(!bytes_left && fseek(file, 0, SEEK_SET))
        return -1;

    return copy_to_end(file, stdout);
}

// Read 'in' from the current file position and write to 'out'
int copy_to_end(FILE *in, FILE *out) {
    size_t n;
    char buffer[BUFFER_SIZE];

    while(!feof(in)) {
        if(!(n = fread(buffer, sizeof(char), BUFFER_SIZE, in)))
            return ferror(in);

        if(n && write_buffer(buffer, n, out))
            return -1;
    }

    return 0;
}

// Write a buffer to the file
int write_buffer(char *buffer, size_t size, FILE *file) {
    int nwritten = 0;
    char *p = buffer;
    while((p += nwritten) < buffer + size) {
        if(!(nwritten = fwrite(p, sizeof(char), buffer + size - p, file)))
            return -1;
    }
    return 0;
}
