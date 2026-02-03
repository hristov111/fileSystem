# Custom File System Implementation in C++

## Overview

A fully functional, container-based file system implementation built from scratch in C++17. This project demonstrates advanced systems programming concepts including file system design, data structures, memory management, and low-level I/O operations without relying on external libraries.

## Key Features

### Core Functionality
- **Complete File System Operations**: Create, read, delete files and directories
- **Hierarchical Directory Structure**: Full support for nested directories with parent-child relationships
- **Binary Container Storage**: All data stored in a single `.bin` file with custom binary format
- **Metadata Management**: Efficient tracking of file/directory information, offsets, and relationships
- **Block-Based Storage**: 4KB block allocation system with content-addressable storage
- **Data Deduplication**: Automatic detection and deduplication of identical content blocks using CRC32 checksums
- **Memory Recycling**: Free block and metadata management for efficient space reuse
- **Data Integrity**: CRC32 checksums for block validation and corruption detection

### Technical Highlights
- **Custom Data Structures**: Implemented from scratch without STL containers
  - Dynamic Arrays with automatic resizing
  - Hash Tables with collision handling
  - Linked Lists with type-safe operations
  - Custom String class with move semantics
- **Advanced C++ Features**:
  - Template metaprogramming
  - RAII (Resource Acquisition Is Initialization)
  - Move semantics and perfect forwarding
  - constexpr for compile-time constants
- **Serialization/Deserialization**: Custom binary protocol for persistent storage
- **Offset Management**: Dynamic offset updating when container structure changes
- **Interactive CLI**: User-friendly command-line interface for file system operations

## Architecture

### Container Structure

```
[Header - 72 bytes]
├── ID Counter (8 bytes)
├── Vector 1 Offset (8 bytes) - Free Blocks
├── Vector 2 Offset (8 bytes) - Free Metadata
├── Hash Table 1 Offset (8 bytes) - Block Hash Table
├── Hash Table 2 Offset (8 bytes) - Metadata Hash Table
├── Vec1 Allocated Size (8 bytes)
├── Vec2 Allocated Size (8 bytes)
├── Hash1 Allocated Size (8 bytes)
└── Hash2 Allocated Size (8 bytes)

[Data Structures]
├── Free Blocks Vector
├── Free Metadata Vector
├── Block Hash Table
└── Metadata Hash Table

[Root Directory Metadata]

[File/Directory Metadata Blocks]
[Content Blocks]
```

### Core Components

#### 1. **Metadata Structure**
```cpp
struct Metadata {
    char name[100];           // File/directory name
    size_t id;                // Unique identifier
    bool isDirectory;         // Type flag
    uint64_t offset;          // Position in container
    uint64_t size;            // Content size
    uint64_t parent;          // Parent directory offset
    DynamicArray<String> keys;       // Block keys
    DynamicArray<uint64_t> children; // Child offsets
}
```

#### 2. **Block Structure**
```cpp
struct Block {
    uint64_t content_offset;  // Data location
    uint64_t block_offset;    // Metadata location
    uint32_t checksum;        // CRC32 validation
    size_t size;              // Content size
    int numberOfFiles;        // Reference count
    String hashBl;            // Content hash
}
```

#### 3. **ResourceManager**
Central management class handling:
- Stream management (input/output)
- Hash table operations
- Free space tracking
- Directory navigation
- Serialization coordination

### Data Deduplication

The file system implements content-addressable storage:
1. Content is hashed using a custom hash function
2. Identical content blocks share the same physical storage
3. Reference counting tracks block usage
4. Blocks are deleted only when reference count reaches zero

This approach significantly reduces storage requirements for files with duplicate content.

### Dynamic Offset Management

When metadata or blocks change size, the system:
1. Performs depth-first search from affected point
2. Updates all downstream offsets
3. Maintains hash table consistency
4. Preserves referential integrity

## Implementation Details

### Custom Data Structures

#### DynamicArray<T>
- Automatic capacity doubling on overflow
- Move semantics support
- Iterator implementation with random access
- Range-based operations (erase, assign)

#### HashTable<Key, Value>
- Generic hash function with type specialization
- Collision handling via chaining
- Serialization support
- Key-based iteration

