// Application layer protocol implementation

#include "application_layer.h"

// file_info *file_info;

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
    puts("llopen ok!\n");

    if (ll.role == LlTx)
    {
        puts("Running in tx mode\n");

        int file = open(filename, O_RDONLY);
        if (file < 0)
        {
            perror("Error: open file\n");
            return;
        }

        puts("Sending start packet");
        if (write_control_packet(file, filename, 2) == -1)
        {
            perror("error: Start control packet not sent");
            llclose(0);
            return;
        }
        puts("Start packet sent");

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

                printf("Writing %d bytes\n", bytes_read + 3);
                if (llwrite(buffer, bytes_read + 3) == -1)
                {
                    perror("Error: llwrite\n");
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
        puts("Running in rx mode\n");

        file_info file_information;

        if (receive_control_packet(&file_information, 2) == -1)
        {
            perror("Error: Start packet");
            llclose(0);
            return;
        }
        puts("Start packet received");

        int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (file < 0)
        {
            puts("Error: open file");
            llclose(0);
            return;
        }

        unsigned char buffer[MAX_PAYLOAD_SIZE * 2];
        int bytes_read = 0;
        int bytes_left = 9999;

        while (bytes_left > 0)
        {
            bytes_read = llread(buffer);
            buffer[bytes_read] = '\0';
            printf("Reading %d bytes\n", bytes_read);

            if (bytes_read < 0)
            {
                puts("Error: llread\n");
                bytes_read = 0;
            }
            else if (bytes_read > 0)
            {
                printf("\n\nbuffer[0]: %d\n\n", buffer[0]);
                if (buffer[0] == 0)
                {
                    puts("File received!");
                    break;
                }
                else if (buffer[0] == 1)
                {
                    if (write(file, buffer + 3, bytes_read - 3) < 0)
                    {
                        perror("Error: write file\n");
                        break;
                    }
                    bytes_left -= bytes_read;
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
            else if (bytes_read == 0)
            {
                puts("Didn't receive anything");
            }

            sleep(1);
        }

        if (receive_control_packet(&file_information, 3) == -1)
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
    unsigned char packet[MAX_PAYLOAD_SIZE];
    size_t packet_size = 0;
    packet[packet_size++] = type; // start packet
    packet[packet_size++] = 0;    // file size type
    packet[packet_size++] = 0;    // file size bytes length
    size_t filesize = lseek(file, 0, SEEK_END);

    while (filesize > 0)
    {
        packet[2] += 1;                   // incerement file size bytes length
        packet[packet_size++] = filesize; // add byte to packet
        filesize >>= 8;
    }

    packet[packet_size++] = 0x01;             // filename type
    packet[packet_size++] = strlen(filename); // filename length
    memcpy(packet + packet_size, filename, strlen(filename));
    packet_size += strlen(filename);
    packet[packet_size] = '\0';

    if (llwrite(packet, packet_size) == -1)
    {
        perror("Error: Start packet");
        llclose(0);
        return -1;
    }
    return 0;
}

int receive_control_packet(file_info *file_information, int type)
{
    unsigned char control_packet[MAX_PAYLOAD_SIZE];
    if (llread(control_packet) == -1)
    {
        perror("error: control packet not received");
        return -1;
    }
    printf("Control packet size: %ld\n", sizeof control_packet);

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
    if (control_packet[2] != file_information->fileSizeBytes)
    {
        perror("Error: Not file size length");
        return -1;
    }

    // if (type == 2)
    // {
    //     int filesize=0;
    //     for(size_t i= 0; i < file_info->fileSizeBytes;i++)

    //     printf("File size: %d bytes\n", file_size);
    //     unsigned char filename_size = control_packet[8];
    //     unsigned char name[filename_size];
    //     memcpy(name, control_packet + 9, filename_size + 1);
    //     printf("File name: %s\n", name);
    // }

    return 0;
}
