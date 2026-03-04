//
//  udx_types.c
//  libudx
//
//  Created by kejinlu on 2026/2/25.
//

#include "udx_types.h"
#include <stdlib.h>

void udx_entry_free_contents(udx_index_entry *entry) {
    if (entry == NULL) return;
    free(entry->word);
    entry->word = NULL;

    // Free items array
    for (size_t i = 0; i < entry->items.size; i++) {
        free(entry->items.data[i].original_word);
    }
    udx_index_entry_item_array_free(&entry->items);
}

void udx_entry_free(udx_index_entry *entry) {
    if (entry == NULL) return;
    udx_entry_free_contents(entry);
    free(entry);
}

void udx_db_entry_free_contents(udx_db_entry *entry) {
    if (entry == NULL) return;
    free(entry->word);
    entry->word = NULL;

    // Free items array
    for (size_t i = 0; i < entry->items.size; i++) {
        free(entry->items.data[i].original_word);
        free(entry->items.data[i].data);
    }
    udx_db_entry_item_array_free(&entry->items);
}

void udx_db_entry_free(udx_db_entry *entry) {
    if (entry == NULL) return;
    udx_db_entry_free_contents(entry);
    free(entry);
}
