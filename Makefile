# Default target executed when you just type 'make'
.PHONY: all
all: build

# 1. Configure step (generates the build/ directory if it doesn't exist)
.PHONY: configure
configure:
	@if [ ! -d "build" ]; then \
		cmake -S . -B build; \
	fi

# 2. Build step (automatically runs configure first if needed)
.PHONY: build
build: configure
	cmake --build build

# 3. Execution step to compile and run your main application/tests immediately
.PHONY: run
run: build
	./build/run_tests

# 4. Clean step to wipe out all generated compiler noise
.PHONY: clean
clean:
	rm -rf build

# 5. Deep clean/rebuild helper
.PHONY: rebuild
rebuild: clean run