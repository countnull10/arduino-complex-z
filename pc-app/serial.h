#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// Returns the serial port descriptor (fd) or -1 on error
int init_serial(const char *portname) {
    // 1. Apertura della porta
    int fd = open(portname, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("Error opening the serial port");
        return -1;
    }

    // 2. Wait 2 seconds to allow Arduino to reset after opening
    usleep(2000000);

// 3. Configure serial parameters (termios)
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error in tcgetattr");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    // Timeout di lettura
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10; // Timeout di 1 secondo

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error in tcsetattr");
        close(fd);
        return -1;
    }

// Clear the initial buffer
    tcflush(fd, TCIFLUSH);

    return fd; // Return the ready-to-use descriptor
}

// Takes the descriptor and pointers where to save the extracted coordinates as input
int get_data(int fd, int *x, int *y) {
    char buf[256];
    memset(buf, 0, sizeof(buf));

    int n = read(fd, buf, sizeof(buf) - 1);

    if (n > 0) {
        // sscanf extracts x and y from the received string
        if (sscanf(buf, "%d,%d", x, y) == 2) {
            return 1; // Success: data read and parsed correctly
        } else {
            return 0; // Data received but invalid format
        }
    }

    return -1; // No data read or timeout expired
}
