default: benchmark

benchmark: benchmark.cpp
	clang++ -std=c++2a benchmark.cpp -o benchmark

.PHONY: test
test:
	@rm -f swap_file
	@./benchmark aio swap_file 10 256
