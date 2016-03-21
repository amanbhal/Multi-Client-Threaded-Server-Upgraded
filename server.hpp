#ifndef EPOCHLABS_TEST_SERVER_HPP
#define EPOCHLABS_TEST_SERVER_HPP

#include <string>
#include <pthread.h>
#include <unordered_map>


using namespace std;

namespace EpochLabsTest {

class Server {
public:
    Server(const std::string& listen_address, int listen_port);
    void run();
  
private:
    int listen_fd;
    //add your members here
    //a global map to hold key value pairs
    unordered_map<string, string> key_val;
    //read-write lock used for synchronization 
    pthread_rwlock_t rwlock;

    int accept_new_connection();
    void throw_error(const char* msg_, int errno_);
    //add your methods here
    //seperate thread for each client will run this method
    void client(int fd);
    bool processData(string incomingData, int fd, string &incompleteLine);
};

}

#endif
