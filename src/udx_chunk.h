//
//  ud_chunk.h
//  libudx
//
//  Created by kejinlu on 2026/2/25.
//

#ifndef ud_chunk_h
#define ud_chunk_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "udx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Chunk Writer
// ============================================================

typedef struct udx_chunk_writer udx_chunk_writer;

/**
 * Create a Chunk Writer
 * @param file Output file (must be opened, writable)
 * @return Writer pointer, or NULL on failure
 */
udx_chunk_writer *udx_chunk_writer_create(FILE *file);

/**
 * Destroy a Chunk Writer
 * @param writer Writer pointer
 */
void udx_chunk_writer_destroy(udx_chunk_writer *writer);

/**
 * Start a new data block
 * @param writer Writer pointer
 * @return Data block address (encoded address)
 *
 * @note If the current chunk has insufficient space, it will be flushed
 *       and a new chunk will be created automatically
 */
udx_chunk_address udx_chunk_writer_start_block(udx_chunk_writer *writer);

/**
 * Append data to the current data block
 * @param writer Writer pointer
 * @param data Data pointer
 * @param size Data size
 * @return true on success, false on failure
 *
 * @note Must call udx_chunk_writer_start_block first
 */
bool udx_chunk_writer_add_data(udx_chunk_writer *writer,
                               const uint8_t *data,
                               size_t size);

/**
 * Finish writing and write the chunk offset table
 * @param writer Writer pointer
 * @return File offset of the chunk offset table, or 0 on failure
 */
uint64_t udx_chunk_writer_finish(udx_chunk_writer *writer);

// ============================================================
// Chunk Reader
// ============================================================

typedef struct udx_chunk_reader udx_chunk_reader;

/**
 * Create a Chunk Reader
 * @param file Input file (must be opened, readable)
 * @param table_offset File offset of the chunk offset table
 * @return Reader pointer, or NULL on failure
 */
udx_chunk_reader *udx_chunk_reader_create(FILE *file, uint64_t table_offset);

/**
 * Destroy a Chunk Reader
 * @param reader Reader pointer
 */
void udx_chunk_reader_destroy(udx_chunk_reader *reader);

/**
 * Read the data block at the specified address
 * @param reader Reader pointer
 * @param address Data block address (from udx_chunk_writer_start_block)
 * @param data_size Exact size of the data block
 * @return Data pointer (caller must free), or NULL on failure
 */
uint8_t *udx_chunk_reader_get_block(udx_chunk_reader *reader,
                                    udx_chunk_address address,
                                    uint32_t data_size);

/**
 * Get the number of chunks
 * @param reader Reader pointer
 * @return Number of chunks
 */
uint64_t udx_chunk_reader_get_chunk_count(const udx_chunk_reader *reader);


#ifdef __cplusplus
}
#endif

#endif /* ud_chunk_h */
