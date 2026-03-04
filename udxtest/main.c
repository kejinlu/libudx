//
//  main.c
//  udxtest
//
//  Created by kejinlu on 2026/2/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "udx_writer.h"
#include "udx_reader.h"

// ============================================================
// Helper Functions
// ============================================================

static const char *get_tmpdir(void) {
    const char *dir = getenv("TMPDIR");
    if (!dir) dir = getenv("TEMP");
    if (!dir) dir = getenv("TMP");
    if (!dir) dir = "/tmp";
    return dir;
}

static void print_separator(const char *title) {
    printf("\n");
    printf("========================================\n");
    printf("  %s\n", title);
    printf("========================================\n");
}

static void print_db_entry(const udx_db_entry *entry) {
    if (entry == NULL) {
        printf("  (null)\n");
        return;
    }
    printf("  Word: \"%s\"\n", entry->word ? entry->word : "(null)");
    printf("  Item Count: %zu\n", entry->items.size);
    for (size_t i = 0; i < entry->items.size; i++) {
        printf("  Item [%zu]:\n", i);
        printf("    Original: \"%s\"\n", entry->items.data[i].original_word);
        printf("    Data Size: %zu bytes\n", entry->items.data[i].size);
        if (entry->items.data[i].data && entry->items.data[i].size > 0) {
            printf("    Data: ");
            size_t show_bytes = entry->items.data[i].size < 32 ? entry->items.data[i].size : 32;
            for (size_t j = 0; j < show_bytes; j++) {
                printf("%02X ", entry->items.data[i].data[j]);
            }
            if (entry->items.data[i].size > 32) printf("...");
            printf("\n");
            // Try to print as string if it looks like text
            if (entry->items.data[i].size > 0 &&
                entry->items.data[i].data[0] >= 32 && entry->items.data[i].data[0] < 127) {
                printf("    As String: \"%.*s\"\n",
                       (int)entry->items.data[i].size,
                       (char*)entry->items.data[i].data);
            }
        }
    }
}

// ============================================================
// Test Functions
// ============================================================

