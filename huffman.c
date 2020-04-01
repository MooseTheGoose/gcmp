#include "huffman.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void init_freq(struct FrequencyTable *table, size_t alloc)
{
    table->symbols = malloc(alloc * sizeof(*table->symbols));
    table->occurences = malloc(alloc * sizeof(*table->occurences));
    table->len = 0;
    table->allocated = alloc;
    
    if(!table->symbols || !table->occurences)	     
    {
	fprintf(stderr, "Unable to allocate table of size %ld", alloc); 
        exit(-1);
    }
}

void add_char_freq(struct FrequencyTable *table, int ch)
{
    size_t len = table->len;
    int i;
    for(i = 0; i < len; i++)
    {
        if(ch == table->symbols[i])
        {
	    table->occurences[i]++;
	    break;
	}
    }

    if(i == len)
    {
       table->symbols[len] = ch;
       table->occurences[len] = 1;
       len++;
    }

    table->len = len;
}

void update_freq(struct FrequencyTable *table, unsigned char *input)
{
    int currC = *input++;

    while(currC)
    {
        add_char_freq(table, currC);
	currC = *input++;
    }
}

void destroy_freq(struct FrequencyTable *table)
{
    free(table->symbols);
    free(table->occurences);
}

#define SWAPFREQ(a, b) do { int __temp = a; a = b; b = __temp; } while(0);
#define SWAPNODE(a, b) do { struct HuffmanTree *__temp = a; a = b; b = __temp; } while(0);

struct HuffmanTree *build_huffman(struct FrequencyTable *finalTable)
{
    size_t i, j;
    size_t len = finalTable->len;
    struct HuffmanTree **queue = malloc(len * sizeof(void *));
    struct HuffmanTree *root;

    assert(queue);

    /* Bubble sort, First is max. */
    for(i = 0; i < len; i++)
    {
        for(j = 0; j < len-1-i; j++)
        {
            int locc = finalTable->occurences[j], rocc = finalTable->occurences[j+1];
            if(locc < rocc)
            {
                SWAPFREQ(finalTable->symbols[j], finalTable->symbols[j + 1]);
                SWAPFREQ(finalTable->occurences[j], finalTable->occurences[j + 1]);
            }
        }
    }

    for(i = 0; i < len; i++)
    {
        struct HuffmanTree *curr = malloc(sizeof(struct HuffmanTree));
        assert(curr);
        curr->sym = finalTable->symbols[i];
        curr->occurences = finalTable->occurences[i];
        queue[i] = curr;
    }

    for(i = len-1; i >= 1; i--)
    {
        struct HuffmanTree *curr = malloc(sizeof(struct HuffmanTree));
        struct HuffmanTree *left = queue[i-1];
        struct HuffmanTree *right = queue[i];
        assert(curr);

        curr->sym = -1;
        curr->occurences = left->occurences + right->occurences;
        curr->right = right;
        curr->left = left;
        
        /* Remove last 2 elements, insert, and sort. */
        queue[i-1] = curr;
        for(j = i-1; j >= 1; j--)
        {
            if(queue[j]->occurences < queue[j-1]->occurences) { break; }
            SWAPNODE(queue[j], queue[j-1]); 
        }
    }
    
    root = queue[0];
    free(queue);
    return root;
}

void destroy_huffman(struct HuffmanTree *huffman)
{
    if(huffman->sym < 0)
    {
        destroy_huffman(huffman->left);
        destroy_huffman(huffman->right);
    }
    free(huffman);
}
