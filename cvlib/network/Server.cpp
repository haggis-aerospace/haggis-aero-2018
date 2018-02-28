#include "Server.h"
#include <unistd.h>

using namespace std;
using namespace cv;


bool sendData = false;

int main(int argc, char * argv[]) {

    if (argc != 2) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    try {
        UDPSocket sock(servPort);
              
        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string clientAddress; // Address of datagram source
        unsigned short clientPort; // Port of datagram source
        
        
        thread connectionThread;
        
        while(true)
        {
            cout << "Server: Listening for connections..." << endl;
            do {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, clientAddress, clientPort);
            } while (recvMsgSize > sizeof(int));
            cout << "Server: Client connected" << endl;
            
            
            if(connectionThread.joinable())
            {
                    sendData = false;
                    connectionThread.join();
            }
                            // Retrieve message from client; used to get client address            
            connectionThread = thread(connection, clientAddress, clientPort, std::ref(sock));
            
        }
        
        // Destructor closes the socket

    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}

void connection(string clientAddress, unsigned short clientPort, UDPSocket &sock)
{
    clock_t last_cycle = clock();
    int jpegqual =  ENCODE_QUALITY; // Compression Parameter
    Mat frame, send;
    vector < uchar > encoded;
    sendData = true;
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cerr << "OpenCV Failed to open camera";
        exit(1);
    }
        
    cout << "Starting data stream" << endl;
    
    while (sendData) {
        cap >> frame;
        if(frame.size().width==0)continue;//simple integrity check; skip erroneous data...
        resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
        vector < int > compression_params;
        compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
        compression_params.push_back(jpegqual);

        imencode(".jpg", send, encoded, compression_params);

        int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

        int ibuf[1];
        ibuf[0] = total_pack;
        sock.sendTo(ibuf, sizeof(int), clientAddress, clientPort);

        for (int i = 0; i < total_pack; i++)
            sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, clientAddress, clientPort);

        
        clock_t next_cycle = clock();
        double duration = (next_cycle - last_cycle) / ((double) CLOCKS_PER_SEC/1000);
        cout << "\teffective FPS:" << (1 / ((double)duration/1000)) << " \tMbps:" << (PACK_SIZE * total_pack / (1.0/(double)FRAME_RATE) / 1024 / 1024) << endl;

        last_cycle = next_cycle;
        //double sleep = (1.0/(double)FRAME_RATE*1000000.0);
        //printf("Sleep: %.3f  Duration %.3f\n", sleep, duration*1000);
        usleep((1000000/FRAME_RATE));
    }
    
    cap.release();
    
    cout << "Connection ended" << endl;
}
