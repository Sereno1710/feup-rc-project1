// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort, serialPort);
    ll.nRetransmissions = nTries;
    ll.baudRate = baudRate;
    ll.role = (strcmp(role, "tx") == 0) ? LlTx : LlRx;
    ll.timeout = timeout;

    if (llopen(ll) == -1)
    {
        perror("Error: llopen\n");
        return;
    }
    // puts("llopen ok!\n");

    if (ll.role == LlTx)
    {
        // puts("Running in tx mode\n");

        int file = open(filename, O_RDONLY);
        if (file < 0)
        {
            perror("Error: open file\n");
            return;
        }

        if (write_control_packet(file, filename, 2) == -1)
        {
            perror("error: Start control packet not sent");
            llclose(0);
            return;
        }

        const int buf_size = MAX_PAYLOAD_SIZE + 3;
        unsigned char buffer[buf_size + 3];
        int bytes_read = 1;
        while (bytes_read > 0)
        {
            bytes_read = read(file, buffer + 3, buf_size);
            if (bytes_read < 0)
            {
                perror("Error: read file\n");
                break;
            }
            else if (bytes_read > 0)
            {
                buffer[0] = 1;
                buffer[1] = (bytes_read >> 8) & 0xFF;
                buffer[2] = (bytes_read & 0xFF);

                // printf("Writing %d bytes\n", bytes_read + 3);
                if (llwrite(buffer, bytes_read + 3) == -1)
                {
                    perror("Error: llwrite\n");
                    break;
                }
            }
            else if (bytes_read == 0)
            {
                // printf("File sent!\n");
                buffer[0] = 0;
                llwrite(buffer, 1);
                break;
            }
        }

        if (write_control_packet(file, filename, 3) == -1)
        {
            perror("error: End control packet not sent");
            llclose(0);
            return;
        }

        llclose(0);
        close(file);
    }
    else if (ll.role == LlRx)
    {
        // puts("Running in rx mode\n");

        if (receive_control_packet(filename, 2) == -1)
        {
            perror("Error: Start packet");
            llclose(0);
            return;
        }

        int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (file < 0)
        {
            // puts("Error: open file");
            llclose(0);
            return;
        }

        unsigned char buffer[MAX_PAYLOAD_SIZE * 2];
        int bytes_read = 0;
        int total_bytes_read = 0;

        while (bytes_read >= 0)
        {
            bytes_read = llread(buffer);
            buffer[bytes_read] = '\0';
            // printf("Reading %d bytes\n", bytes_read);

            if (bytes_read < 0)
            {
                perror("Error: llread\n");
                break;
            }
            else if (bytes_read > 0)
            {
                if (buffer[0] == 0)
                {
                    // puts("File received!");
                    break;
                }
                else if (buffer[0] == 1)
                {
                    if (write(file, buffer + 3, bytes_read - 3) < 0)
                    {
                        perror("Error: write file\n");
                        break;
                    }
                    total_bytes_read += bytes_read - 1;
                }
                else if (buffer[0] == 2)
                {
                    puts("Start packet received!");
                }
                else if (buffer[0] == 3)
                {
                    puts("End packet received!");
                }
            }
        }

        if (receive_control_packet(filename, 3) == -1)
        {
            perror("Error: Start packet");
            llclose(0);
            return;
        }

        llclose(0);
        close(file);
    }
}

int write_control_packet(int file, const char *filename, int type)
{

    size_t filesize = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);
    unsigned char start[MAX_PAYLOAD_SIZE] = {
        type,
        0,
        4,
        (filesize >> 24) & 0xFF,
        (filesize >> 16) & 0xFF,
        (filesize >> 8) & 0xFF,
        filesize & 0xFF,
        1,
        strlen(filename)};

    memcpy(start + 9, filename, strlen(filename));
    int start_size = 9 + strlen(filename);

    if (llwrite(start, start_size) == -1)
    {
        perror("Error: Start packet");
        llclose(0);
        return -1;
    }
    return 0;
}

int receive_control_packet(const char *filename, int type)
{
    unsigned char control_packet[MAX_PAYLOAD_SIZE];
    if (llread(control_packet) == -1)
    {
        perror("error: control packet not received");
        return -1;
    }
    if (control_packet[0] != type)
    {
        perror("Error: Incorrect control packet type");
        return -1;
    }
    if (control_packet[1] != 0)
    {
        perror("Error: Not file size type");
        return -1;
    }
    if (control_packet[2] != 4)
    {
        perror("Error: Not file size length");
        return -1;
    }

    if (type == 2)
    {
        int file_size = ((control_packet[3] << 24) & 0xFF000000) | ((control_packet[4] << 16) & 0xFF0000) | ((control_packet[5] << 8) & 0xFF00) | ((control_packet[6]) & 0xFF);
        printf("File size: %d bytes\n", file_size);
        unsigned char filename_size = control_packet[8];
        unsigned char name[filename_size];
        memcpy(name, control_packet + 9, filename_size + 1);
        printf("File name: %s\n", name);
    }

    return 0;
}