static int test_write(const char *filename) {
    print_separator("TEST: Writing UDX File");

    printf("Creating UDX file: %s\n", filename);

    // Open writer
    udx_writer *writer = udx_writer_open(filename);
    if (writer == NULL) {
        fprintf(stderr, "Failed to create writer\n");
        return -1;
    }
    printf("Writer created successfully\n");

    // Create first dictionary: English
    print_separator("Creating Dictionary: 'english'");
    const char *metadata_en = "Source: English Dictionary\nVersion: 1.0";
    udx_db_builder *english = udx_db_builder_create_with_metadata(writer, "english",
                                                                  (const uint8_t*)metadata_en,
                                                                  strlen(metadata_en) + 1);
    if (english == NULL) {
        fprintf(stderr, "Failed to create english dictionary\n");
        udx_writer_close(writer);
        return -1;
    }
    printf("English dictionary created with metadata\n");

    // Add some entries
    printf("\nAdding entries to english dictionary...\n");

    udx_db_builder_add_entry(english, "apple",  (const uint8_t*)"A red or green fruit", 20);
    udx_db_builder_add_entry(english, "application", (const uint8_t*)"A computer program", 22);
    udx_db_builder_add_entry(english, "apply", (const uint8_t*)"To put into use", 17);
    udx_db_builder_add_entry(english, "banana", (const uint8_t*)"A yellow curved fruit", 24);
    udx_db_builder_add_entry(english, "book", (const uint8_t*)"A written or printed work", 27);
    udx_db_builder_add_entry(english, "cat", (const uint8_t*)"A small domesticated carnivore", 31);
    udx_db_builder_add_entry(english, "dog", (const uint8_t*)"A man's best friend", 21);
    udx_db_builder_add_entry(english, "elephant", (const uint8_t*)"A large gray mammal", 21);

    printf("Added 8 entries\n");

    // Finish english dictionary
    int ret = udx_db_builder_finish(english);
    if (ret != 0) {
        fprintf(stderr, "Failed to finish english dictionary: %d\n", ret);
        udx_writer_close(writer);
        return -1;
    }
    printf("English dictionary finished successfully\n");

    // Create second dictionary: Chinese (Pinyin)
    print_separator("Creating Dictionary: 'chinese'");
    const char *metadata_zh = "Source: Chinese Dictionary\nType: Pinyin";
    udx_db_builder *chinese = udx_db_builder_create_with_metadata(writer, "chinese",
                                                                  (const uint8_t*)metadata_zh,
                                                                  strlen(metadata_zh) + 1);
    if (chinese == NULL) {
        fprintf(stderr, "Failed to create chinese dictionary\n");
        udx_writer_close(writer);
        return -1;
    }
    printf("Chinese dictionary created with metadata\n");

    // Add some entries
    printf("\nAdding entries to chinese dictionary...\n");

    udx_db_builder_add_entry(chinese, "nihao", (const uint8_t*)"hello", 6);
    udx_db_builder_add_entry(chinese, "xieexie", (const uint8_t*)"thank you", 10);
    udx_db_builder_add_entry(chinese, "zhongguo", (const uint8_t*)"china", 6);
    udx_db_builder_add_entry(chinese, "meiguo", (const uint8_t*)"USA", 4);
    udx_db_builder_add_entry(chinese, "shijie", (const uint8_t*)"world", 6);

    printf("Added 5 entries\n");

    // Finish chinese dictionary
    ret = udx_db_builder_finish(chinese);
    if (ret != 0) {
        fprintf(stderr, "Failed to finish chinese dictionary: %d\n", ret);
        udx_writer_close(writer);
        return -1;
    }
    printf("Chinese dictionary finished successfully\n");

    // Close writer
    print_separator("Closing Writer");
    ret = udx_writer_close(writer);
    if (ret != 0) {
        fprintf(stderr, "Failed to close writer: %d\n", ret);
        return -1;
    }
    printf("Writer closed successfully\n");
    printf("File written: %s\n", filename);

    return 0;
}

