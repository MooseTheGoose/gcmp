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

std::vector<char> HuffmanCompress(std::vector<char> bytes, 
                                  HuffmanRecord **huffptr)
{
    std::vector<HuffmanRecord *> records = std::vector<HuffmanRecord *>();
    HuffmanRecord *root;
    std::vector<char> compressed = std::vector<char>();

    if(bytes.size() == 0) { *huffptr = NULL; return compressed; }

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

    *huffptr = records[0];

    return compressed;
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

    return encoding;
}

std::vector<char> HuffmanEncode(std::vector<char> compressed,
                                HuffmanRecord *root)
{
    std::vector<char> encoding = std::vector<char>();

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
