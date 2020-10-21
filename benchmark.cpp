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
#include <liburing.h>

static int chunk_size = 4096;
// the O_DIRECT flag requires the data to be block aligned
static char buffer[4096] __attribute__ ((__aligned__ (4096)));

int setup(char* filename) {
  int flags = O_RDWR | O_DIRECT;
  int fd = open(filename, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    std::cerr << "Unable to open file\n";
    std::abort();
  }
  
  return fd;
}

void baseline_read(int fd, size_t filesize) {
  int total = 0;
  ssize_t bytes_read;
  while ((bytes_read = read(fd, buffer, chunk_size)) > 0) {
    total += bytes_read;
  }

  if (bytes_read == -1) {
    std::cout << "Error: " << errno << "\n";
    std::abort();
  }
  std::cout << "Number of bytes read: " << total << "\n";
}


/* 
 * Initializes the aio context object and return the file descriptor
 * of the swap file created
 */
void initialize_aio(io_context_t *aio_ctx) {
  int rv;
  if ((rv = io_setup(10, aio_ctx)) != 0) {
    std::cerr << "Failed to initialize aio context object: "<< rv << "\n";
    std::abort();
  }
}

void aio_read(io_context_t *aio_ctx, int fd, size_t filesize) {
  std::chrono::duration<double> average_time = std::chrono::steady_clock::duration::zero();
  int num_iterations = 0;
  for (int i = 0; i + chunk_size <= filesize; i += chunk_size) {
    auto start = std::chrono::steady_clock::now();
    struct iocb op;
    struct iocb* op_ptr = &op;
    io_prep_pread(op_ptr, fd, buffer, chunk_size, i);
    if (io_submit(*aio_ctx, 1, &op_ptr) != 1) {
      std::cerr << "Failed to submit aio read operation\n";
      std::abort();
    } 
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> diff = end-start;
    average_time += diff;
    num_iterations += 1;

    // block by waiting for the io_response to return 
    struct io_event events[1]; 
    int rv = io_getevents(*aio_ctx, 1, 1, events, nullptr);
    if (rv < 1) {
      std::cerr << "Failed to retrieve events: " << rv;
      std::abort();
    }

    if (events[0].res < 0) {
      std::cerr << "The system call invoked asynchonously failed: " << events[0].res << "\n";
      std::abort();
    }
  }

  std::cout << "Average Initialization Time (AIO): " << average_time.count() / num_iterations << "\n";
}

void initialize_uring(struct io_uring *ring) {
  io_uring_queue_init(10, ring, 0);
}

void uring_read(struct io_uring *ring, int fd, size_t filesize) {
  std::chrono::duration<double> average_time = std::chrono::steady_clock::duration::zero();
  int num_iterations = 0;
  for (int i = 0; i + chunk_size <= filesize; i += chunk_size) {
    auto start = std::chrono::steady_clock::now();
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_read(sqe, fd, buffer, chunk_size, i);
    if (io_uring_submit(ring) != 1) {
      std::cerr << "Failed to submit uring read operation\n";
      std::abort();
    } 
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end-start;
    average_time += diff;
    num_iterations += 1;

    struct io_uring_cqe *cqe;
    int rv = io_uring_wait_cqe(ring, &cqe);
    if (rv < 0) {
      std::cerr << "Failed to retrieve events: " << rv;
      std::abort();
    }

    if (cqe->res < 0) {
      std::cerr << "The system call invoked asynchonously failed: " << cqe->res << "\n";
      std::abort();
    }
    io_uring_cqe_seen(ring, cqe);
  }
  std::cout << "Average Initialization Time (Uring): " << average_time.count() / num_iterations << "\n";
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./benchmark swapfile(string) filesize(int)\n";
    return 1;
  }

  int swapfd = setup(argv[1]);
  size_t filesize = (size_t) std::stoi(argv[2]);

  /** Mostly a sanity check to make sure that the read operation works **/
  /**
  baseline_read(swapfd, filesize);
   // reset the file pointer to the beginning
  lseek(swapfd, 0, SEEK_SET);
  **/

  /** Execute AIO operations **/
  //io_context_t aio_ctx = 0;
  //initialize_aio(&aio_ctx);
  //aio_read(&aio_ctx, swapfd, filesize);
  //io_destroy(aio_ctx);

  //// reset the file pointer to the beginning
  //lseek(swapfd, 0, SEEK_SET);

  /** Execute URING operations **/
  struct io_uring ring;
  initialize_uring(&ring);
  uring_read(&ring, swapfd, filesize);

  close(swapfd);
  return 0;
}
