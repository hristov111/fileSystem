# Build and Run Instructions

## Quick Start

### Building the Project

```bash
# Navigate to the project directory
cd /path/to/fileSystem

# Compile with optimizations
g++ -std=c++17 -O2 -o filesystem FileName.cpp

# Or compile with debug symbols
g++ -std=c++17 -g -o filesystem FileName.cpp
```

### Running the File System

```bash
# Start the interactive shell
./filesystem
```

## Compilation Requirements

### Minimum Requirements
- **Compiler**: GCC 7.0+ or Clang 5.0+
- **Standard**: C++17
- **Platform**: Linux/Unix (tested on Ubuntu 20.04+)
- **Dependencies**: None (uses only standard C++ library)

### Compilation Flags

**Recommended (Production)**:
```bash
g++ -std=c++17 -O2 -o filesystem FileName.cpp
```

**With All Warnings**:
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o filesystem FileName.cpp
```

**Debug Build**:
```bash
g++ -std=c++17 -g -O0 -o filesystem FileName.cpp
```

## First Run

When you first run the program, it will automatically:
1. Create a `container.bin` file in the current directory
2. Initialize the file system with a root directory
3. Display the interactive prompt

```
=== Custom File System ===
Available commands:
  md <dirname>       - Create directory
  rd <dirname>       - Remove directory
  cpin <src> <dest>  - Copy file into filesystem
  cpout <src> <dest> - Copy file out of filesystem
  rm <filename>      - Remove file
  ls [path]          - List directory contents
  cd <dirname>       - Change directory
  exit               - Exit program
===========================

/> 
```

## Usage Examples

### Example Session 1: Basic File Operations

```bash
# Start the filesystem
./filesystem

# Create a directory
/> md projects

# Change into the directory
/> cd projects

# Copy a file into the filesystem
/projects> cpin /home/user/document.txt document.txt

# List contents
/projects> ls
document.txt 1024: parent is projects

# Copy file back out
/projects> cpout document.txt /home/user/backup.txt
File successfully copied to /home/user/backup.txt.

# Go back to root
/projects> cd ..

# List root contents
/> ls
projects 1024: parent is /

# Exit
/> exit
Exiting filesystem...
```

### Example Session 2: Directory Hierarchy

```bash
# Create nested directory structure
/> md work
/> cd work
/work> md reports
/work> md data
/work> ls
reports 0: parent is work
data 0: parent is work

# Add files to subdirectories
/work> cd reports
/work/reports> cpin /tmp/report.pdf report.pdf
/work/reports> cd ..

# Remove directory
/work> cd ..
/> rd work
Directory removed successfully
```

### Example Session 3: File Management

```bash
# Copy multiple files
/> md documents
/> cpin file1.txt documents/file1.txt
/> cpin file2.txt documents/file2.txt
/> cpin file3.txt documents/file3.txt

# List all files
/> ls documents

# Remove specific file
/> rm documents/file2.txt
File removed successfully

# Verify deletion
/> ls documents
```

## Container File

The file system stores all data in a single `container.bin` file:

- **Location**: Created in the current working directory
- **Format**: Custom binary format
- **Persistence**: Data persists between program runs
- **Safety**: Can be backed up by copying the .bin file

### Backing Up

```bash
# Backup the container
cp container.bin container_backup.bin

# Restore from backup
cp container_backup.bin container.bin
```

### Starting Fresh

```bash
# Delete the container to start over
rm container.bin

# Next run will create a new empty filesystem
./filesystem
```

## Troubleshooting

### Problem: Compilation fails with "strncpy_s not found"

**Solution**: The code has been fixed to use standard `strncpy`. Ensure you're using the latest version of FileName.cpp.

### Problem: Segmentation fault on startup

**Possible causes**:
- Corrupted container.bin file
- Insufficient disk space

**Solution**:
```bash
# Remove the corrupted container
rm container.bin

# Restart the program
./filesystem
```

### Problem: "Cannot open file container"

**Possible causes**:
- No write permissions in current directory
- Disk full

**Solution**:
```bash
# Check permissions
ls -l container.bin

# Check disk space
df -h .

# Ensure write access
chmod 644 container.bin
```

### Problem: File system becomes unresponsive

**Solution**:
- Press Ctrl+C to exit
- Check container.bin size (may indicate corruption)
- Consider starting with a fresh container

## Performance Notes

### File Size Recommendations
- **Small files** (< 4KB): Stored efficiently in single blocks
- **Large files** (> 1MB): May cause slower operations due to offset updates
- **Very large files** (> 100MB): Not recommended for this implementation

### Directory Limits
- No hard limit on number of files per directory
- Performance degrades with > 1000 files in single directory
- Deep nesting (> 10 levels) may slow down operations

### Best Practices
1. Keep directory structures relatively flat (< 5 levels)
2. Limit files per directory to < 500 for best performance
3. Regularly backup container.bin for important data
4. Use descriptive names (up to 99 characters)

## Advanced Usage

### Scripting

You can script file system operations (though currently limited):

```bash
# Example: automated backup script
echo "ls" | ./filesystem
```

### Inspecting the Container

```bash
# View container size
ls -lh container.bin

# View hex dump of container header
hexdump -C container.bin | head -20
```

## Development

### Modifying the Code

Key files:
- `FileName.cpp`: Main implementation (all code is in this single file)
- `container.bin`: Binary container file (auto-generated)

### Debugging

```bash
# Compile with debug symbols
g++ -std=c++17 -g -O0 -o filesystem FileName.cpp

# Run with gdb
gdb ./filesystem

# Or with valgrind for memory leak detection
valgrind --leak-check=full ./filesystem
```

### Code Structure

The implementation is organized into several key components:
1. **Custom Data Structures** (Lines 1-1310)
   - DynamicArray
   - LinkedList
   - HashTable
   - String class

2. **File System Structures** (Lines 1311-1920)
   - Metadata
   - Block
   - ResourceManager

3. **File System Operations** (Lines 2104-3722)
   - File operations (cpin, cpout, rm)
   - Directory operations (md, rd, cd, ls)
   - Helper functions

## Getting Help

If you encounter issues:
1. Check the README.md for architecture details
2. Review this BUILD_AND_RUN.md for usage examples
3. Ensure your compiler supports C++17
4. Try with a fresh container.bin file

## Contributing

This project is a portfolio piece. If you'd like to suggest improvements:
1. Document the issue or enhancement
2. Test with the latest version of the code
3. Provide specific examples or test cases
