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

        int bytes_read = 1;
        while (bytes_read > 0)
        {
            bytes_read = read(file, buffer, buf_size);
            if (bytes_read < 0)
            {
                perror("Error: read file\n");
                exit(1);
            }
            else if (bytes_read > 0)
            {
                buffer[0] = 1;
                printf("Writing %d bytes\n", bytes_read + 1);
                if (llwrite(buffer, bytes_read + 1) == -1)
                {
                    perror("Error: llwrite\n");
                    exit(1);
                }
            }
            else if (bytes_read == 0)
            {
                printf("File sent!\n");
                buffer[0] = 0;
                llwrite(buffer, 1);
                break;
            }
        }
        llclose(0);
        close(file);
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

        while (bytes_read >= 0)
        {
            bytes_read = llread(buffer);
            buffer[bytes_read] = '\0';
            printf("Reading %d bytes\n", bytes_read);

            if (bytes_read <= 0)
            {
                perror("Error: llread\n");
                exit(1);
            }
            else if (bytes_read > 0)
            {
                if (buffer[0] == 0)
                {
                    puts("File received!");
                    break;
                }
                else if (buffer[0] == 1)
                {
                    if (write(file, buffer + 1, bytes_read - 1) < 0)
                    {
                        perror("Error: write file\n");
                        exit(1);
                    }
                    total_bytes_read += bytes_read - 1;
                }
            }
        }
        llclose(0);
        close(file);
    }
}
