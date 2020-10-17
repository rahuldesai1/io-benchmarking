FILENAME = swap_file.out
FILESIZE = 200000000 #500MB

default: benchmark create_file

benchmark: benchmark.cpp
	clang++ -std=c++2a -laio benchmark.cpp -o benchmark

create_file: create_test_file.cpp
	clang++ -std=c++2a create_test_file.cpp -o create_file

.PHONY: test
test:
	@./benchmark $(FILENAME) $(FILESIZE)

.PHONY: write
write:
	@rm -f $(FILENAME)
	@./create_file $(FILENAME) $(FILESIZE)

.PHONY: clean
clean:
	rm -f create_file
	rm -f benchmark
