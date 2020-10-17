#include <iostream>
#include <cassert>
#include <chrono>
#include <libaio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>

int chunk_size = 64;

int setup(char* filename, bool direct) {
  int flags = O_CREAT | O_RDWR;
  if (direct) {
    flags |= O_DIRECT;
  }

  int fd = open(filename, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    std::cerr << "Unable to create file\n";
    std::abort();
  }
  
  return fd;
}


/* 
 * Initializes the aio context object and return the file descriptor
 * of the swap file created
 */
//void initialize_aio(int concurrent_swaps, io_context_t *aio_ctx) {
  //if (io_setup(concurrent_swaps, aio_ctx)) {

  //}
//}

void aio_sequential_read(int fd, size_t filesize) {
  char buffer[chunk_size + 1]; 
  int total = 0;
  ssize_t bytes_read;
  while ((bytes_read = read(fd, buffer, chunk_size)) > 0) {
    total += bytes_read;
  }

  std::cout << "Number of bytes read: " << total << "\n";
}

void aio_random_read(int fd, size_t filesize, int num_iterations) {
  for (int i = 0; i < num_iterations; i++) {
    int r = rand() % filesize;

  }

}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: ./benchmark <aio/io_uring> swapfile(string) filesize(int) max_concurrent_swaps(int)\n";
    return 1;
  }

  std::string lib = argv[1];
  size_t filesize = (size_t) std::stoi(argv[3]);

  int swapfd = setup(argv[2], lib == "aio");

  //io_context_t aio_ctx;

  auto start = std::chrono::steady_clock::now();
  aio_sequential_read(swapfd, filesize);
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end-start;
  std::cout << "Total Execution Time (Sequential Read): " << diff.count() << " seconds\n";

  start = std::chrono::steady_clock::now();
  aio_random_read(swapfd, filesize, 1000);
  end = std::chrono::steady_clock::now();
  diff = end-start;
  std::cout << "Total Execution Time (Random Read): " << diff.count() << " seconds\n";

  close(swapfd);
  return 0;
}
