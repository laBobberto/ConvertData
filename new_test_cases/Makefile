CXX = ./i486-linux-musl-cross/bin/i486-linux-musl-g++
BASE_FLAGS = -march=i586 -mtune=i686 -fno-omit-frame-pointer -std=c++20 -static

all: binaries_o1 binaries_o2 binaries_o3

binaries_o1:
	$(CXX) $(BASE_FLAGS) -O1 -DOPT_LEVEL='"O1"' version_original.cpp -o original_o1
	$(CXX) $(BASE_FLAGS) -O1 -DOPT_LEVEL='"O1"' version_v1.cpp -o v1_o1
	$(CXX) $(BASE_FLAGS) -O1 -DOPT_LEVEL='"O1"' version_v2.cpp -o v2_o1
	$(CXX) $(BASE_FLAGS) -O1 -DOPT_LEVEL='"O1"' version_v4.cpp -o v4_o1

binaries_o2:
	$(CXX) $(BASE_FLAGS) -O2 -DOPT_LEVEL='"O2"' version_original.cpp -o original_o2
	$(CXX) $(BASE_FLAGS) -O2 -DOPT_LEVEL='"O2"' version_v1.cpp -o v1_o2
	$(CXX) $(BASE_FLAGS) -O2 -DOPT_LEVEL='"O2"' version_v2.cpp -o v2_o2
	$(CXX) $(BASE_FLAGS) -O2 -DOPT_LEVEL='"O2"' version_v4.cpp -o v4_o2

binaries_o3:
	$(CXX) $(BASE_FLAGS) -O3 -DOPT_LEVEL='"O3"' version_original.cpp -o original_o3
	$(CXX) $(BASE_FLAGS) -O3 -DOPT_LEVEL='"O3"' version_v1.cpp -o v1_o3
	$(CXX) $(BASE_FLAGS) -O3 -DOPT_LEVEL='"O3"' version_v2.cpp -o v2_o3
	$(CXX) $(BASE_FLAGS) -O3 -DOPT_LEVEL='"O3"' version_v4.cpp -o v4_o3

run: all
	echo "Version,OptLevel,Average_ns,Median_ns,Min_ns,Max_ns,P95_ns,P99_ns" > results.csv
	./original_o1 >> results.csv
	./v1_o1 >> results.csv
	./v2_o1 >> results.csv
	./v4_o1 >> results.csv
	./original_o2 >> results.csv
	./v1_o2 >> results.csv
	./v2_o2 >> results.csv
	./v4_o2 >> results.csv
	./original_o3 >> results.csv
	./v1_o3 >> results.csv
	./v2_o3 >> results.csv
	./v4_o3 >> results.csv

clean:
	rm -f original_o1 v1_o1 v2_o1 v4_o1 original_o2 v1_o2 v2_o2 v4_o2 original_o3 v1_o3 v2_o3 v4_o3 results.csv
