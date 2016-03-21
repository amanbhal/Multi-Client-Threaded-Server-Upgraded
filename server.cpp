#include "server.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <thread>

using namespace std;

namespace EpochLabsTest {

    Server::Server(const std::string& listen_address, int listen_port)
        : listen_fd(-1)
    {
        std::cout << "creating server" << std::endl;

        sockaddr_in listen_sockaddr_in;
        std::memset(&listen_sockaddr_in, 0, sizeof(listen_sockaddr_in));
        listen_sockaddr_in.sin_family = AF_INET;
        inet_aton(listen_address.c_str(), &listen_sockaddr_in.sin_addr);
        listen_sockaddr_in.sin_port = htons(listen_port);

        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(listen_fd < 0) {
            throw_error("could not create socket", errno);
        }

        int t = 1;
        if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) {
            throw_error("could not set SO_REUSEADDR", errno);
        }

        if(bind(listen_fd, (struct sockaddr*) &listen_sockaddr_in, sizeof(listen_sockaddr_in))) {
            throw_error("could not bind listen socket", errno);
        }

        if(listen(listen_fd, 48)) {
            throw_error("could not listen on socket", errno);
        }

        //Server is listening successfully 
        std::cout << "listening on " << listen_address << ":" << listen_port << std::endl;
    }

    int Server::accept_new_connection() {
        sockaddr_in peer_addr;
        socklen_t peer_addr_size = sizeof(peer_addr);
        std::memset(&peer_addr, 0, peer_addr_size);
        
        //peer_fd is the file descriptor for the socket of the newly connected client
        int peer_fd = accept4(listen_fd, (struct sockaddr*) &peer_addr, &peer_addr_size, SOCK_CLOEXEC);
      
        if (peer_fd < 0) {
            throw_error("error accepting connection", errno);
        }

        std::cout << "accepted connection, peer_fd=" << peer_fd << std::endl;

        return peer_fd;
    }


bool Server::processData(string incomingData, int fd, string &incompleteLine){
    //flag to indicate that quit request is received  
    int quit = 0;

                    //cout<<"##### RECEIVED LINE ######\n"+incomingData<<endl;
                    //cout<<"incompleteLine "+incompleteLine<<endl;
    
    //append the previous incomplete data with incomingData to make it complete and process it.
    incomingData = incompleteLine+incomingData;
    incompleteLine = "";

    //process data while it contains "\n"
    while(incomingData.find("\n")!=string::npos){
        string str = incomingData.substr(0,incomingData.find_first_of("\n"));
        incomingData = incomingData.substr(incomingData.find_first_of("\n")+1);
        //process set command
        if(str.find("set")!=string::npos){
            char buf[1024];
            str = str.substr(str.find_first_of(" ")+1);
                    ////cout<<"key value pair"<<str<<endl;

            //extracting the key
            string key = str.substr(0,str.find_first_of(" "));
                    //cout<<"\nkey is "<<key<<endl;
            str = str.substr(str.find_first_of(" ")+1);
            string val = str;
                    //cout<<"\nval is "<<val<<endl;
            
            //acquiring write lock
            pthread_rwlock_wrlock(&rwlock);
            key_val[key] = val;
            //releasing the lock
            pthread_rwlock_unlock(&rwlock);
            
            //setting the response
            string rsp = key + '=' + val + '\n';
            
            //send the response for set command back to the client
            for(unsigned int i = 0; i < rsp.size(); i++)
            buf[i] = rsp[i];
                    ////cout<<"\nresponse is "<<rsp<<endl;
            send(fd, buf,rsp.size(), 0);
            incompleteLine = "";
        }
        //process get command
        else if(str.find("get")!=string::npos){
            char buf[1024];
            str = str.substr(str.find_first_of(" ")+1);
                    //cout<<"\nkey is "<<str<<endl;
            
            //acquiring read lock
            pthread_rwlock_rdlock(&rwlock);
            unordered_map<string, string>::iterator it = key_val.find(str);
            
            //setting the response
            string rsp;
            if(it == key_val.end())
            {
                rsp = str + "=null\n";
            }
            else
            {
                rsp = str + '=' + key_val[str] + '\n';
            }
            //releaseing the read lock
            pthread_rwlock_unlock(&rwlock);
            
            //send the response for get command back to client
                    //cout<<"Response from get is "+rsp<<endl;
            for(unsigned int i = 0; i < rsp.size(); i++)
            buf[i] = rsp[i];
            
                    ////cout<<"\nresponse is "<<rsp<<endl;
            
            send(fd, buf,rsp.size(), 0);

            incompleteLine = "";
        }
        //process quit command
        else if(str.find("quit")!=string::npos){
            close(fd);
            //set quit flag
            quit = 1;
        }
    }

    //if there is any incoming data left to process then that is will be processed later and it is set in incompleteLine
    if(incomingData.size()!=0){
        incompleteLine = incomingData;
    }

    //return whether the quit command was received or not
    if(quit == 1)
        return true;
    else
        return false;
}

//method to handle each client connection
    void Server::client_handle(int fd)
    {
        // variable to hold incomplete commands
        string incompleteLine = "";

        while(true)
        {
            char buf[1024];
            //to hold the data coming from socket
            string incomingData = "";
            int len = recv(fd, buf, 1024, 0); 
            bool breakFlag = false;
            for(int i = 0; i < len; i++)
            {
                incomingData = incomingData + buf[i];
            }
            //if incomingData does not have any "\n" therefore the commands are not complete therefore update incompleteLine variable
            if(incomingData.find_first_of("\n")==string::npos){
                incompleteLine += incomingData;
            }
            //if incomingData has atleast one "\n" then send data for processing
            else{
                if(processData(incomingData, fd, incompleteLine)){
                    breakFlag = true;
                }
            }
                        ////cout<<"INCOMPLETE LINE "+incompleteLine<<endl;

            //if quit command has been issued then break quit receiving more data and kill the thread
            if(breakFlag)
                break;        
        }
    }
    
    void Server::run() {
        std::cout << "running ..." << std::endl;
        //replace with your code to implement the run method
        //run() should loop forever servicing requests/connections
    	    
    	    //initializing the read-write lock
    	    pthread_rwlock_init(&rwlock, NULL);
    	    
    	    //run the server forever
    	while(true)
    	{
    	    int fd = accept_new_connection();
    	    thread t(&Server::client_handle, this, fd); //seperate thread for each connection
    	    t.detach(); //detaching the thread execution to run the thread independently
    	    
    	    
    	}
        throw_error("Server::run() is not not implemented", 0);
    }

    void Server::throw_error(const char* msg_, int errno_) {
        std::string msg = msg_ + std::string(" errno=") + std::to_string(errno_);
        throw std::runtime_error(msg);
    }

}