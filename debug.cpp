
std::vector<char> DecompLZEncoding(std::vector<char> encoding, uint32_t decompsz)
{
    std::vector<char> message = std::vector<char>();
    int flag_shift = 7;
    uint8_t flags = encoding[0];
    uint8_t encodptr = 1;

    while(message.size() < decompsz)
    {
        int curflag = (flags >> flag_shift) & 1;
        if(curflag)
        {
            uint8_t mixed = encoding[encodptr++];
            uint8_t displo = encoding[encodptr++];
            uint16_t disp = (displo | mixed << 8 & 0xF00) + 1;
            uint8_t window = (mixed >> 4 & 0xF) + 3;
         
            for(int i = 0; i < window; i++)
            {
                message.push_back(message[message.size()-disp]);
            }   
        }
        else
        { message.push_back(encoding[encodptr++]); }

        flag_shift--;
        if(flag_shift < 0) { flag_shift += 8; flags = encoding[encodptr++]; }  
    }   

    return message;
}

std::vector<char> DecompHuffEncoding(std::vector<char> encoding, uint32_t decompsz)
{
    int32_t stream_index = (encoding[0] * 2 + 2) / 4;
    int32_t table_index = 1;
    std::vector<char> message = std::vector<char>();
    uint32_t *bitstream = (uint32_t *)encoding.data();
    uint32_t currint = bitstream[stream_index++];
    int flag_shift = 31;

    while(message.size() < decompsz)
    {
        int flag = (currint >> flag_shift) & 1;
        int32_t left_index = (table_index & ~1) + (encoding[table_index] & 0x3F) * 2 + 2; 
        int32_t right_index = (table_index & ~1) + (encoding[table_index] & 0x3F) * 2 + 3; 

        if(flag)
        {
            if(encoding[table_index] & 0x40) 
            { message.push_back(encoding[right_index]); table_index = 1;}
            else 
            { table_index = right_index; }
        }
        else
        {
            if(encoding[table_index] & 0x80) 
            { message.push_back(encoding[left_index]); table_index = 1;}
            else 
            { table_index = left_index; }
        }         
        
        flag_shift--;
        if(flag_shift < 0) { flag_shift += 32; currint = bitstream[stream_index++]; }
    }

    return message;
}

HuffmanRecord *Convert(std::vector<char> encoding, int32_t index)
{
    HuffmanRecord *converted = new HuffmanRecord(),
                  *left, *right;

    int32_t left_index = (index & ~1) + 2 * (encoding[index] & 0x3F) + 2,
            right_index = (index & ~1) + 2 * (encoding[index] & 0x3F) + 3;

    if(encoding[index] & 0x80) 
    {
        left = new HuffmanRecord();
        left->left = 0;
        left->right = 0;
        left->byte = encoding[left_index];
    }
    else
    {
        left = Convert(encoding, left_index);
    }

    if(encoding[index] & 0x40)
    {
        right = new HuffmanRecord();
        right->left = 0;
        right->right = 0;
        right->byte = encoding[right_index];
    }
    else
    {
        right = Convert(encoding, right_index);
    }

    converted->left = left;
    converted->right = right;

    return converted;
}

int HuffEquals(HuffmanRecord *t1, HuffmanRecord *t2)
{
    if(t1->left)
    { return t2->left && HuffEquals(t1->left, t2->left) && HuffEquals(t1->right, t2->right); }
    else
    { return !t2->left && t2->byte == t1->byte; }
}