#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

char *getline(FILE *f) {
    int c, i = 0, size = 0, growby = 80;
    char *buf = NULL;

    while (1) {
        c = fgetc(f);
        if (c == EOF) break;
        while (i > size - 2) buf = (char*)realloc(buf, size += growby);
        buf[i++] = (char)c;
        if (c == '\n' || c == '\0') break;
    }
    if (i == 0) return NULL;
    buf[i] = 0;
    return buf;
}

int main(int argc, char **argv) {
	FILE *pipe = NULL, *file = NULL;
	int append = 0, nofile = 0;
    char *line, *timestamp;
    size_t timestamp_size = 0;
    time_t ct;
    struct tm *t;
    char *format = NULL;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "ahf:")) != -1) {
        switch (opt) {
            case 'a': append = 1; break;
            case 'f':
                format = (char*)malloc(strlen(optarg) + 3);
                sprintf(format, "[%s] ", optarg);
                break;
            case 'h':
                printf("Usage: %s [-a|-h|-f format] file\n", argv[0]);
                exit(0);
            case '?':
                printf("Unknown command '-%c'.\n", optopt);
            default: exit(1);
        }
    }

    if (format == NULL) {
        format = (char*)malloc(21);
        strcpy(format, "[%Y-%m-%d %H:%M:%S] ");
    }

    timestamp_size = 5 * strlen(format);
    timestamp = (char*)malloc(timestamp_size);


    if (optind > argc - 1 || !strcmp(argv[optind], "-")) nofile = 1;
    else file = fopen(argv[optind], append ? "ab" : "wb");

    if (file == NULL && nofile == 0) {
        printf("Could not open file.\n");
        exit(1);
    }

    _setmode(_fileno(stdin), _O_BINARY);
    pipe = fdopen(_fileno(stdin), "rb");
    
    if (pipe == NULL) {
        printf("Could not open pipe.\n");
        exit(1);
    }

    while ((line = getline(pipe)) != NULL) {
        time(&ct);
        t = localtime(&ct);
        strftime(timestamp, timestamp_size, format, t);
        fputs(timestamp, stdout);
        fputs(line, stdout);
        if (!nofile) {
            fputs(timestamp, file);
            fputs(line, file);
        }
    }
    
    free(line);
    free(format);
	fclose(file);
	fclose(pipe);
    return 0;
}