#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#define ERR -1
#define SEED 37
#define BIT_ERROR_RATE  0.01

#define InputFileName "Lena512.bmp"
#define OutputFileName "Output.bmp"
#define debug 0
#define errorCorrecting 1

typedef struct Fifo {
    unsigned char symbol;
    struct Fifo *next;
} FifoStruct;


typedef struct Bsc {
    unsigned long currentState;
    unsigned long badBitCount;
    unsigned long goodBitCount;
    FifoStruct *bitStream;
} BinaryChannelStruct;

BinaryChannelStruct Channel;


typedef struct Buffer {
    char buffer;
    char position;
} BufferStruct;

BufferStruct SrcBuffer, DstBuffer;

int WhiteNoise(char *word, double p) {
    double noise = (double) rand() / ((double) RAND_MAX + 1.0);

    if (noise < p) {
        if (*word == 1) { *word = 0; } else { *word = 1; }
        return ERR;
    }
    return 0;
}

void OpenChannel(BinaryChannelStruct *bc) {
    srand(SEED);
    bc->currentState = 0L;
    bc->badBitCount = 0L;
    bc->goodBitCount = 0L;
    bc->bitStream = NULL;
}

void CloseChannel(BinaryChannelStruct *bc, double *ber) {
    FifoStruct *bit;

    while (bc->bitStream != NULL) {
        bit = bc->bitStream;
        bc->bitStream = bc->bitStream->next;
        free(bit);
    }

    if ((bc->badBitCount + bc->goodBitCount) == 0) { *ber = 0.0; }
    else { *ber = (double) bc->badBitCount / (double) (bc->badBitCount + bc->goodBitCount); }
}

int InChannel(BinaryChannelStruct *bc, const unsigned char *word, unsigned int size, double ber) {
    FifoStruct *bit, *ptr;
    char symbol;

    for (int i = 0; i < size; i++) {
        if ((word[i] != 1) && (word[i] != 0)) return ERR;
        if ((bit = (FifoStruct *) malloc(sizeof(FifoStruct))) == NULL) return ERR;
        bit->next = NULL;
        if (bc->bitStream == NULL) { bc->bitStream = bit; }
        else {
            ptr = bc->bitStream;
            while (ptr->next != NULL) ptr = ptr->next;
            ptr->next = bit;
        }

        bc->currentState++;
        symbol = (char) word[i];
        if (WhiteNoise(&symbol, ber) == ERR) { bc->badBitCount++; } else { bc->goodBitCount++; }
        bit->symbol = symbol;
    }
    return 0;
}

int OutChannel(BinaryChannelStruct *bc, unsigned char *word, unsigned int size) {
    FifoStruct *bit;

    for (int i = 0; i < size; i++) {
        if (bc->currentState == 0) return ERR;
        bit = bc->bitStream;
        word[i] = bit->symbol;
        bc->bitStream = bc->bitStream->next;
        free(bit);
        bc->currentState--;
    }
    return 0;
}

int getFromSourceCoder(FILE *f, unsigned char *m, int size, int *len) {
    unsigned int cr;
    int count = 0;
    while (count != size) {
        if (SrcBuffer.position == 0) {
            cr = fread(&SrcBuffer.buffer, 1, 1, f);
            if (cr == 0) return ERR;    // Fin de fichier
            SrcBuffer.position = 8;
        } else {
            m[count] = ((unsigned) SrcBuffer.buffer >> ((unsigned) SrcBuffer.position - 1u)) & 0x01u;
            SrcBuffer.position--;
            count++;
        }
    }

    *len = count;
    return 0;
}


int putToSourceDecoder(FILE *f, const unsigned char *m, int size) {
    int len = 0;

    while (len != size) {
        if (DstBuffer.position == 0) {
            fwrite(&DstBuffer.buffer, 1, 1, f);
            DstBuffer.position = 8;
            DstBuffer.buffer = 0;
        } else {
            DstBuffer.buffer = (char) ((unsigned) DstBuffer.buffer | (unsigned) (m[len] << (DstBuffer.position - 1u)));

            DstBuffer.position--;
            len++;
        }
    }

    if (DstBuffer.position == 0) {
        fwrite(&DstBuffer.buffer, 1, 1, f);
        DstBuffer.position = 8;
        DstBuffer.buffer = 0;
    }
    return 0;
}

