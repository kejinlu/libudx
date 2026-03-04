//
//  udx_words.h
//  libudx
//
//  Created by kejinlu on 2026/2/25.
//

#ifndef udx_words_h
#define udx_words_h

#include <stdint.h>
#include <stddef.h>
#include "udx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Ordered Word Container
// ============================================================
typedef struct udx_words udx_words;

// ============================================================
// Creation and Destruction
// ============================================================

/**
 * Create an ordered word container
 * @return Container pointer, or NULL on failure
 */
udx_words *udx_words_create(void);

/**
 * Destroy an ordered word container
 * @param words Container pointer
 */
void udx_words_destroy(udx_words *words);

// ============================================================
// Word Operations
// ============================================================

/**
 * Add a word
 * @param words Container pointer
 * @param word Word (UTF-8)
 * @param data_address Data address
 * @param data_size Data size in bytes
 * @return true on success, false on failure
 *
 * @note The word will be folded (lowercased, etc.) for sorting and lookup
 * @note Multiple addresses can be stored under the same word
 */
bool udx_words_add(udx_words *words,
                          const char *word,
                          udx_chunk_address data_address,
                          uint32_t data_size);

/**
 * Get the number of unique words
 * @param words Container pointer
 * @return Number of unique words
 */
size_t udx_words_count(const udx_words *words);

/**
 * Get the total number of items
 * @param words Container pointer
 * @return Total number of items across all words
 */
size_t udx_words_item_count(const udx_words *words);

// ============================================================
// Iterator
// ============================================================

/**
 * Iterator type
 */
typedef struct udx_words_iter udx_words_iter;

/**
 * Create an iterator
 * @param words Container pointer
 * @return Iterator pointer, or NULL on failure
 */
udx_words_iter *udx_words_iter_create(udx_words *words);

/**
 * Destroy an iterator
 * @param iter Iterator pointer
 */
void udx_words_iter_destroy(udx_words_iter *iter);

/**
 * Get the next entry
 * @param iter Iterator pointer
 * @return Entry pointer, or NULL when traversal is complete
 */
const udx_index_entry *udx_words_iter_next(udx_words_iter *iter);

/**
 * Peek at the current entry (without advancing the iterator)
 * @param iter Iterator pointer
 * @return Entry pointer, or NULL when traversal is complete
 */
const udx_index_entry *udx_words_iter_peek(udx_words_iter *iter);

#ifdef __cplusplus
}
#endif

#endif /* udx_words_h */
