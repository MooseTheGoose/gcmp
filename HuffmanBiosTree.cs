using System;
using System.Collections.Generic;

namespace gcmp
{
    struct HuffmanBiosTree
    {
        public byte treeSize;
        public List<byte> nodes;

        /* BUGGED */
        
        int FigureNodes(HuffmanTree node, int currOff)
        {
            byte currNode = 0;
            if(node.symbol >= 0)
            {
                currNode = (byte)node.symbol;
                nodes.Add(currNode);
                return currOff;
            }
            else
            {
                if(node.left.symbol >= 0) { currNode |= 0x80; }
                if(node.right.symbol >= 0) { currNode |= 0x40; }
                currNode |= (byte)(currOff);
                nodes.Add(currNode);
                currOff = FigureNodes(node.left, currOff + 1);
                return FigureNodes(node.right, currOff);
            }
        }

        public HuffmanBiosTree(HuffmanTree tree)
        {
            nodes = new List<byte>();
            treeSize = 0;
            treeSize = (byte)FigureNodes(tree, 0); 
        }

        public void PrintBiosTree()
        {
            Console.Write(String.Format("{0:X}", treeSize) + " ");
            foreach (byte node in nodes)
            {
                Console.Write(String.Format("{0:X}", node) + " ");
            }
            Console.WriteLine();
        }
    }
}