static int test_read(const char *filename) {
    print_separator("TEST: Reading UDX File");

    printf("Opening UDX file: %s\n", filename);

    // Open reader
    udx_reader *reader = udx_reader_open(filename);
    if (reader == NULL) {
        fprintf(stderr, "Failed to open reader\n");
        return -1;
    }
    printf("Reader opened successfully\n");

    // List available dictionaries
    printf("\nAvailable dictionaries (%u):\n", udx_reader_get_db_count(reader));
    for (uint32_t i = 0; i < udx_reader_get_db_count(reader); i++) {
        printf("  [%u] Name: \"%s\", Offset: %llu\n",
               i, udx_reader_get_db_name(reader, i),
               (unsigned long long)udx_reader_get_db_offset(reader, i));
    }

    // Test english dictionary
    print_separator("Reading Dictionary: 'english'");
    udx_db *english = udx_db_open(reader, "english");
    if (english == NULL) {
        fprintf(stderr, "Failed to open english dictionary\n");
        udx_reader_close(reader);
        return -1;
    }
    printf("English dictionary opened successfully\n");

    // Print dictionary info
    printf("\nDictionary Information:\n");
    printf("  Name: %s\n", udx_db_get_name(english) ? udx_db_get_name(english) : "(null)");
    printf("  Entry Count: %u\n", udx_db_get_index_entry_count(english));
    printf("  Address Count: %u\n", udx_db_get_index_item_count(english));
    printf("  Tree Height: %u\n", udx_db_get_index_bptree_height(english));

    // Get metadata
    printf("\nReading Metadata...\n");
    uint32_t metadata_size = 0;
    const uint8_t *metadata = udx_db_get_metadata(english, &metadata_size);
    if (metadata != NULL && metadata_size > 0) {
        printf("Metadata (%u bytes):\n", metadata_size);
        printf("  %s\n", (char*)metadata);
    } else {
        printf("No metadata\n");
    }

    // Test lookups
    print_separator("Testing Word Lookup");
    struct {
        const char *word;
        bool should_exist;
    } test_words[] = {
        { "apple", true },
        { "banana", true },
        { "cat", true },
        { "dog", true },
        { "zebra", false },  // Not in dictionary
        { "Apple", true },   // Case insensitive
        { "BANANA", true },  // Case insensitive
        { NULL, false }
    };

    for (int i = 0; test_words[i].word != NULL; i++) {
        const char *word = test_words[i].word;
        printf("\nLooking up: \"%s\"\n", word);

        udx_db_entry *entry = udx_db_lookup(english, word);
        if (entry != NULL) {
            printf("  Found:\n");
            print_db_entry(entry);
            udx_db_entry_free(entry);
        } else {
            if (test_words[i].should_exist) {
                printf("  NOT FOUND (expected to exist!)\n");
            } else {
                printf("  NOT FOUND (as expected)\n");
            }
        }
    }

    // Test prefix matching
    print_separator("Testing Prefix Match");
    const char *prefix = "app";
    printf("Finding words starting with: \"%s\"\n", prefix);

    // Use index prefix match, then load data for each result
    udx_index_entry_array index_results = udx_db_index_prefix_match(english, prefix, 0);
    printf("Found %zu results:\n", index_results.size);
    for (size_t i = 0; i < index_results.size; i++) {
        printf("\n  [%zu] ", i + 1);
        // Load data for this entry
        udx_db_entry *entry = udx_db_entry_from_index(english, &index_results.data[i]);
        if (entry != NULL) {
            print_db_entry(entry);
            udx_db_entry_free(entry);
        }
    }
    udx_index_entry_array_free_contents(&index_results);

    // Test iterator
    print_separator("Testing Iterator");
    printf("Iterating through all entries:\n\n");

    udx_db_iter *iter = udx_db_iter_create(english);
    if (iter != NULL) {
        size_t count = 0;
        const udx_db_entry *entry;
        while ((entry = udx_db_iter_next(iter)) != NULL) {
            printf("[%zu] ", ++count);
            printf("Word: \"%s\"", entry->word ? entry->word : "(null)");
            // Show first item's data
            if (entry->items.size > 0 && entry->items.data[0].data && entry->items.data[0].size > 0) {
                printf(" -> \"%.*s\"", (int)entry->items.data[0].size, (char*)entry->items.data[0].data);
            }
            printf("\n");
        }
        udx_db_iter_destroy(iter);
        printf("\nTotal entries iterated: %zu\n", count);
    }

    // Close english dictionary
    printf("\nClosing english dictionary...\n");
    udx_db_close(english);

    // Test chinese dictionary
    print_separator("Reading Dictionary: 'chinese'");
    udx_db *chinese = udx_db_open(reader, "chinese");
    if (chinese == NULL) {
        fprintf(stderr, "Failed to open chinese dictionary\n");
        udx_reader_close(reader);
        return -1;
    }
    printf("Chinese dictionary opened successfully\n");

    // Test lookups in chinese
    printf("\nLooking up: \"nihao\"\n");
    udx_db_entry *entry = udx_db_lookup(chinese, "nihao");
    if (entry != NULL) {
        printf("  Found:\n");
        print_db_entry(entry);
        udx_db_entry_free(entry);
    }

    printf("\nLooking up: \"zhongguo\"\n");
    entry = udx_db_lookup(chinese, "zhongguo");
    if (entry != NULL) {
        printf("  Found:\n");
        print_db_entry(entry);
        udx_db_entry_free(entry);
    }

    // Close chinese dictionary
    printf("\nClosing chinese dictionary...\n");
    udx_db_close(chinese);

    // Test opening default (first) dictionary
    print_separator("Testing Default Dictionary");
    udx_db *default_db = udx_db_open(reader, NULL);
    if (default_db != NULL) {
        printf("Default dictionary opened: \"%s\"\n", udx_db_get_name(default_db));
        udx_db_close(default_db);
    }

    // Close reader
    print_separator("Closing Reader");
    udx_reader_close(reader);
    printf("Reader closed successfully\n");

    return 0;
}