#### String Class
- Dynamic memory management
- Custom string operations without `<cstring>`
- Move semantics for efficient transfers
- Null-safety guarantees

### CRC32 Implementation
- Standard polynomial (0xEDB88320)
- Precomputed lookup table
- Used for data integrity verification
- Detects corruption in stored blocks

## Build Instructions

### Requirements
- GCC 7.0+ or Clang 5.0+ with C++17 support
- Linux/Unix environment (tested on Ubuntu)
- No external dependencies

### Compilation
```bash
g++ -std=c++17 -O2 -o filesystem FileName.cpp
```

### Running
```bash
./filesystem
```

## Usage Examples

### Basic Operations
```bash
# Create directory
/> md documents

# Copy file into filesystem
/> cpin /path/to/file.txt documents/file.txt

# List contents
/> ls documents

# Navigate
/> cd documents

# Copy file out
/documents> cpout file.txt /path/to/output.txt

# Delete file
/documents> rm file.txt

# Go back
/documents> cd ..

# Remove directory
/> rd documents

# Exit
/> exit
```

## Supported Commands

| Command | Description | Syntax |
|---------|-------------|--------|
| `md` | Make directory | `md <dirname>` |
| `rd` | Remove directory | `rd <dirname>` |
| `cpin` | Copy file in | `cpin <source> <dest>` |
| `cpout` | Copy file out | `cpout <source> <dest>` |
| `rm` | Remove file | `rm <filename>` |
| `ls` | List directory | `ls [path]` |
| `cd` | Change directory | `cd <dirname>` |
| `help` | Show commands | `help` |
| `exit` | Exit program | `exit` |

## Performance Characteristics

### Time Complexity
- File creation: O(1) amortized
- File deletion: O(n) where n = directory tree depth
- Directory listing: O(m) where m = number of children
- Block lookup: O(1) average case with hash table

### Space Efficiency
- 4KB block size balances overhead and efficiency
- Metadata overhead: ~170-250 bytes per file/directory
- Header overhead: 72 bytes + hash tables
- Deduplication reduces storage for identical content

## Technical Challenges Solved

1. **Dynamic Offset Management**: Solved the challenge of maintaining consistency when container structure changes by implementing recursive offset updates with DFS traversal.

2. **Memory Safety**: Implemented custom RAII patterns and careful pointer management to prevent memory leaks in a complex system with many allocations.

3. **Serialization**: Designed a robust binary serialization format that handles variable-length data (strings, arrays) while maintaining alignment and efficiency.

4. **Deduplication**: Implemented content-addressable storage with reference counting to avoid duplication while ensuring data isn't deleted prematurely.

5. **Cross-Platform Compatibility**: Replaced platform-specific functions (strncpy_s) with standard alternatives.

## Code Quality

- **No memory leaks**: Verified with proper destructor implementation and RAII
- **Exception safety**: Error handling with proper cleanup
- **const-correctness**: Proper use of const for read-only operations
- **Type safety**: Template constraints and static assertions
- **Compilation**: Zero warnings with -Wall -Wextra

## Future Enhancements

Potential improvements for production use:
- [ ] File permissions and access control
- [ ] Journaling for crash recovery
- [ ] Compression support
- [ ] Concurrent access with locking
- [ ] File system repair/fsck utility
- [ ] B-tree indexing for large directories
- [ ] Symbolic link support
- [ ] Extended attributes

## Learning Outcomes

This project demonstrates proficiency in:
- **Systems Programming**: Low-level file I/O, binary formats, memory management
- **Data Structures**: Custom implementations without relying on STL
- **C++ Expertise**: Templates, RAII, move semantics, modern C++17 features
- **Algorithm Design**: DFS traversal, hash table implementation, block allocation
- **Software Architecture**: Separation of concerns, SOLID principles
- **Problem Solving**: Complex offset management, deduplication, consistency

## Repository

**GitHub**: [https://github.com/hristov111/fileSystem](https://github.com/hristov111/fileSystem)

## Author

**Hristov** ([@hristov111](https://github.com/hristov111))

This file system was designed and implemented as a demonstration of advanced C++ programming and systems-level software development capabilities.

## License

This project is provided for educational and portfolio purposes.
