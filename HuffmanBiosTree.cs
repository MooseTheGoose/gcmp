using System;
using System.Collections.Generic;

namespace gcmp
{
    public struct HuffmanBiosTree
    {
        public List<byte> nodes;

        void FigureNodes(HuffmanTree node, int selfIndex)
        {
            if(node.symbol >= 0)
            {
                nodes[selfIndex] = (byte)node.symbol;
            }
            else
            {
                int node0 = nodes.Count;
                int node1 = nodes.Count + 1;
                int selfOffset = (selfIndex + 1 >> 1) - 1;
                int childrenOffset = node0 - 1 >> 1;

                if(node.left.symbol >= 0) { nodes[selfIndex] |= 0x80; }
                if(node.right.symbol >= 0) { nodes[selfIndex] |= 0x40; }
                nodes[selfIndex] |= (byte)(childrenOffset - selfOffset - 1);
                nodes.Add(0);
                nodes.Add(0);
                FigureNodes(node.left, node0);
                FigureNodes(node.right, node1);
            }
        }

        public HuffmanBiosTree(HuffmanTree tree)
        {
            nodes = new List<byte>();
            nodes.Add(0);
            if (tree.left.symbol >= 0) { nodes[0] |= 0x80; }
            if (tree.right.symbol >= 0) { nodes[0] |= 0x40; }
            nodes.Add(0);
            nodes.Add(0);
            FigureNodes(tree.left, 1);
            FigureNodes(tree.right, 2);
        }

        public void PrintBiosTree()
        {
            Console.Write(String.Format("{0:X}", (nodes.Count >> 1) - 1) + " ");
            foreach (byte node in nodes)
            {
                Console.Write(String.Format("{0:X}", node) + " ");
            }
            Console.WriteLine();
        }

        private HuffmanTree ConvertToHuffmanHelper(int selfIndex)
        {
            HuffmanTree root = new HuffmanTree(-1, 0, null, null);
            HuffmanTree left, right;
            int selfOff = (selfIndex + 1 >> 1) - 1;
            int lpos = 2 * ((nodes[selfIndex] & 0x1F) + selfOff + 1) + 1;
            int rpos = lpos + 1;

            if ((nodes[selfIndex] & 0x80) != 0)
            {
                left = new HuffmanTree(nodes[lpos], 1, null, null);
            }
            else
            {
                left = ConvertToHuffmanHelper(lpos);
            }

            if ((nodes[selfIndex] & 0x40) != 0)
            {
                right = new HuffmanTree(nodes[rpos], 1, null, null);
            }
            else
            {
                right = ConvertToHuffmanHelper(rpos);
            }

            root.left = left;
            root.right = right;
            return root;
        }

        public HuffmanTree ConvertToHuffman()
        {
            return new HuffmanTree(-1, 0, ConvertToHuffmanHelper(1), ConvertToHuffmanHelper(2));
        }
    }
}
