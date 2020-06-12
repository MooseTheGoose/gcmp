using System;
using System.Collections.Generic;
using System.Text;

namespace gcmp
{
    public struct LZInfo
    {
        public int term, disp;

        public LZInfo(int terminal, int displacement)
        {
            term = terminal;
            disp = displacement;
        }
    }

    public struct LZData
    {
        private const int MaxWindowSize = 18;
        private const int MinWindowSize = 3;
        private const int MaxDisp = 0x1000;

        
        List<byte> data;

        public LZData(byte[] stream)
        {
            data = new List<byte>();
            Compress(stream);
        }

        public int MemCmp(byte[] b1, byte[] b2, int b1s, int b2s, int n)
        {
            int cmp = 0;

            for (int i = 0; i < n; i++)
            {
                int entry1 = b1[b1s + i];
                int entry2 = b2[b2s + i];
                if (entry1 != entry2) { cmp = entry2 - entry1; break; }
            }

            return cmp;
        }

        public void Compress(byte[] stream)
        {
            SortedList<int, LZInfo> occupiedWindows = new SortedList<int, LZInfo>();

            /* Build the windows */
            for (int ws = MaxWindowSize; ws >= MinWindowSize; ws--)
            {
                for (int wp = 0; wp < stream.Length - ws; wp++)
                {
                    int scanBound = Math.Min(wp + MaxDisp + 1, stream.Length - ws);

                    for (int scan = wp + ws; scan < scanBound; scan++)
                    {
                        if (MemCmp(stream, stream, wp, scan, ws) == 0)
                        {
                            /* Check for collisions. If there are any, move on. */
                            foreach (KeyValuePair<int, LZInfo> ent in occupiedWindows)
                            {
                                if (ent.Key <= scan && scan < ent.Value.term) 
                                {
                                    goto NextWindow;
                                }
                            }
                        }
                        occupiedWindows.Add(scan, new LZInfo(scan + ws, scan - wp));
                        NextWindow: ;
                    }
                }
            }

            /* Build the compression stream for BIOS. */
            int blockFlags = 0;
            int cFlag = 7;
            int uncompressedBegin = 0;
            List<byte> buffer = new List<byte>(16);

            foreach (KeyValuePair<int, LZInfo> window in occupiedWindows)
            {
                int uncompressedEnd = window.Key;
                for (int i = uncompressedBegin; i < uncompressedEnd; i++)
                {
                    buffer.Add(stream[i]);
                    cFlag--;
                    if (cFlag < 0) 
                    {
                        data.Add((byte)blockFlags);
                        blockFlags = 0;
                        cFlag = 7;
                        data.AddRange(buffer.ToArray());
                        buffer.Clear();
                    }
                }

                blockFlags |= (1 << cFlag);
                int dispData = window.Value.disp - 1;
                int copyData = window.Value.term - window.Key - 3;
                buffer.Add((byte)(dispData >> 8 | copyData << 4));
                buffer.Add((byte)dispData);
                cFlag--;
                if (cFlag < 0)
                {
                    data.Add((byte)blockFlags);
                    blockFlags = 0;
                    cFlag = 7;
                    data.AddRange(buffer.ToArray());
                    buffer.Clear();
                }

                uncompressedBegin = window.Value.term;
            }

            for (int i = uncompressedBegin; i < stream.Length; i++)
            {
                buffer.Add(stream[i]);
                cFlag--;
                if (cFlag < 0)
                {
                    data.Add((byte)blockFlags);
                    blockFlags = 0;
                    cFlag = 7;
                    data.AddRange(buffer.ToArray());
                    buffer.Clear();
                }
            }

            data.Add((byte)blockFlags);
            data.AddRange(buffer.ToArray());
        }
    }
}
