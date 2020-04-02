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

        public GcmpIO()
        {
        }
    }
}
