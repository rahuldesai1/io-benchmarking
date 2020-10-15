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

int setup(char* filename, int filesize, bool direct) {
  int flags = O_CREAT | O_RDWR | O_TRUNC;
  if (direct) {
    flags |= O_DIRECT;
  }

  int fd = open(filename, flags, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    std::cerr << "Unable to create file\n";
    std::abort();
  }
  
  if (ftruncate(fd, (off_t) filesize) != 0) {
    std::cerr << "Unable to create file with length: " + std::to_string(filesize) + "\n";
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

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: ./benchmark <aio/io_uring> swapfile(string) filesize(int) max_concurrent_swaps(int)\n";
    return 1;
  }

  std::string lib = argv[1];

  int swapfd = setup(argv[2], std::stoi(argv[3]), lib == "aio");
  int chunk_size = 64;
  void* buffer = (void*) malloc (chunk_size); 

  //io_context_t aio_ctx;

  auto start = std::chrono::steady_clock::now();
  perform_sequential_read();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end-start;
  std::cout << "Total Execution Time (Sequential Read): " << diff.count() << " seconds\n";

  int num_iterations = 10000;
  start = std::chrono::steady_clock::now();
  perform_random_read();
  end = std::chrono::steady_clock::now();
  diff = end-start;
  std::cout << "Total Execution Time (Random Read): " << diff.count() << " seconds\n";

  free(buffer);
  return 0;
}
