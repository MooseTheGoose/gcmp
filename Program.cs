using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace gcmp
{
    class Program
    {
        static void Main(string[] args)
        {
            string str = "THIS IS A STRING";
            int[] compressedBits;
            Dictionary<int, int> freqTable = new Dictionary<int, int>();

            Debug.Assert(args.Length > 0);

            for (int i = 0; i < str.Length; i++)
            {
                int ch = str[i];

                if (freqTable.ContainsKey(ch))
                {
                    freqTable[ch]++;
                }
                else
                {
                    freqTable[ch] = 1;
                }
            }

            HuffmanTree root = HuffmanTree.Construct(freqTable);
            HuffmanTree.PrintTree(root, 0);
            compressedBits = root.Compress(str);
            Console.WriteLine(root.Decompress(compressedBits, str.Length));
        }
    }
}
