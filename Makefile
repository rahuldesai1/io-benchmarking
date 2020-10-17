FILENAME = swap_file.out
FILESIZE = 1000000000

default: benchmark create_file

benchmark: benchmark.cpp
	clang++ -std=c++2a benchmark.cpp -o benchmark

create_file: create_test_file.cpp
	clang++ -std=c++2a create_test_file.cpp -o create_file

.PHONY: test
test:
	@./benchmark other $(FILENAME) $(FILESIZE) 256

.PHONY: write
write:
	@rm -f $(FILENAME)
	@./create_file $(FILENAME) $(FILESIZE)

