using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace gcmp
{
    class Program
    {
        static int dataSize = 8;

        static void AddByte(Dictionary<int, int> freqTable, int b)
        {
            if (freqTable.ContainsKey(b))
            {
                freqTable[b]++;
            }
            else
            {
                freqTable[b] = 1;
            }
        }

        static void AddStream(Dictionary<int, int> freqTable, byte[] stream)
        {
            foreach (byte b in stream)
            {
                AddByte(freqTable, b);
            }
        }

        static void Main(string[] args)
        {
            int[] compressedBits;
            byte[] strArray = GcmpIO.ReadFile("/Users/yoyomoose/Desktop/gcmp/test.txt");
            Dictionary<int, int> table = new Dictionary<int, int>();

            AddStream(table, strArray);
            byte[] confirm = new byte[strArray.Length];

            HuffmanTree root = HuffmanTree.Construct(table);
            HuffmanTree.PrintTree(root, 0);
            compressedBits = root.Compress(strArray);
            confirm  = root.Decompress(compressedBits, strArray.Length);
            foreach (byte b in confirm)
            {
                Console.Write((char)b);
            }
            Console.WriteLine();
            Console.WriteLine(compressedBits.Length * 32 + " vs. " + strArray.Length * 8);

            HuffmanBiosTree bios = new HuffmanBiosTree(root);
            bios.PrintBiosTree();
        }
    }
}
