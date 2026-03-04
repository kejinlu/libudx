//
//  udx_writer.h
//  libudx
//
//  Created by kejinlu on 2026/2/25.
//

#ifndef udx_writer_h
#define udx_writer_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "udx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct udx_writer udx_writer;
typedef struct udx_db_builder udx_db_builder;

// ============================================================
// File-level Writer API
// ============================================================

/**
 * Open a new UDX file for writing
 * @param output_path Path to the output file (will be created or truncated)
 * @return Writer pointer, or NULL on failure (UDX_ERR_MEMORY)
 *
 * @note Only one database can be built at a time per writer
 * @note Call udx_writer_close() to finalize and close the file
 */
udx_writer *udx_writer_open(const char *output_path);

/**
 * Close the writer and finalize the UDX file
 * @param writer Writer pointer
 * @return UDX_OK on success, error code on failure:
 *         UDX_ERR_ACTIVE_DB: a database builder is still active
 *         UDX_ERR_IO: file I/O error
 *
 * @note All builders must be finished before closing
 */
udx_error_t udx_writer_close(udx_writer *writer);

// ============================================================
// Database Builder API
// ============================================================

/**
 * Create a database builder (without metadata)
 * @param writer Writer pointer
 * @param name Database name (must be unique within the file)
 * @return Builder pointer, or NULL on failure (UDX_ERR_MEMORY, UDX_ERR_ACTIVE_DB, UDX_ERR_DUPLICATE_NAME)
 *
 * @note This is equivalent to calling udx_db_builder_create_with_metadata
 *       with metadata=NULL and metadata_size=0
 */
udx_db_builder *udx_db_builder_create(udx_writer *writer, const char *name);

/**
 * Create a database builder with metadata
 * @param writer Writer pointer
 * @param name Database name (must be unique within the file)
 * @param metadata Metadata bytes (can be NULL if metadata_size is 0)
 * @param metadata_size Size of metadata in bytes (must be 0 if metadata is NULL)
 * @return Builder pointer, or NULL on failure (UDX_ERR_MEMORY, UDX_ERR_ACTIVE_DB,
 *         UDX_ERR_DUPLICATE_NAME, UDX_ERR_METADATA)
 *
 * @note Constraint: if metadata is NULL, metadata_size MUST be 0.
 *       Conversely, if metadata_size > 0, metadata MUST NOT be NULL.
 * @note Metadata is stored immediately after the database header
 * @note Only one builder can be active at a time per writer
 */
udx_db_builder *udx_db_builder_create_with_metadata(udx_writer *writer,
                                                    const char *name,
                                                    const uint8_t *metadata,
                                                    size_t metadata_size);

/**
 * Finish building the database
 * @param builder Builder pointer
 * @return UDX_OK on success, error code on failure:
 *         UDX_ERR_INVALID_PARAM: invalid parameter or empty database (no entries added)
 *         UDX_ERR_CHUNK: chunk writer failed
 *         UDX_ERR_BPTREE: B+ tree build failed
 *         UDX_ERR_HEADER: header write failed
 *
 * @note After this call, the builder is freed and must not be used again
 * @note Empty databases (with no entries) are not allowed and will return UDX_ERR_INVALID_PARAM
 */
udx_error_t udx_db_builder_finish(udx_db_builder *builder);

/**
 * Add an entry to the database
 * @param builder Builder pointer
 * @param word Word string (UTF-8, will be folded for case-insensitive lookup)
 * @param data Data bytes
 * @param data_size Size of data in bytes (must not exceed UINT32_MAX)
 * @return UDX_OK on success, error code on failure:
 *         UDX_ERR_INVALID_PARAM: invalid parameter
 *         UDX_ERR_CHUNK: chunk writer failed
 *         UDX_ERR_WORDS: words container failed
 *
 * @note Multiple entries can be added under the same word
 * @note The original word case is preserved
 */
udx_error_t udx_db_builder_add_entry(udx_db_builder *builder,
                           const char *word,
                           const uint8_t *data,
                           size_t data_size);


#ifdef __cplusplus
}
#endif

#endif /* udx_writer_h */
