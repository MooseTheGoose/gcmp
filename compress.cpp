#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>

struct RLRecord
{
    char byte;
    int32_t len;
};

struct LZRecord
{
    union
    {
        int32_t len;
        int32_t negative;
    }
    flag;

    union
    {
        char byte;
        int32_t disp;
    }
    data;
};

struct HuffmanRecord
{
    char byte;
    int32_t freq;
    HuffmanRecord *left;
    HuffmanRecord *right;
};

struct LZInfo
{
    int32_t max_window;
    int32_t min_window;
    int32_t max_disp;
    int32_t min_disp; 
};

std::vector<RLRecord> RLCompress(const std::vector<char> bytes)
{
    int ch;
    int32_t len = 1;
    std::vector<RLRecord> records = std::vector<RLRecord>();
    RLRecord curr_record;

    if(bytes.size() <= 0) { return records; }

    ch = bytes[0];
    for(int32_t i = 1; i < bytes.size(); i++)
    {
        if(bytes[i] != ch)
        {
            curr_record.byte = ch;
            curr_record.len = len;
            ch = bytes[i];
            len = 0;
            records.push_back(curr_record);
        }
        len++;
    }

    curr_record.byte = ch;
    curr_record.len = len;
    records.push_back(curr_record);   

    return records;
}

/*
 *  NOTE: LZRecords stored in reverse
 */

std::vector<LZRecord> LZCompress(const LZInfo *info, const std::vector<char> bytes)
{
    std::vector<LZRecord> records = std::vector<LZRecord>();
    LZRecord curr_record;
    int32_t end = bytes.size();
    const char *data = bytes.data();

    while(end > 0)
    {
        int32_t windowlen = info->max_window;
        int windowfound = 0;

        while(windowlen >= info->min_window)
        {
            int32_t start = end - windowlen - info->min_disp;
            int32_t window = end - windowlen;
            
            while(start > 0 && window-start <= info->max_disp)
            {
                if(!memcmp(data + start, data + window, windowlen))
                {
                    curr_record.data.disp = window-start;
                    curr_record.flag.len = windowlen;
                    records.push_back(curr_record);

                    end -= windowlen;
                    start = 1;
                    windowlen = info->min_window;
                    windowfound = 1;
                }
                start--;
            }
            windowlen--;
        }

        if(!windowfound)
        {
            curr_record.flag.negative = -1;
            curr_record.data.byte = data[end-1];
            records.push_back(curr_record);
            end--;
        }
    }

    return records;
}

int32_t HuffStream(HuffmanRecord *rec, char *buffer, int32_t index, char ch)
{
    if(rec->left)
    {
        int32_t size;
        buffer[index] = 0;
        size = HuffStream(rec->left, buffer, index+1, ch);
        if(size >= 0) { return size; }
        buffer[index] = 1;
        return HuffStream(rec->right, buffer, index+1, ch);
    }
    else
    { 
        if(rec->byte != ch) { return -1; }
        return index;
    }
}

std::vector<uint32_t> HuffmanCompress(std::vector<char> bytes,
                                      HuffmanRecord **huffptr)
{
    std::vector<HuffmanRecord *> records = std::vector<HuffmanRecord *>();
    HuffmanRecord *root;
    std::vector<uint32_t> bitstream = std::vector<uint32_t>();
    char buffer[512];

    if(bytes.size() == 0) { *huffptr = NULL; return bitstream; }

    for(int32_t i = 0; i < bytes.size(); i++)
    {
        HuffmanRecord *new_record;
        int32_t j = 0;

        for(j = 0; j < records.size(); j++)
        {
            if(records[j]->byte == bytes[i])
            { records[j]->freq++; break; }
        }

        if(j == records.size())
        {
            new_record = new HuffmanRecord();
            new_record->byte = bytes[j];
            new_record->freq = 1;
            new_record->left = 0;
            new_record->right = 0;
            records.push_back(new_record);
        }
    }

    for(int32_t i = 0; i < records.size()-1; i++)
    {
        int swapped = 0;

        for(int32_t j = 0; j < records.size()-1-i; j++)
        {
            if(records[j+1]->freq < records[j]->freq)
            {
                HuffmanRecord *temp = records[j];
                records[j] = records[j+1];
                records[j+1] = temp;
            }           
        }

        if(!swapped) { break; }
    }

    while(records.size() > 1)
    {
        HuffmanRecord *right = records[records.size()-1];
        HuffmanRecord *left = records[records.size()-2];
        HuffmanRecord *new_record = new HuffmanRecord();

        records.pop_back();
        records.pop_back();

        new_record->left = left;
        new_record->right = right;
        new_record->freq = left->freq + right->freq;
        
        records.push_back(new_record);

        for(int32_t j = records.size()-2; j >= 0; j--)
        {
            HuffmanRecord *temp;
            if(records[j+1]->freq <= records[j]->freq) break;

            temp = records[j];
            records[j] = records[j+1];
            records[j+1] = temp;            
        }
    }

    root = records[0];
    *huffptr = root;

    uint32_t currint = 0;
    int shift = 31;

    for(int32_t i = 0; i < bytes.size(); i++)
    {
        int32_t size = HuffStream(root, buffer, 0, bytes[i]);

        for(int32_t j = 0; j < size; j++)
        {
            currint |= buffer[j] << shift;
            shift--;
            if(shift < 0)
            {
                shift += 32;
                bitstream.push_back(currint);
            }
        }
    }

    if(shift != 31) { bitstream.push_back(currint); }

    return bitstream;
}

