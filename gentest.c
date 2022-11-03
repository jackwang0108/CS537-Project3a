#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct rec {
    int key;
    int value[24]; // 96 bytes
} rec_t;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "generate numkeys fail\n");
        fprintf(stderr, "Usage: <executable> num_record filename\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    char *file = argv[2];
    int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    int k;
    for (k = 0; k < n; k++) {
        long u = random();
        rec_t r;
        r.key = u;
        int i;
        for (i = 0; i < 24; i++) {
            r.value[i] = random();
        }
        int rc = write(fd, &r, sizeof(rec_t));
        assert(rc == sizeof(rec_t));
    }
    close(fd);
    return 0;
}
