#include <assert.h>
#include <string>
#include <ctype.h>
#include <time.h>

#include "compress.cpp"
#include "debug.cpp"

void OutputCFile(std::vector<char> encoding, FILE *fp);
void OutputAsmFile(std::vector<char> encoding, FILE *fp);
std::vector<char> ReadFile(FILE *fp);

LZInfo settings[10] =
{
    { 8, 18, 1, 0x40  },
    { 8, 18, 1, 0xC0  },
    { 7, 18, 1, 0x120 },
    { 6, 18, 1, 0x180 },
    { 5, 18, 1, 0x250 },
    { 5, 18, 1, 0x400 },
    { 4, 18, 1, 0x600 },
    { 4, 18, 1, 0x980 },
    { 3, 18, 1, 0xC80 },
    { 3, 18, 1, 0x1000 }
};
LZInfo lz_info = {3, 18, 1, 1 << 12};

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define OUTPUT_C   0
#define OUTPUT_ASM 1
#define OUTPUT_RAW 2

const char *output_ids[3] = { "c", "asm", "raw" };

#define COMPRESS_HUFF 0
#define COMPRESS_LZ   1
#define COMPRESS_RL   2
#define COMPRESS_DIFF 3

const char *comp_ids[4] = { "huff", "lz", "rl", "diff" };

#define OFORMAT_FLAG   1
#define COMPTYPE_FLAG  2
#define SIZE_FLAG      4
#define ONAME_FLAG     8
#define COMPLEVEL_FLAG 16
#define INPUT_FLAG     32

struct
{
    int output_mode;
    int compress_mode;
    int data_size;
    int flags;
    FILE *input_fp;
    FILE *output_fp;
}
prog_info =
{
    OUTPUT_RAW,
    COMPRESS_LZ,
    8,
    0,
    stdin,
    stdout
};

void Usage()
{
    printf("Usage: gcmp [-t [huff | diff | lz | rl]] [-s size]\r\n"
           "            [-f [raw | c | asm]] [-o file] [-0..9] input_file\r\n"
           "\r\n"
           "Take input data and output compression data for Gameboy Advance\r\n"
           "\r\n"
           "-t    : Specify type of compression or filter (Default is LZ)\r\n"
           "-s    : Specify data size in bits (Default is 8)\r\n"
           "-f    : Specify data format (Default is raw file)\r\n"
           "-o    : Specify output file path (output to stdout by default)\r\n"
           "-0..9 : LZ compression level (-0 is worst, -9 is best)\r\n"); 

    exit(1);
}

