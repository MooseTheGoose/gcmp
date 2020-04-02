using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace gcmp
{
    class Program
    {

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
            uint[] compressedBits;
            byte[] strArray = GcmpIO.ReadFile("C:\\\\Users\\s198\\gcmp\\test.txt");
            Dictionary<int, int> table = new Dictionary<int, int>();

            AddStream(table, strArray);
            byte[] confirm = new byte[strArray.Length];

            HuffmanTree root = HuffmanTree.Construct(table);
            HuffmanBiosTree bios = new HuffmanBiosTree(root);
            HuffmanTree.PrintTree(root, 0);
            HuffmanTree.PrintTree(bios.ConvertToHuffman(), 0);
            compressedBits = root.Compress(strArray);
            confirm  = bios.ConvertToHuffman().Decompress(compressedBits, strArray.Length);
            foreach (byte b in confirm)
            {
                Console.Write((char)b);
            }
            Console.WriteLine();
            Console.WriteLine(compressedBits.Length * 32 + " vs. " + strArray.Length * 8);

            bios.PrintBiosTree();
            GcmpIO.OutputHuffmanBiosAssembly("C:\\\\Users\\s198\\gcmp\\test.asm", bios);
            GcmpIO.OutputCompressedBitstreamAssembly("C:\\\\Users\\s198\\gcmp\\bitstream.asm", compressedBits, (uint)strArray.Length << 8 | 2 << 4 | 8);
        }
    }
}