void HammingEncoding(const unsigned char *m, unsigned char *c) {
    // Initial word
    c[0] = m[0];
    c[1] = m[1];
    c[2] = m[2];
    c[3] = m[3];
    // Redundancy
    c[4] = (char) ((unsigned) m[1] ^ (unsigned) m[2] ^ (unsigned) m[3]);
    c[5] = (char) ((unsigned) m[0] ^ (unsigned) m[2] ^ (unsigned) m[3]);
    c[6] = (char) ((unsigned) m[0] ^ (unsigned) m[1] ^ (unsigned) m[3]);
}

void HammingDecoding(unsigned char *c, unsigned char *m) {
    unsigned int s1, s2, s3;
    s1 = ((unsigned) c[1] ^ (unsigned) c[2] ^ (unsigned) c[3] ^ (unsigned) c[4]);
    s2 = ((unsigned) c[0] ^ (unsigned) c[2] ^ (unsigned) c[3] ^ (unsigned) c[5]);
    s3 = ((unsigned) c[0] ^ (unsigned) c[1] ^ (unsigned) c[3] ^ (unsigned) c[6]);
    int errorArr[8]={-1,6,5,0,4,1,2,3};
    int errorCode=errorArr[s1*4+s2*2+s3];
    if (debug) printf("Error Detected: %d %d %d %d\n",s1,s2,s3,errorCode);
    if (errorCode!=-1 && errorCorrecting){
        // Error
        c[errorCode]=c[errorCode]==0?1:0;
    }
    if (debug) {
        printf("Error corrected: ");
        for (int i = 0; i < 7; ++i) {
            printf("%d ", c[i]);
        }
        printf("\n");
    }
    m[0]=c[0];
    m[1]=c[1];
    m[2]=c[2];
    m[3]=c[3];
}

int main() {
    FILE *InputImageFile, *OutputImageFile;

    unsigned char m[4], r[4], c;
    int len;

    unsigned int loop = 0;
    double ber;

    if ((InputImageFile = fopen(InputFileName, "r")) == NULL) {
        printf("Erreur d'ouverture du fichier source (%s)\n", InputFileName);
        return -1;
    }
    if ((OutputImageFile = fopen(OutputFileName, "w")) == NULL) {
        printf("Erreur d'ouverture du fichier de destination (%s)\n", OutputFileName);
        return -1;
    }
    // Header
    for (int i = 0; i < 1080; i++) {
        fread(&c, 1, 1, InputImageFile);
        fwrite(&c, 1, 1, OutputImageFile);
    }
    SrcBuffer.buffer = 0;
    DstBuffer.buffer = 0;
    SrcBuffer.position = 0;
    DstBuffer.position = 8;
    OpenChannel(&Channel);

    while (getFromSourceCoder(InputImageFile, m, 4, &len) != ERR) {
        loop += 1;
        if (loop == 1000) {
            printf("!");
            fflush(stdout);
            loop = 0;
        }
        unsigned char encodedMessage[7];
        if (debug) printf("Original message: %d %d %d %d\n",m[0],m[1],m[2],m[3]);
        HammingEncoding(m,encodedMessage);
        if (debug) {
            printf("Encoded message:");
            for (int i = 0; i < 7; ++i) {
                printf("%d ", encodedMessage[i]);
            }
            printf("\n");
        }
        InChannel(&Channel, encodedMessage, 7, BIT_ERROR_RATE);
        OutChannel(&Channel, r, 7);
        unsigned char decodedMessage[4];
        HammingDecoding(r,decodedMessage);
        if (debug) printf("Decoded message: %d %d %d %d\n\n",decodedMessage[0],decodedMessage[1],decodedMessage[2],decodedMessage[3]);
        putToSourceDecoder(OutputImageFile, decodedMessage, 4);
    }

    CloseChannel(&Channel, &ber);


    printf("\n\nTaux d'erreur binaire : %1.4f\n", ber);

    fclose(InputImageFile);
    fclose(OutputImageFile);

    return 0;
}



