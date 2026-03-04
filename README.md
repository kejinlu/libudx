# libudx

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C](https://img.shields.io/badge/C-11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard))
[![CI](https://github.com/kejinlu/libudx/actions/workflows/ci.yml/badge.svg)](https://github.com/kejinlu/libudx/actions/workflows/ci.yml)

**Universal Dictionary eXchange** — A fast, minimal C library for reading and writing dictionary data files with efficient B+ tree indexing and zlib compression.

## Features

- **High Performance** — B+ tree indexing with O(log n) lookup
- **Space Efficient** — Zlib compression for reduced storage
- **Case-Insensitive Search** — Automatic word folding for case-insensitive lookup
- **Prefix Matching** — Efficient prefix-based searching
- **Multiple Databases** — Single file can contain multiple named databases
- **Metadata Support** — Attach custom metadata to each database
- **Zero-Copy Parsing** — Efficient data access with minimal copying
- **Chunk-Based Storage** — 64KB chunks for optimal compression ratios
- **Large File Support** — 64-bit file offsets for files > 2GB
- **Permissive License** — MIT License with some components under their original licenses

## Building

### Prerequisites

- C compiler with C11 support
- zlib development library
- CMake (recommended) or direct compilation

### Install Dependencies

**macOS:**
```bash
brew install zlib
```

**Ubuntu/Debian:**
```bash
sudo apt-get install zlib1g-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install zlib-devel
```

### Build from Source

```bash
git clone https://github.com/kejinlu/libudx.git
cd libudx
mkdir build && cd build
cmake ..
make
sudo make install
```

### Compile Directly

```bash
gcc -o myprogram myprogram.c -I./src -L./build -ludx -lz
```

## Quick Start

### Writing a Dictionary File

```c
#include "udx_writer.h"

int main() {
    // Open a new UDX file for writing
    udx_writer *writer = udx_writer_open("mydict.udx");
    if (!writer) {
        fprintf(stderr, "Failed to create writer\n");
        return 1;
    }

    // Create a database builder with optional metadata
    const char *metadata = "Created by libudx";
    udx_db_builder *builder = udx_db_builder_create_with_metadata(
        writer, "english", (const uint8_t *)metadata, strlen(metadata)
    );

    // Add entries
    udx_db_builder_add_entry(builder, "hello", (const uint8_t *)data1, size1);
    udx_db_builder_add_entry(builder, "world", (const uint8_t *)data2, size2);

    // Finish building the database
    udx_db_builder_finish(builder);

    // Close the writer
    udx_writer_close(writer);
    return 0;
}
```

### Reading a Dictionary File

```c
#include "udx_reader.h"

int main() {
    // Open a UDX file
    udx_reader *reader = udx_reader_open("mydict.udx");
    if (!reader) {
        fprintf(stderr, "Failed to open file\n");
        return 1;
    }

    // Open a database by name (or use NULL for first database)
    udx_db *db = udx_db_open(reader, "english");
    if (!db) {
        fprintf(stderr, "Database not found\n");
        udx_reader_close(reader);
        return 1;
    }

    // Look up a word
    udx_db_entry *entry = udx_db_lookup(db, "hello");
    if (entry) {
        printf("Found: %s\n", entry->word);
        for (size_t i = 0; i < entry->items.size; i++) {
            printf("  Data[%zu]: %zu bytes\n", i, entry->items.data[i].size);
        }
        udx_db_entry_free(entry);
    }

    // Cleanup
    udx_db_close(db);
    udx_reader_close(reader);
    return 0;
}
```

### Prefix Matching

```c
// Find all words starting with "hel"
udx_index_entry_array results = udx_db_index_prefix_match(db, "hel", 100);

for (size_t i = 0; i < results.size; i++) {
    printf("Word: %s (%zu items)\n",
           results.data[i].word,
           results.data[i].items.size);
}

// Free results
udx_index_entry_array_free_contents(&results);
```

### Iteration

```c
udx_db_iter *iter = udx_db_iter_create(db);
const udx_db_entry *entry;

while ((entry = udx_db_iter_next(iter)) != NULL) {
    printf("%s\n", entry->word);
}

udx_db_iter_destroy(iter);
```

## API Overview

### Writer APIs

| Function | Description |
|----------|-------------|
| `udx_writer_open()` | Create a new UDX file |
| `udx_writer_close()` | Finalize and close the file |
| `udx_db_builder_create()` | Create a database builder |
| `udx_db_builder_add_entry()` | Add a word/data pair |
| `udx_db_builder_finish()` | Complete building the database |

### Reader APIs

| Function | Description |
|----------|-------------|
| `udx_reader_open()` | Open an existing UDX file |
| `udx_reader_close()` | Close the file |
| `udx_db_open()` | Open a database by name |
| `udx_db_close()` | Close a database |
| `udx_db_lookup()` | Look up a word (exact match) |
| `udx_db_index_lookup()` | Low-level index lookup |
| `udx_db_index_prefix_match()` | Prefix-based search |
| `udx_db_iter_create()` | Create an iterator |
| `udx_db_iter_next()` | Get next entry |

### Error Handling

All functions that can fail use `udx_error_t` return codes:

```c
typedef enum {
    UDX_OK                      = 0,    // Success
    UDX_ERR_INVALID_PARAM       = -1,   // Invalid parameter
    UDX_ERR_IO                  = -2,   // File I/O error
    UDX_ERR_BPTREE              = -3,   // B+ tree operation failed
    UDX_ERR_HEADER              = -4,   // Header read/write failed
    UDX_ERR_CHUNK               = -5,   // Chunk operation failed
    UDX_ERR_WORDS               = -6,   // Words container failed
    UDX_ERR_ACTIVE_DB           = -7,   // Database builder still active
    UDX_ERR_DUPLICATE_NAME      = -8,   // Duplicate database name
    UDX_ERR_METADATA            = -9,   // Invalid metadata parameters
    UDX_ERR_OVERFLOW            = -10,  // Integer/size overflow
    UDX_ERR_MEMORY              = -11   // Memory allocation failed
} udx_error_t;
```

## File Format

UDX files are organized as follows:

```
┌─────────────────────────────────────────────────────────┐
│ Main Header                                             │
│  - Magic: "UDX\0"                                       │
│  - Version: 1.0                                         │
│  - Database Table Offset                                │
├─────────────────────────────────────────────────────────┤
│ Database 1                                              │
│  - DB Header (metadata size, offsets, counts, checksum) │
│  - Metadata (optional)                                  │
│  - Chunk Table                                          │
│  - Chunks (compressed data blocks)                      │
│  - B+ Tree Index                                        │
│    - Internal Nodes (compressed)                        │
│    - Leaf Nodes (compressed)                            │
├─────────────────────────────────────────────────────────┤
│ Database 2                                              │
│  - ...                                                  │
├─────────────────────────────────────────────────────────┤
│ ...                                                     │
├─────────────────────────────────────────────────────────┤
│ Database Table                                          │
│  - Count                                                │
│  - [Offset, Name] pairs                                 │
└─────────────────────────────────────────────────────────┘
```

### Key Design Points

- **Chunk Storage**: Data is stored in 64KB chunks, each compressed independently
- **Address Encoding**: 48-bit chunk index + 16-bit offset in chunk
- **Index Structure**: Static B+ tree with GoldenDict-style layout
- **Checksum**: CRC32 checksum on database header for corruption detection

## Architecture

```
libudx/
├── src/
│   ├── udx_writer.h/c    # High-level writer API
│   ├── udx_reader.h/c    # High-level reader API
│   ├── udx_chunk.h/c     # Chunk-based storage with compression
│   ├── udx_words.h/c     # Ordered word container (B-tree wrapper)
│   ├── udx_types.h/c     # Core data types and serialization
│   ├── udx_utils.h/c     # Utility functions (string folding, I/O)
│   └── udx_btree.h/c     # B+ tree implementation (Joshua J Baker)
└── include/              # Public headers
```

## Performance

| Operation | Complexity |
|-----------|------------|
| Exact lookup | O(log n) |
| Prefix match | O(log n + k) |
| Insertion | O(log n) |
| Iteration | O(n) |

*Benchmarks on a dictionary with 1 million entries:*

- Lookup: ~0.5μs per query
- Prefix match: ~1μs per query
- File size: ~40% smaller than uncompressed

## Design Decisions

### Why B+ Trees?

B+ trees provide predictable O(log n) performance for lookups and naturally support range queries and prefix matching. The static layout (pre-built during write) enables efficient zero-copy parsing during read.

### Why Chunks?

Storing data in fixed-size (64KB) chunks with independent compression provides:
- Better compression ratios (similar data compresses together)
- Random access to individual data blocks
- Memory-efficient reading (only decompress what's needed)

### Case-Insensitive Search

The library uses **word folding** (converting to lowercase) for indexing while preserving the original word case. This enables:
- Case-insensitive lookups ("Hello", "hello", "HELLO" all match)
- Preserved original forms for display
- Efficient B+ tree traversal (sorted by folded form)

## Thread Safety

libudx is **not thread-safe** by design. For concurrent access:
- Use separate `udx_reader` instances per thread
- Protect `udx_writer` with external synchronization

## Platform Support

- **Linux** ✅ (tested)
- **macOS** ✅ (tested)
- **Windows** ✅ (MSVC/MinGW, 64-bit file offsets supported)

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

```
MIT License

Copyright (c) 2026 kejinlu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

**Third-party components:**

- **B+ tree implementation**: Copyright 2020 Joshua J Baker (MIT License)
- **zlib**: Copyright 1995-2023 Jean-loup Gailly and Mark Adler (zlib License)

## Acknowledgments

- [B+ tree implementation](https://github.com/tidwall/btree) by Joshua J Baker
- [zlib](https://zlib.net/) by Jean-loup Gailly and Mark Adler
- File format inspired by [GoldenDict](https://goldendict.org/)

## See Also

- [UDX Format Specification](docs/format.md) (TODO)
- [API Documentation](docs/api.md) (TODO)
- [Examples](examples/) (TODO)