std::vector<char> Diff8Filter(std::vector<char> bytes)
{
    std::vector<char> diff = std::vector<char>();
    char refbyte;

    if(bytes.size() == 0) { return diff; }

    refbyte = bytes[0]; 
    diff.push_back(refbyte);   
    for(int32_t i = 1; i < bytes.size(); i++)
    {
        diff.push_back(bytes[i] - refbyte);
        refbyte = bytes[i];   
    }

    return diff;
}

std::vector<uint16_t> Diff16Filter(std::vector<uint16_t> shorts)
{
    std::vector<uint16_t> diff = std::vector<uint16_t>();
    uint16_t refshort;

    if(shorts.size() == 0) { return diff; }

    refshort = shorts[0];
    diff.push_back(refshort);
    for(int32_t i = 1; i < shorts.size(); i++)
    {
        diff.push_back(shorts[i] - refshort);
        refshort = shorts[i];
    }

    return diff;
}

std::vector<char> RlEncode(std::vector<RLRecord> records)
{
    std::vector<char> encoding = std::vector<char>();
    std::vector<char> uncomp = std::vector<char>();

    const int min_compress = 3;

    for(int32_t i = 0; i < records.size(); i++)
    {
        char byte = records[i].byte;
        int32_t len = records[i].len;

        if(len < min_compress)
        {
             while(len--)
             {
                 uncomp.push_back(byte);
                 if(uncomp.size() == 128)
                 {
                     encoding.push_back(0x7F);
                     for(int32_t j = 0; j < 128; j++)
                     { encoding.push_back(uncomp[j]); }
                     uncomp.clear();
                 }
             }  
        }
        else
        {
            if(uncomp.size() > 0)
            {
                encoding.push_back(uncomp.size() - 1);
                for(int32_t j = 0; j < uncomp.size(); j++)
                { encoding.push_back(uncomp[j]); }
                uncomp.clear();
            }

            while(len > min_compress + 127)
            {
                encoding.push_back(0xFF);
                encoding.push_back(byte);
                len -= min_compress + 127;
            }
            encoding.push_back((len - min_compress) | 0x80);
            encoding.push_back(byte);
        }
    }

    if(uncomp.size() > 0)
    {
        encoding.push_back(uncomp.size() - 1);
        for(int32_t j = 0; j < uncomp.size(); j++)
        { encoding.push_back(uncomp[j]); }
        uncomp.clear();
    }

    while(encoding.size() & 3) { encoding.push_back(0); }

    return encoding;
}

std::vector<char> LZEncode(std::vector<LZRecord> records)
{
    std::vector<char> encoding = std::vector<char>();
    std::vector<char> block_bytes = std::vector<char>();
    int shift = 7;
    char flag_byte = 0;

    int32_t min_disp = 1;
    int32_t min_window = 3;

    for(int32_t i = records.size()-1; i >= 0; i--)
    {
        int32_t flag = records[i].flag.negative;
        
        if(flag < 0)
        {
            char byte = records[i].data.byte;
            block_bytes.push_back(byte);           
        }
        else
        {
            int32_t disp = records[i].data.disp;
            int32_t len = records[i].flag.len;
            flag_byte |= 1 << shift;
            block_bytes.push_back(disp - min_disp >> 8 & 0xF 
                                  | len - min_window << 4 & 0xF0);
            block_bytes.push_back(disp - min_disp & 0xFF);
        }

        shift--;
        if(shift < 0) 
        { 
            shift += 8;
            encoding.push_back(flag_byte);
            for(int32_t j = 0; j < block_bytes.size(); j++)
            { encoding.push_back(block_bytes[j]); }
            block_bytes.clear();
        }

        records.pop_back();
    }

    if(shift != 7)
    {
        encoding.push_back(flag_byte);
        for(int32_t j = 0; j < block_bytes.size(); j++)
        { encoding.push_back(block_bytes[j]); }
        block_bytes.clear();
    }

    while(encoding.size() & 3) { encoding.push_back(0); }

    return encoding;
}

struct HuffData 
{ 
    union { uint8_t ofs; uint8_t byte; } data[2]; 
    uint8_t is_byte[2];
};

struct HuffPair
{ 
    HuffmanRecord *left, *right; 
    uint8_t parent, index; 
};

