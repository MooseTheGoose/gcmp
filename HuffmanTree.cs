using System;
using System.Collections.Generic;
using System.Text;

namespace gcmp
{
    /*
     * NOTE: Huffman Tree only supports compression of ASCII characters.
     *
     * INTERNAL NODES: symbol < 0
     * EXTERNAL NODES: 0 <= symbol <= 255
     */
    public class HuffmanTree
    {
        public int symbol;
        public int occurences;
        public HuffmanTree left;
        public HuffmanTree right;

        public HuffmanTree(int symbol, int occurences, HuffmanTree left, HuffmanTree right)
        {
            this.symbol = symbol;
            this.occurences = occurences;
            this.left = left;
            this.right = right;
        }

        public static HuffmanTree Construct(Dictionary<int, int> frequencyTable)
        {
            HuffmanTree root;
            List<HuffmanTree> nodes = new List<HuffmanTree>();

            foreach(KeyValuePair<int, int> pair in frequencyTable)
            {
                nodes.Add(new HuffmanTree(pair.Key, pair.Value, null, null));
            }

            /* Bubble sort dat by occurence, max is first. */
            for (int i = 0; i < nodes.Count; i++)
            {
                for (int j = 0; j < nodes.Count-1-i; j++)
                {
                    if (nodes[j].occurences < nodes[j+1].occurences)
                    {
                        HuffmanTree temp = nodes[j];
                        nodes[j] = nodes[j + 1];
                        nodes[j + 1] = temp;
                    }
                }
            }

            for (int i = nodes.Count - 1; i >= 1; i--)
            {
                HuffmanTree left = nodes[i];
                HuffmanTree right = nodes[i-1];

                nodes.RemoveAt(i);
                nodes.RemoveAt(i - 1);
                HuffmanTree newNode = new HuffmanTree(-1, left.occurences + right.occurences, left, right);
                nodes.Add(newNode);

                for (int j = i-1; j >= 1; j--)
                {
                    HuffmanTree temp;
                    if (nodes[j].occurences < nodes[j - 1].occurences) { break; }
                    temp = nodes[j];
                    nodes[j] = nodes[j - 1];
                    nodes[j - 1] = temp;
                }
            }

            root = nodes[0];
            return root;
        }

        public int CompressChar(byte[] bits, int currLen, byte c)
        {
            if (this.symbol == c)
            {
                return currLen;
            }
            else if(this.symbol < 0)
            {
                int recurLen = currLen + 1;
                bits[currLen] = 0;
                int finalLen = this.left.CompressChar(bits, recurLen, c);

                if (finalLen == 0)
                {
                    bits[currLen] = 1;
                    return this.right.CompressChar(bits, recurLen, c);
                }

                return finalLen;
            }

            return 0;
        }

        public uint[] Compress(byte[] text)
        {
            List<uint> bits = new List<uint>();
            bits.Add(0);
            int nbits = 0;
            byte[] charBits = new byte[256];

            for (int i = 0; i < text.Length; i++)
            {
                int final = CompressChar(charBits, 0, text[i]);

                for (int j = 0; j < final; j++)
                {
                    bits[nbits >> 5] |= (uint)charBits[j] << (~nbits & 31);

                    nbits++;
                    if ((nbits & 31) == 0) { bits.Add(0); }
                }
            }

            return bits.ToArray();
        }

        /*
         * Not strictly necessary. This is a sanity check method.
         * Consider making this a method in a separate class.
         */
        public byte[] Decompress(uint[] bits, int decompLength)
        {
            byte[] builder = new byte[decompLength];
            HuffmanTree currNode = this;
            int bitpos = 0;

            for (int i = 0; i < decompLength; i++)
            {
                while (true)
                {
                    uint currBit = bits[bitpos >> 5] >> (~bitpos & 31) & 1;

                    if (currNode.symbol < 0)
                    {
                        if (currBit == 1)
                        {
                            currNode = currNode.right;
                        }
                        else
                        {
                            currNode = currNode.left;
                        }
                    }
                    else
                    {
                        builder[i] = (byte)currNode.symbol;
                        currNode = this;
                        break;
                    }

                    bitpos++;
                }
            }

            return builder;
        }

        public static void PrintTree(HuffmanTree tree, int indent)
        {
            StringBuilder builder = new StringBuilder("");
            for (int i = 0; i < indent; i++) { builder.Append(' '); }

            if (tree.symbol < 0)
            {
                Console.WriteLine(builder + "(NON-DATA n='" + tree.occurences + "') {");
                PrintTree(tree.left, indent + 4);
                PrintTree(tree.right, indent + 4);
                Console.WriteLine(builder + "}");
            }
            else
            {
                Console.WriteLine(builder + "(DATA)" + (char)tree.symbol + " " + tree.occurences +  "(/DATA)");
            }
        }
    }
}
