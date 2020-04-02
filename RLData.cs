using System;
using System.Collections.Generic;
using System.Text;

namespace gcmp
{
    public struct RLData
    {
        private const int MinRepeat = 3;
        private const int MaxExpandLen = 127;
        public List<byte> data;

        public RLData(byte[] stream)
        {
            data = new List<byte>();
            Compress(stream);
        }

        public void Compress(byte[] stream)
        {
            int currLength = 0;
            int i;

            for (i = 0; i < stream.Length-2; i++)
            {
                if (currLength == MaxExpandLen + 1)
                {
                    data.Add((byte)(currLength - 1));
                    for (int j = i - currLength; j < i; j++) { data.Add(stream[j]); }
                    currLength = -1;
                }

                if (stream[i] == stream[i + 1] && stream[i + 1] == stream[i + 2])
                {
                    byte repeat = stream[i];

                    if (currLength != 0)
                    {
                        data.Add((byte)(currLength - 1));
                        for (int j = i - currLength; j < i; j++) { data.Add(stream[j]);  }  
                    }

                    currLength = 3;
                    while (i < stream.Length - 3 && repeat == stream[i + 3] && currLength <= MaxExpandLen + 3)
                    {
                        currLength++;
                        i++;
                    }

                    data.Add((byte)(currLength - 3 | 0x80));
                    data.Add(repeat);
                    i += 3;
                    currLength = -1;
                }

                currLength++;
            }

            i -= currLength;
            if (i < stream.Length) 
            {
                data.Add((byte)(currLength - 1));
                for (int j = i; j < stream.Length; j++)
                {
                    data.Add(stream[j]);
                }
            }
        }
    }
}