std::vector<char> HuffmanEncode(HuffmanRecord *root,
                                std::vector<uint32_t> bitstream)
{
    std::vector<char> encoding = std::vector<char>();

    HuffData curr_data;
    HuffPair curr_pair;

    std::vector<HuffPair> pair_queue = std::vector<HuffPair>();
    uint32_t head = 0;
    std::vector<HuffData> data = std::vector<HuffData>(); 

    curr_pair.left = root->left;
    curr_pair.right = root->right;
    curr_pair.parent = 0;
    curr_pair.index = 1;
    pair_queue.push_back(curr_pair);
    data.push_back(curr_data);
    

    while(head < pair_queue.size())
    {
        HuffmanRecord *left = pair_queue[head].left,
                      *right = pair_queue[head].right;

        uint8_t parent = pair_queue[head].parent;
        uint8_t index = pair_queue[head].index;
        data[parent].data[index].ofs = head - parent;

        if(left->left) 
        {
            curr_data.is_byte[0] = 0;
            curr_pair.left = left->left;
            curr_pair.right = left->right;
            curr_pair.parent = head + 1;
            curr_pair.index = 0;
            pair_queue.push_back(curr_pair);
        }
        else
        {
            curr_data.is_byte[0] = 1;
            curr_data.data[0].byte = left->byte;
        }

        if(right->left)
        {
            curr_data.is_byte[1] = 0;
            curr_pair.left = right->left;
            curr_pair.right = right->right;
            curr_pair.parent = head + 1;
            curr_pair.index = 1;
            pair_queue.push_back(curr_pair);
        }
        else
        {
            curr_data.is_byte[1] = 1;
            curr_data.data[1].byte = right->byte;
        }

        data.push_back(curr_data);
        head++;
    }

    pair_queue.clear();

    char node0,
         node1 = 0;
    encoding.push_back(data.size());
    if(data[0].is_byte[0]) { node1 |= 1 << 7; }
    if(data[0].is_byte[1]) { node1 |= 1 << 6; }
    encoding.push_back(node1);

    for(int32_t i = 1; i < data.size(); i++)
    {
        if(data[i].is_byte[0])
        {
            node0 = data[i].data[0].byte;
        }
        else
        {
            uint8_t ofs = data[i].data[0].ofs;
            if(ofs > 64) { return std::vector<char>(); }
            node0 = ofs;
            if(data[ofs].is_byte[0]) { node0 |= 1 << 7; }
            if(data[ofs].is_byte[1]) { node0 |= 1 << 6; }
        }

        if(data[i].is_byte[1])
        {
            node1 = data[i].data[1].byte;
        }
        else
        {
            uint8_t ofs = data[i].data[1].ofs;
            if(ofs > 64) { return std::vector<char>(); }
            node1 = ofs;
            if(data[ofs].is_byte[0]) { node1 |= 1 << 7; }
            if(data[ofs].is_byte[1]) { node1 |= 1 << 6; }
        }
        
        encoding.push_back(node0);
        encoding.push_back(node1);
    }

    while(encoding.size() & 3) { encoding.push_back(0); }

    for(int32_t i = 0; i < bitstream.size(); i++)
    {
        uint32_t currint = bitstream[i];
        encoding.push_back(((const char *)&currint)[0]);
        encoding.push_back(((const char *)&currint)[1]);
        encoding.push_back(((const char *)&currint)[2]);
        encoding.push_back(((const char *)&currint)[3]);
    }

    return encoding;
}

void DebugTree(HuffmanRecord *node, int offset)
{
    char *spaces = new char[offset + 1];
    int32_t i;

    for(i = 0; i < offset; i++) { spaces[i] = ' '; }
    spaces[i] = 0;

    if(node->left)
    {
        printf("%s(NON-DATA freq='%d') {\n", spaces, node->freq);
        DebugTree(node->left, offset + 4);
        DebugTree(node->right, offset + 4);
        printf("%s}\n", spaces);
    }
    else
    {
        printf("%s(DATA freq='%d' data='%d')\n", spaces, node->freq, node->byte);
    }

    delete[] spaces;
}

std::string str = "AAAA     B CCC DEED ASD";

int main()
{
    std::vector<char> bytes = std::vector<char>();
    std::vector<RLRecord> rlrecords;
    std::vector<LZRecord> lzrecords;
    LZInfo default_info;
    HuffmanRecord *root;

    default_info.min_window = 3;
    default_info.max_window = 18;
    default_info.min_disp = 1;
    default_info.max_disp = 1 << 12;

    bytes.reserve(str.length());
    for(int32_t i = 0; i < str.length(); i++)
    {
        bytes.push_back(str[i]);
    }

    rlrecords = RLCompress(bytes);
    lzrecords = LZCompress(&default_info, bytes);
    HuffmanCompress(bytes, &root);

    for(int32_t i = 0; i < rlrecords.size(); i++)
    {
        printf("DATA:          %c\n", rlrecords[i].byte);
        printf("LEN:           %d\n\n", rlrecords[i].len);   
    }

    for(int32_t i = 0; i < lzrecords.size(); i++)
    {
        if(lzrecords[i].flag.negative < 0)
        {
            printf("LZDATA:             %c\n\n", lzrecords[i].data.byte);
        }   
        else
        {
            printf("LZDISP:             %d\n", lzrecords[i].data.disp);
            printf("LZLEN:              %d\n\n", lzrecords[i].flag.len);
        }
    }

    DebugTree(root, 0);

    return 0;
}
