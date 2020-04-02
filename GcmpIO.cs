using System;
using System.IO;
using System.Collections.Generic;

namespace gcmp
{
    public class GcmpIO
    {
        public static byte[] ReadFile(string fname)
        {
            List<byte> bytes = new List<byte>();
            byte[] read;

            using (BinaryReader rdr =
                new BinaryReader(new FileStream(fname, FileMode.OpenOrCreate)))
                do
                {
                    read = rdr.ReadBytes(512);
                    bytes.AddRange(read);
                }
                while (read.Length == 512);

            return bytes.ToArray();
        }

        public static List<byte[]> SplitLines(byte[] stream)
        {
            List<byte[]> lines = new List<byte[]>();

            int j = 0, i = 0;

            for (i = 0; i < stream.Length; i++)
            {
                if (stream[i] == '\n')
                {
                    lines.Add(new byte[i - j]);
                    Array.Copy(stream, i, lines[lines.Count - 1], 0, i - j);
                    j = i + 1;
                }
            }

            lines.Add(new byte[j - i]);
            Array.Copy(stream, i, lines[lines.Count - 1], 0, j - i);

            return lines;
        }

        public static void OutputHuffmanBiosAssembly(string asmName, HuffmanBiosTree tree)
        {
            int nNodes = (tree.nodes.Count + 4 & ~3) - 1; /* Huffman number of nodes divisible by 4 for alignment. */
            int huffmanTreeSize = (nNodes >> 1) - 1;
            byte[] nodes = tree.nodes.ToArray();

            using (StreamWriter sw = new StreamWriter(asmName))
            {
                sw.WriteLine("EXTERN HuffmanTreeSize");
                sw.WriteLine("EXTERN HuffmanTree");
                sw.WriteLine();
                sw.WriteLine("HuffmanTreeSize:");
                sw.WriteLine("    DB " + String.Format("0x{0:X2}", huffmanTreeSize));
                sw.WriteLine();
                sw.WriteLine("HuffmanTree:");
                for (int i = 0; i < nodes.Length; i++)
                {
                    sw.WriteLine("    DB " + String.Format("0x{0:X2}", nodes[i]));
                    nNodes--;
                }
                while(nNodes-- > 0)
                {
                    sw.WriteLine("    DB 0x00 ; Filler");
                }
            }
        }

        public static void OutputCompressedBitstreamAssembly(string asmName, uint[] compressed, string compressedName)
        {
            int j = 0;
            
            using (StreamWriter sw = new StreamWriter(asmName))
            {
                sw.WriteLine("EXTERN " + compressedName);
                sw.WriteLine();
                sw.Write(compressedName + ":");
                for (int i = 0; i < compressed.Length; i++)
                {
                    if ((j & 7) == 0)
                    {
                        sw.WriteLine();
                        sw.Write(String.Format("    DD 0x{0:X8}", compressed[i]));
                    }
                    else
                    {
                        sw.Write(String.Format(", 0x{0:X8}", compressed[i]));
                    }
                    j++;
                }
            }
        }

        public GcmpIO()
        {
        }
    }
}
