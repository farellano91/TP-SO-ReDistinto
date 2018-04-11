#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

int main(int argc, char** argv) {
    size_t filesize = getFilesize(argv[1]);
    //Open file
    int fd = open(argv[1], O_RDONLY, 0);
    assert(fd != -1);
    //Execute mmap
    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    assert(mmappedData != MAP_FAILED);
    //Write the mmapped data to stdout (= FD #1)
   // write(1, mmappedData, filesize);

    char* datos = malloc(filesize + 1);
    memcpy(datos,mmappedData,filesize + 1);
    datos[strlen(datos)] = '\0';

    printf("CONTENIDO EN CHARs: %s\n",datos );
    //Cleanup
    int rc = munmap(mmappedData, filesize);
    assert(rc == 0);
    close(fd);
    free(datos);
}