static int test_large_scale(const char *filename) {
    print_separator("TEST: Large Scale Test");

    printf("Creating large dictionary...\n");

    udx_writer *writer = udx_writer_open(filename);
    if (writer == NULL) {
        fprintf(stderr, "Failed to create writer\n");
        return -1;
    }

    udx_db_builder *builder = udx_db_builder_create(writer, "large_test");
    if (builder == NULL) {
        udx_writer_close(writer);
        return -1;
    }

    // Add many entries
    const size_t num_entries = 1000;
    printf("Adding %zu entries...\n", num_entries);

    char word[64];
    char data[128];

    for (size_t i = 0; i < num_entries; i++) {
        // Create unique words
        snprintf(word, sizeof(word), "word%zu", i);
        snprintf(data, sizeof(data), "Definition for word %zu", i);

        udx_db_builder_add_entry(builder, word, (const uint8_t*)data, strlen(data) + 1);

        if ((i + 1) % 100 == 0) {
            printf("  Added %zu/%zu entries...\n", i + 1, num_entries);
        }
    }

    printf("Finishing dictionary...\n");
    int ret = udx_db_builder_finish(builder);
    if (ret != 0) {
        fprintf(stderr, "Failed to finish dictionary: %d\n", ret);
        udx_writer_close(writer);
        return -1;
    }

    printf("Closing writer...\n");
    udx_writer_close(writer);

    // Now test reading and searching
    printf("\nReading back large dictionary...\n");
    udx_reader *reader = udx_reader_open(filename);
    if (reader == NULL) {
        fprintf(stderr, "Failed to open reader\n");
        return -1;
    }

    udx_db *db = udx_db_open(reader, "large_test");
    if (db == NULL) {
        fprintf(stderr, "Failed to open dictionary\n");
        udx_reader_close(reader);
        return -1;
    }

    printf("Dictionary info:\n");
    printf("  Entry count: %u\n", udx_db_get_index_entry_count(db));
    printf("  Tree height: %u\n", udx_db_get_index_bptree_height(db));

    // Test random lookups
    printf("\nTesting random lookups...\n");
    for (int i = 0; i < 10; i++) {
        size_t idx = (size_t)(rand() % num_entries);
        snprintf(word, sizeof(word), "word%zu", idx);

        udx_db_entry *entry = udx_db_lookup(db, word);
        if (entry != NULL) {
            printf("  [%d] Found: \"%s\"\n", i + 1, entry->word);
            udx_db_entry_free(entry);
        } else {
            printf("  [%d] NOT FOUND: \"%s\" (ERROR!)\n", i + 1, word);
        }
    }

    // Test prefix match
    printf("\nTesting prefix match for \"word1\"...\n");
    udx_index_entry_array index_results = udx_db_index_prefix_match(db, "word1", 20);
    printf("Found %zu results (max 20):\n", index_results.size);
    for (size_t i = 0; i < index_results.size && i < 20; i++) {
        printf("  [%zu] %s\n", i + 1, index_results.data[i].word);
    }
    udx_index_entry_array_free_contents(&index_results);

    udx_db_close(db);
    udx_reader_close(reader);

    printf("\nLarge scale test completed successfully!\n");
    return 0;
}

