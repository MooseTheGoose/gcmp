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

        static void HuffmanBiosOption(string[] fnames, string outputName)
        {
            byte[][] fileBytes = new byte[fnames.Length][];
            Dictionary<int, int> table = new Dictionary<int, int>();

            for (int i = 0; i < fnames.Length; i++)
            {
                fileBytes[i] = GcmpIO.ReadFile(fnames[i]);

            }

            foreach (byte[] fileContents in fileBytes)
            {
                AddStream(table, fileContents);
            }

            HuffmanTree root = HuffmanTree.Construct(table);
            HuffmanBiosTree bios = new HuffmanBiosTree(root);

            GcmpIO.OutputHuffmanBiosAssembly(outputName, bios);
        }

        static void Main(string[] args)
        {
            Debug.Assert(args.Length == 1);
            string input = args[0];
            byte[] bytes = new byte[input.Length];
            for (int i = 0; i < input.Length; i++) { bytes[i] = (byte)input[i]; } 
            Dictionary<int, int> frequencyTable = new Dictionary<int, int>();
            AddStream(frequencyTable, bytes);
            HuffmanTree t = HuffmanTree.Construct(frequencyTable);
            HuffmanBiosTree bios = new HuffmanBiosTree(t);


            uint[] bitstream = t.Compress(bytes);
            GcmpIO.OutputHuffmanBiosAssembly("C:\\\\Users\\s198\\gcmp\\test.s", bios);
            GcmpIO.OutputCompressedBitstreamsAssembly("C:\\\\Users\\s198\\gcmp\\test_compressed.s", bitstream, 8 | 2 << 4 |  (uint)bitstream.Length * 4 << 8);
        }

        static void MainStuff(string[] args)
        {
            uint[] compressedBits;
            byte[][] fileBytes = new byte[args.Length][];
            Dictionary<int, int> table = new Dictionary<int, int>();

            for (int i = 0; i < args.Length; i++)
            {
                fileBytes[i] = GcmpIO.ReadFile(args[i]);

            }

            foreach (byte[] fileContents in fileBytes)
            {
                AddStream(table, fileContents);
            }

            HuffmanTree root = HuffmanTree.Construct(table);
            HuffmanBiosTree bios = new HuffmanBiosTree(root);

            foreach (byte[] fileContents in fileBytes)
            {
                int i = 0;
                Console.WriteLine("--------------------------------------");
                compressedBits = root.Compress(fileContents);
                foreach (byte b in root.Decompress(compressedBits, fileContents.Length))
                {
                    Console.Write((char)b);
                    i++;
                }
                Console.WriteLine();
                Console.WriteLine(i * 8 + " vs. " + compressedBits.Length * 32);
                Console.WriteLine("--------------------------------------");
            }
            HuffmanBiosOption(args, "C:\\\\Users\\s198\\gcmp\\huffmantree.asm");
        }
    }
}
