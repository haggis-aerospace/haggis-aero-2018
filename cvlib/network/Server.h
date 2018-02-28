#ifndef SERVER_H
#define SERVER_H

#include "PracticalSocket.h"      // For UDPSocket and SocketException
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()
#include <thread>

#define BUF_LEN 65540 // Larger than maximum UDP packet size

#include "opencv2/opencv.hpp"
#include "config.h"
#include <unistd.h>
#include <sys/time.h>


int main(int argc, char * argv[]);
void connection(std::string clientAddress, unsigned short clientPort, UDPSocket &sock);

#endif // SERVER_H