int main(int argc, char *argv[])
{
    for(int i = 1; i < argc; i++)
    {
        char *carg = argv[i];

        if(!strncmp("-t", carg, 2))
        {
            if(prog_info.flags & COMPTYPE_FLAG)
            { fprintf(stderr, "Error: Multiple compression types specified\r\n");
              exit(-1); }

            char *comp_format = carg[2] ? &carg[2] : argv[++i];
            int id = 0;

            for(id = 0; id < ARRLEN(comp_ids); id++)
            { if(!strcmp(comp_format, comp_ids[id])) { break; } }

            if(id >= ARRLEN(comp_ids)) 
            { fprintf(stderr, "Error: Unknown option '%s'\r\n", carg); 
               exit(-1); }

            prog_info.compress_mode = id;
            prog_info.flags |= COMPTYPE_FLAG;
        }
        else if(!strncmp("-s", carg, 2))
        {
            char *size = carg[2] ? &carg[2] : argv[++i];
            int sz = 0;

            if(prog_info.flags & SIZE_FLAG)
            { fprintf(stderr, "Error: Multiple sizes specified\r\n");
              exit(-1); }

            while(isdigit(*size))
            {
                if(sz >= 16) { break; }
                sz *= 10; 
                sz += (*size++ - '0'); 
            }

            if(sz != 4 && sz != 8 && sz != 16)
            { fprintf(stderr, "Error: Invalid size %d. Valid sizes are 4, 8, 16\r\n"); 
              exit(-1); }

            prog_info.data_size = sz;
            prog_info.flags |= SIZE_FLAG;
        }
        else if(!strncmp("-f", carg, 2)) 
        {
            if(prog_info.flags & OFORMAT_FLAG)
            { fprintf(stderr, "Error: Multiple output types specified\r\n");
              exit(-1); }

            char *oformat = carg[2] ? &carg[2] : argv[++i];

            int id = 0;

            for(id = 0; id < ARRLEN(output_ids); id++)
            { if(!strcmp(oformat, output_ids[id])) { break; } }

            if(id >= ARRLEN(output_ids)) 
            { fprintf(stderr, "Error: Unknown option '%s'\n", carg); 
               exit(-1); }

            prog_info.output_mode = id;
            prog_info.flags |= OFORMAT_FLAG;
        }
        else if(!strncmp("-o", carg, 2)) 
        {
            char *output_name = carg[2] ? &carg[2] : argv[++i];
            if(prog_info.flags & ONAME_FLAG)
            { fprintf(stderr, "Error: Multiple output files specified\r\n");
              exit(-1); }

            prog_info.output_fp = fopen(output_name, "wb");
            if(!prog_info.output_fp)
            { fprintf(stderr, "Error opening output file for writing: ");
              perror(""); exit(-1); }

            prog_info.flags |= ONAME_FLAG;
        }
        else if(*carg == '-' && isdigit(carg[1]))
        {
            if(prog_info.flags & COMPLEVEL_FLAG)
            { fprintf(stderr, "Error: Multiple compression levels specified\r\n");
              exit(-1); }

            if(carg[2]) 
            { fprintf(stderr, "Unknown option '%s'\r\n", carg); 
              exit(-1); }

            lz_info = settings[carg[1] - '0'];
            prog_info.flags |= COMPLEVEL_FLAG;
        }
        else if(*carg != '-') 
        {
            if(prog_info.flags & INPUT_FLAG)
            { fprintf(stderr, "Error: Multiple input files specified\r\n");
              exit(-1); }

            prog_info.input_fp = fopen(carg, "rb");
            if(!prog_info.input_fp)
            { 
                fprintf(stderr, "Error opening file '%s': ", carg);
                perror(""); exit(-1); 
            }

            prog_info.flags |= INPUT_FLAG;
        }
    }

    std::vector<char> bytes = ReadFile(prog_info.input_fp);
    std::vector<char> encoding;
    uint32_t header;

    if(bytes.size() >= 1 << 24) 
    {
        fprintf(stderr, "File size >= 16 MiB. Unable to compress\r\n");
        exit(-1);
    }
    if(!bytes.size()) 
    { fprintf(stderr, "Empty file. Nothing to compress\r\n.");
      exit(-1); }

    fprintf(stderr, "Compressing...\r\n");
    clock_t start = clock();

    switch(prog_info.compress_mode)
    {
        case COMPRESS_DIFF:
        {
            if(prog_info.data_size == 16)
            {
                std::vector<uint16_t> temp = Bytes2Shorts(bytes);
                encoding = Shorts2Bytes(Diff16Filter(temp));
            }
            else if(prog_info.data_size == 8)
            {
                encoding = Diff8Filter(bytes);
            }
            else
            {
                 fprintf(stderr, "Error: Invalid data size for diff\r\n"
                                 "       Acceptable data sizes are 8 and 16\r\n");
                 exit(-1);
            }

            header = prog_info.data_size >> 3 | 0x80 | bytes.size() << 8;
            break;
        }
        case COMPRESS_HUFF:
        {
             HuffmanRecord *root;
             std::vector<uint32_t> bitstream;

             if(prog_info.data_size == 4)
             {
                 std::vector<char> temp = std::vector<char>();
                 for(int32_t i = 0; i < bytes.size(); i++)
                 {
                     temp.push_back(bytes[i] & 0xF);
                     temp.push_back(bytes[i] >> 4 & 0xF);
                 }

                 bitstream = HuffmanCompress(temp, &root);
             }
             else if(prog_info.data_size == 8)
             {
                 bitstream = HuffmanCompress(bytes, &root);
             }
             else
             {
                 fprintf(stderr, "Error: Invalid data size for huffman\r\n"
                                 "       Acceptable data sizes are 4 and 8\r\n");
                 exit(-1);
             }

             if(!root->left)
             { 
               fprintf(stderr, "Root is data node in huffman. Can't compress.\r\n"
                               "Use RL or loop instead.\r\n"); 
               exit(-1); 
             }

             encoding = HuffmanEncode(root, bitstream);
             header = 0x20 | prog_info.data_size | bytes.size() << 8;
             break;
        }
        case COMPRESS_LZ:
        {
            std::vector<LZRecord> lzrecords = LZCompress(bytes, lz_info);
            encoding = LZEncode(lzrecords);
            header = bytes.size() << 8 | 0x10;
            
            break;
        }
        case COMPRESS_RL:
        {
            std::vector<RLRecord> rlrecords = RLCompress(bytes);
            encoding = RLEncode(rlrecords);
            header = bytes.size() << 8 | 0x30;

            break;
        }
        default:
            fprintf(stderr, "Bugged\r\n");
            exit(-1);
    }

    std::vector<uint32_t> encoding_ints = Bytes2Ints(encoding);
    encoding_ints.insert(encoding_ints.begin(), header);
    encoding = Ints2Bytes(encoding_ints);

    clock_t end = clock();

    fprintf(stderr,
            "Compression done\r\n"
            "Bytes uncompressed: %d\r\n"
            "Bytes compressed:   %d\r\n"
            "Compression rate:   %f%%\r\n"
            "Time elapsed:       %f s\r\n",
            bytes.size(), encoding.size(),
            (1.0f - (float)(encoding.size())/bytes.size()) * 100.0f,
            (float)(end - start) / CLOCKS_PER_SEC);

    switch(prog_info.output_mode)
    {
        case OUTPUT_C:
        {
            OutputCFile(encoding, prog_info.output_fp);
            break;
        }

        case OUTPUT_ASM:
        {
            OutputAsmFile(encoding, prog_info.output_fp);
            break;
        }

        case OUTPUT_RAW:
        {
            int32_t i;
            for(i = 0; i + 512 < encoding.size(); i += 512)
            {
                fwrite(encoding.data() + i, 1, 512, prog_info.output_fp);
            }
            fwrite(encoding.data() + i, 1, bytes.size() - i, prog_info.output_fp);
            break;
        }
        default:
        {
            fprintf(stderr, "Bugged...\r\n");
            exit(-1);
        }
    }

    return 0;
}

std::vector<char> ReadFile(FILE *fp)
{
    char buffer[512];
    std::vector<char> contents = std::vector<char>();
    int nbytes;   

    do
    {
        nbytes = fread(buffer, 1, 512, fp);
        for(int i = 0; i < nbytes; i++)
        { contents.push_back(buffer[i]); }
    }
    while(nbytes >= 512);    

    return contents;
}

#define BUFLEN 9192
void OutputCFile(std::vector<char> encoding, FILE *fp)
{

    char buffer[BUFLEN];
    uint32_t curint = 0;
    int32_t i = 0;
    int bufptr = 0;
    int nleft = 8;

    std::vector<uint32_t> encoding_ints = Bytes2Ints(encoding);

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

void OutputAsmFile(std::vector<char> encoding, FILE *fp)
{
    char buffer[BUFLEN];
    uint32_t curint;
    int32_t i = 0;
    int bufptr = 0;
    int nleft = 8;

    assert(!(encoding.size() & 3));

    fprintf(fp,
                 ".global bitstream\r\n"
                 ".align 4\r\n"
                 ".section .rodata\r\n"
                 "\r\n"
                 "bitstream:\r\n\r\n"
                 ".word ",
                 encoding.size());

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