static int test_error_cases(const char *filename) {
    print_separator("TEST: Error Cases");

    printf("Testing various error conditions...\n\n");

    // Test opening non-existent file
    printf("1. Opening non-existent file...\n");
    char nonexistent_file[512];
    snprintf(nonexistent_file, sizeof(nonexistent_file), "%s/nonexistent_file.udx", get_tmpdir());
    udx_reader *reader = udx_reader_open(nonexistent_file);
    if (reader == NULL) {
        printf("   Correctly returned NULL for non-existent file\n");
    } else {
        printf("   ERROR: Should have returned NULL!\n");
        udx_reader_close(reader);
    }

    // Test looking up non-existent word
    printf("\n2. Looking up non-existent word in existing file...\n");
    reader = udx_reader_open(filename);
    if (reader != NULL) {
        udx_db *db = udx_db_open(reader, NULL);
        if (db != NULL) {
            udx_db_entry *entry = udx_db_lookup(db, "thisworddoesnotexist");
            if (entry == NULL) {
                printf("   Correctly returned NULL for non-existent word\n");
            } else {
                printf("   ERROR: Should have returned NULL!\n");
                udx_db_entry_free(entry);
            }
            udx_db_close(db);
        }
        udx_reader_close(reader);
    }

    // Test opening non-existent dictionary name
    printf("\n3. Opening non-existent dictionary...\n");
    reader = udx_reader_open(filename);
    if (reader != NULL) {
        udx_db *db = udx_db_open(reader, "nonexistent_dict");
        if (db == NULL) {
            printf("   Correctly returned NULL for non-existent dictionary\n");
        } else {
            printf("   ERROR: Should have returned NULL!\n");
            udx_db_close(db);
        }
        udx_reader_close(reader);
    }

    // Test adding entries with NULL parameters
    printf("\n4. Testing NULL parameter handling...\n");
    char null_params_file[512];
    snprintf(null_params_file, sizeof(null_params_file), "%s/test_null_params.udx", get_tmpdir());
    udx_writer *writer = udx_writer_open(null_params_file);
    if (writer != NULL) {
        // NULL word should fail
        int ret = udx_db_builder_add_entry(NULL, "test", (const uint8_t*)"data", 5);
        printf("   udx_db_builder_add_entry with NULL builder: %d (expected -1)\n", ret);

        udx_db_builder *builder = udx_db_builder_create(writer, "test");
        if (builder != NULL) {
            ret = udx_db_builder_add_entry(builder, NULL, (const uint8_t*)"data", 5);
            printf("   udx_db_builder_add_entry with NULL word: %d (expected -1)\n", ret);

            ret = udx_db_builder_add_entry(builder, "test", NULL, 5);
            printf("   udx_db_builder_add_entry with NULL data: %d (expected -1)\n", ret);

            udx_db_builder_finish(builder);
        }
        udx_writer_close(writer);
    }

    printf("\nError case tests completed!\n");
    return 0;
}

// ============================================================
// Main
// ============================================================

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║   libudx - Complete Test Program               ║\n");
    printf("║   Universal Dictionary eXchange                ║\n");
    printf("╚════════════════════════════════════════════════╝\n");

    const char *tmpdir = get_tmpdir();

    char test_file[512];
    char large_test_file[512];
    snprintf(test_file, sizeof(test_file), "%s/test.udx", tmpdir);
    snprintf(large_test_file, sizeof(large_test_file), "%s/test_large.udx", tmpdir);

    int result = 0;

    // Run basic write test
    result = test_write(test_file);
    if (result != 0) {
        fprintf(stderr, "\n❌ Write test FAILED!\n");
        return 1;
    }

    // Run basic read test
    result = test_read(test_file);
    if (result != 0) {
        fprintf(stderr, "\n❌ Read test FAILED!\n");
        return 1;
    }

    // Run large scale test
    result = test_large_scale(large_test_file);
    if (result != 0) {
        fprintf(stderr, "\n❌ Large scale test FAILED!\n");
        return 1;
    }

    // Run error cases test
    result = test_error_cases(test_file);
    if (result != 0) {
        fprintf(stderr, "\n❌ Error cases test FAILED!\n");
        return 1;
    }

    print_separator("All Tests PASSED!");
    printf("✅ All tests completed successfully!\n\n");

    return 0;
}
