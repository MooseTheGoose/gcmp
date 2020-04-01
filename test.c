#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "huffman.h"
#include "gcmp_io.h"


int main(int argc, char **argv)
{
   struct FrequencyTable table;
   struct HuffmanTree *root;
   size_t i;
   assert(argc == 2);

   char *contents = read_file(argv[1]);
   init_freq(&table, 256);
   update_freq(&table, contents);

   root = build_huffman(&table);
   destroy_huffman(root);

   for(i = 0; i < table.len; i++)
   {
        printf("%c %d\n", table.symbols[i], table.occurences[i]);
   }

   return 0;
}

