#include <assert.h>
#include <string>

#include "compress.cpp"
#include "debug.cpp"

void OutputCFile(std::vector<char> encoding, uint32_t header, FILE *fp);
void OutputAsmFile(std::vector<char> encoding, FILE *fp, char *fname);
std::vector<char> ReadFile(char *fname);

int main(int argc, char *argv[])
{
    HuffmanRecord *root;
    HuffmanRecord *test_encoding;
    std::vector<char> bytes = std::vector<char>();
    uint32_t *encoding_ints;

    LZInfo default_info = 
    {
        3,
        18,
        1,
        1 << 12
    };

    if(argc != 2)
    {
        fprintf(stderr, "Usage: gcmp [test]\r\n");
        exit(-1);
    }

    bytes = ReadFile(argv[1]);

    std::vector<LZRecord> lzrecords = LZCompress(bytes, default_info);
    std::vector<RLRecord> rlrecords = RLCompress(bytes);
    std::vector<uint32_t> bitstream = HuffmanCompress(bytes, &root);

    std::vector<char> encoding = HuffmanEncode(root, bitstream);
    std::vector<char> reversed = DecompHuffEncoding(encoding, bytes.size());
    fprintf(stdout, "#define DECOMP_LEN %u\r\n\r\n", bytes.size());
    OutputCFile(encoding, 0x10 | bytes.size() << 8, stdout);
    printf("%s\n", reversed.data());

    return 0;
}

std::vector<char> ReadFile(char *fname)
{
    char buffer[512];
    std::vector<char> contents = std::vector<char>();
    FILE *fp = fopen(fname, "rb");
    int nbytes;

    if(!fp) { return contents; }    

    do
    {
        nbytes = fread(buffer, 1, 512, fp);
        for(int i = 0; i < nbytes; i++)
        { contents.push_back(buffer[i]); }
    }
    while(nbytes >= 512);    

    return contents;
}

void OutputCFile(std::vector<char> encoding, uint32_t header, FILE *fp)
{
    #define BUFLEN 2048
    char buffer[BUFLEN];
    uint32_t curint = 0;
    int32_t i = 0;
    int bufptr = 0;
    int nleft = 8;

    std::vector<uint32_t> encoding_ints = Bytes2Ints(encoding);
    encoding_ints.insert(encoding_ints.begin(), header);

    fprintf(fp, "\r\n" "const unsigned int bitstream[] = {\r\n    ",
                 encoding.size());

    for(i = 0; i < encoding_ints.size()-1; i++)
    {
        curint = encoding_ints[i];

        bufptr += sprintf(buffer + bufptr, "0x%08X, ", curint);

        nleft--;
        if(!nleft) 
        { bufptr += sprintf(buffer + bufptr, "\r\n    "); nleft = 8; }

        if(bufptr > BUFLEN-512) { fprintf(fp, "%s", buffer); bufptr = 0; }
    }

    if(encoding_ints.size()) curint = encoding_ints[i];

    sprintf(buffer + bufptr, "0x%08X\r\n};\r\n", curint);
    fprintf(fp, buffer);
}

void OutputAsmFile(std::vector<char> encoding, FILE *fp, char *fname)
{
    #define BUFLEN 2048
    char buffer[BUFLEN];
    uint32_t curint;
    int32_t i = 0;
    int bufptr = 0;
    int nleft = 8;

    assert(!(encoding.size() & 3));

    fprintf(fp, "#define BITSTREAM_LEN %u\r\n" 
                 "\r\n"
                 "file \"%s\"\r\n"
                 ".global bitstream\r\n"
                 ".align 4\r\n"
                 ".section .rodata\r\n"
                 "\r\n"
                 "bitstream:\r\n\r\n"
                 ".word ",
                 encoding.size(), fname);

    for(i = 0; i < encoding.size()-4; i += 4)
    {
        curint = encoding[i] | encoding[i+1] << 8 
                 | encoding[i+2] << 16 | encoding[i+3] << 24; 

        bufptr += sprintf(buffer + bufptr, "0x%08X, ", curint);

        nleft--;
        if(!nleft) 
        { bufptr += sprintf(buffer + bufptr, "\r\n.word "); nleft = 8; }

        if(bufptr > BUFLEN-512) { fprintf(fp, "%s", buffer); bufptr = 0; }
    }

    curint = encoding[i] | encoding[i+1] << 8 
             | encoding[i+2] << 16 | encoding[i+3] << 24;

    sprintf(buffer + bufptr, "0x%08X\r\n", curint);
    fprintf(fp, buffer);
}
