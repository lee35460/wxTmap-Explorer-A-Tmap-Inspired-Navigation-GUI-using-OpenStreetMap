.PHONY: all build debug test format clean

all: build

# Release/normal build
build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j

# Debug build (디버깅 용)
debug:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j

# Run tests
test: build
	cd build && ctest --output-on-failure

# Code format (clang-format 필요)
format:
	@if [ -d app/src ]; then clang-format -i app/src/**/*.cpp; fi
	@if [ -d app/include ]; then clang-format -i app/include/**/*.h; fi
	@if [ -d app/test ]; then clang-format -i app/test/**/*.cpp; fi

# Clean build artifacts
clean:
	rm -rf build