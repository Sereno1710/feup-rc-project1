// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort, serialPort);
    ll.nRetransmissions = nTries;
    ll.baudRate = baudRate;
    ll.role = strcmp(role, "tx") == 0 ? LlTx : LlRx;
    ll.timeout = timeout;

    if (llopen(ll) == -1)
    {
        perror("Error: llopen\n");
        exit(1);
    }
    printf("llopen ok!\n");

    if (ll.role == LlTx)
    {
        printf("Running in tx mode\n");

        int file = open(filename, O_RDONLY);
        if (file < 0)
        {
            perror("Error: open file\n");
            exit(1);
        }

        const int buf_size = MAX_PAYLOAD_SIZE - 1;
        unsigned char buffer[buf_size + 1];
    }
    else if (ll.role == LlRx)
    {
        printf("Running in rx mode\n");

        int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (file < 0)
        {
            llclose(0);
            perror("Error: open file\n");
            exit(1);
        }

        const int buf_size = MAX_PAYLOAD_SIZE * 2;
        unsigned char buffer[buf_size];
        int bytes_read = 0;
        int total_bytes_read = 0;
    }
}
