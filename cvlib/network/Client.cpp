#include "Client.h"
#include <unistd.h>
#include <sstream>
using namespace std;
using namespace cv;

TCPStream::TCPStream(){}

void TCPStream::connect(const string servAddress, unsigned short servPort, UDPStream *vidStream)
{
    camLib library;
    while(true){
        try {
            //Initilize connection to server
            TCPSocket sock;
            printf("TCP: Attempting to connect to server...\n");
            sock.connect(servAddress, servPort);
            connected = true;
            ostringstream ss;
            while(true)
            {
                Mat src = vidStream->getLastFrame();
                Letter ltr = library.findLetter(src);
                ss.clear();
                ss.str(string());
                ss << ltr.letter
                    << "," << to_string(ltr.width)
                    << "," << to_string(ltr.height)
                    << "," << to_string(ltr.x)
                    << "," << to_string(ltr.y)
                    << "," << to_string(ltr.pos)
                    << "," << to_string(ltr.avSize);
                
                string toSend = ss.str();
                const char *buffer = toSend.c_str();
                cout << toSend << "\n";
                try{
                    std::flush(cout);
                    int status = sock.send(buffer, strlen(buffer));
                    if(status == EPIPE){ break; }
                }catch(SocketException &e){
                    cerr << e.what() << endl;
                    break;
                }
                usleep(50000);
            }
                                    
        } catch (SocketException & e) {
            cerr << e.what() << endl;
        }
        printf("TCP: Error during connection to server\n");
        usleep(1000000);
    }

    printf("TCP: ERROR: This message should never be displayed... TCP has failed\n");
}


UDPStream::UDPStream(){}

void UDPStream::connect(string servAddress, unsigned short servPort)
{
    try {
        UDPSocket sock;

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message

        clock_t last_cycle = clock();
        sock.setRecvTimeout(5);
        
        while(true){
            //Initilize connection to server
            printf("UDP: Attempting to connect to server...\n");
            for(int i=0; i<5; i++)
            {
                int ibuf[1];
                ibuf[0] = 1;
                sock.sendTo(ibuf, sizeof(int), servAddress, servPort);            
            }
            
            
            while (1) {
                        
                // Block until receive message from a client
                do {
                    recvMsgSize = sock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
                    //if(recvMsgSize == EWOULDBLOCK || recvMsgSize == EWOULDBLOCK)
                    //    break;
                } while (recvMsgSize > sizeof(int));
                int total_pack = ((int * ) buffer)[0];

                //cout << "UDP: expecting length of packs:" << total_pack << endl;
                char * longbuf = new char[PACK_SIZE * total_pack];
                for (int i = 0; i < total_pack; i++) {
                    recvMsgSize = sock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
                    if (recvMsgSize != PACK_SIZE) {
                        cerr << "UDP: Received unexpected size pack:" << recvMsgSize << endl;
                        continue;
                    }
                    memcpy( & longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
                }

                //cout << "UDP: Received packet from " << servAddress << ":" << servPort << endl;
     
                Mat rawData = Mat(1, PACK_SIZE * total_pack, CV_8UC1, longbuf);
                Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);
                if (frame.size().width == 0) {
                    cerr << "decode failure!" << endl;
                    continue;
                }
                imshow("recv", frame);
                lastFrame = frame;
                free(longbuf);

                waitKey(1);
                clock_t next_cycle = clock();
                double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
                //cout << "\tUDP: effective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

                //cout << next_cycle - last_cycle;
                last_cycle = next_cycle;
            }
        }
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
}


int main(int argc, char * argv[]) {
        if ((argc < 4) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <TCP Port> <UDP Port>\n";
        exit(1);
    }

    string servAddress = argv[1]; // First arg: server address
    unsigned short servPortUDP = Socket::resolveService(argv[3], "udp");
    unsigned short servPortTCP = Socket::resolveService(argv[2], "tcp");
    
    Mat *m = new Mat();
    UDPStream *udpStream = new UDPStream();
    TCPStream *tcpStream = new TCPStream();

    printf("MAIN: Starting TCP Thread\n");
    thread tcpThread(&TCPStream::connect,tcpStream, servAddress, servPortTCP, udpStream);

    do{
        printf("MAIN: Waiting for tcp connection\n");
        this_thread::sleep_for(chrono::seconds(2));
    }while(!tcpStream->isConnected());
    
    printf("MAIN: Starting UDP thread\n");
    thread udpThread(&UDPStream::connect,udpStream, servAddress, servPortUDP);
    
    tcpThread.join();
    udpThread.join();
    
    return 0;
}