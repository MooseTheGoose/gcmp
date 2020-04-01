#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string.h>
#include <sys/types.h>

/*
 *  EXTERNAL NODES: 0 <= sym <= 255
 *  INTERNAL NODES: sym < 0
 */

struct HuffmanTree
{
   int sym;
   size_t occurences;
   struct HuffmanTree *left;
   struct HuffmanTree *right;
};

struct FrequencyTable
{
   int *symbols;
   int *occurences;
   size_t len;
   size_t allocated;
};

void init_freq(struct FrequencyTable *table, size_t allocate);
void update_freq(struct FrequencyTable *table, unsigned char *input);
void destroy_freq(struct FrequencyTable *table);
void add_char_freq(struct FrequencyTable *table, int ch);
struct HuffmanTree *build_huffman(struct FrequencyTable *finalTable);
void destroy_huffman(struct HuffmanTree *huffman);

#endif

