#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>

void write_to_file(int fd, size_t filesize) {
  // this is required with the O_DIRECT flag
  struct stat fs;
  fstat(fd, &fs);
  int blksize = (int) fs.st_blksize;

  char zeroes[blksize];
  for (int i = 0; i < blksize - 1; i++) {
    zeroes[i] = 'x';
  }
  zeroes[blksize - 1] = '\n';

  size_t bytes_written = 0;
  ssize_t size;
  while (bytes_written + blksize <= filesize) {
    size = write(fd, zeroes, blksize);
    if (size <= 0) {
      std::cerr << "Unable to write to file\n";
      std::abort();
    }

    bytes_written += size;
  }
}

int main(int argc, char *argv[]) {
  int flags = O_CREAT | O_RDWR | O_CREAT;

  int fd = open(argv[1], flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    std::cerr << "Unable to create file\n";
    std::abort();
  }
  
  size_t filesize = (size_t) std::stoi(argv[2]);
  write_to_file(fd, filesize);
  return 0;
}
