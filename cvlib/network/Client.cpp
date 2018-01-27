#include "Client.h"
#include <unistd.h>
using namespace std;
using namespace cv;

TCPStream::TCPStream(){}

void TCPStream::connect(const string servAddress, unsigned short servPort)
{
    while(true){
        try {
            //Initilize connection to server
            TCPSocket sock;
            cout << "TCP: Attempting to connect to server..." << endl;
            sock.connect(servAddress, servPort);
            connected = true;
            
            while(true)
            {
                cout << "TCP: Sending letter data" << endl;
                char *buffer = (char*)"null,0,0,0,0,0,0";
                try{
                    int status = sock.send(buffer, strlen(buffer));
                    if(status == EPIPE){ break; }
                }catch(SocketException &e){
                    cerr << e.what() << endl;
                    break;
                }
                usleep(100000);
            }
                                    
        } catch (SocketException & e) {
            cerr << e.what() << endl;
        }
        cout << "TCP: Error during connection to server" << endl;
        usleep(1000000);
    }

    cout << "TCP: ERROR: This message should never be displayed... TCP has failed" << endl;

}


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
            cout << "UDP: Attempting to connect to server..." << endl;
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
                    if(recvMsgSize == EWOULDBLOCK || recvMsgSize == EWOULDBLOCK)
                        break;
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
                free(longbuf);

                waitKey(1);
                clock_t next_cycle = clock();
                double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
                cout << "\tUDP: effective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

                cout << next_cycle - last_cycle;
                last_cycle = next_cycle;
            }
        }
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

UDPStream::UDPStream(){}
int main(int argc, char * argv[]) {
        if ((argc < 4) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <TCP Port> <UDP Port>\n";
        exit(1);
    }

    string servAddress = argv[1]; // First arg: server address
    unsigned short servPortUDP = Socket::resolveService(argv[3], "udp");
    unsigned short servPortTCP = Socket::resolveService(argv[2], "tcp");
    
    TCPStream *tcpStream = new TCPStream();
    cout << "MAIN: Starting TCP Thread" << endl;
    thread tcpThread(&TCPStream::connect,tcpStream, servAddress, servPortTCP);
    
    do{
        cout << "MAIN: Waiting for tcp connection" << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }while(!tcpStream->isConnected());
    
    UDPStream *udpStream = new UDPStream();
    cout << "MAIN: Starting UDP thread" << endl;
    thread udpThread(&UDPStream::connect,udpStream, servAddress, servPortUDP);
    
    tcpThread.join();
    udpThread.join();
    
    return 0;
}