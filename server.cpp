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

void Server::processSet(string str, int fd){
    //cout<<"inside set"<<endl;
    char buf[1024];
    str = str.substr(str.find_first_of(" ")+1);
    ////cout<<"key value pair"<<str<<endl;
    string key = str.substr(0,str.find_first_of(" ")); //extracting the key
    //cout<<"\nkey is "<<key<<endl;
    str = str.substr(str.find_first_of(" ")+1);
    string val = str;//str.substr(0,str.find_first_of("\n"));   ///////////////
    //cout<<"\nval is "<<val<<endl;
    
    //acquiring write lock
    pthread_rwlock_wrlock(&rwlock);
    key_val[key] = val;
    pthread_rwlock_unlock(&rwlock); //releasing the lock
    
    //setting the response
    string rsp = key + '=' + val + '\n';
    
    //send the response for set command back to the client
    for(unsigned int i = 0; i < rsp.size(); i++)
    buf[i] = rsp[i];
    ////cout<<"\nresponse is "<<rsp<<endl;
    send(fd, buf,rsp.size(), 0);
}

void Server::processGet(string str, int fd){
    //cout<<"inside get"<<endl;
    char buf[1024];
    str = str.substr(str.find_first_of(" ")+1);
    //str = str.substr(0,str.find_first_of("\n"));      ///////////////////////////
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
    
    pthread_rwlock_unlock(&rwlock); //releaseing the read lock
    
    //send the response for get command back to client
    //cout<<"Response from get is "+rsp<<endl;
    for(unsigned int i = 0; i < rsp.size(); i++)
    buf[i] = rsp[i];
    
    ////cout<<"\nresponse is "<<rsp<<endl;
    
    send(fd, buf,rsp.size(), 0);
}

void Server::processQuit(int fd){
    //cout<<"inside quit"<<endl;
    close(fd);
}

bool Server::processData(string incomingData, int fd, string &incompleteLine){
    //string incompleteLine = "";
    int quit = 0; //flag to indicate that quit request is received  
    //cout<<"##### RECEIVED LINE ######\n"+incomingData<<endl;
    //cout<<"incompleteLine "+incompleteLine<<endl;
    incomingData = incompleteLine+incomingData;
    incompleteLine = "";
    while(incomingData.find("\n")!=string::npos){
        string command = incomingData.substr(0,incomingData.find_first_of("\n"));
        incomingData = incomingData.substr(incomingData.find_first_of("\n")+1);
        if(command.find("set")!=string::npos){
            //process set command
            //cout<<"calling set func 1"<<endl;
            processSet(command,fd);
            incompleteLine = "";
        }
        else if(command.find("get")!=string::npos){
            //process get command
            //cout<<"calling get func 2"<<endl;
            processGet(command,fd);
            incompleteLine = "";
        }
        else if(command.find("quit")!=string::npos){
            //process quit command
            //cout<<"calling quit func 3"<<endl;
            processQuit(fd);
            incompleteLine = "";
        }
    }
    if(incomingData.size()!=0){
        incompleteLine = incomingData;
    }
    //exit from thread
    if(quit == 1)
        return true;
    else
        return false;
}
//thread function to handle each client connection
    void Server::client_handle(int fd)
    {
        string incompleteLine = "";

        while(true)
        {
            char buf[1024];
            string incomingData = ""; //to hold the data coming from socket
            int len = recv(fd, buf, 1024, 0); 
            bool breakFlag = false;
            for(int i = 0; i < len; i++)
            {
                incomingData = incomingData + buf[i];
            }
            if(incomingData.find_first_of("\n")==string::npos){
                incompleteLine += incomingData;
            }
            else{
                if(processData(incomingData, fd, incompleteLine)){
                    breakFlag = true;
                }
            }
            ////cout<<"INCOMPLETE LINE "+incompleteLine<<endl;
            //output the data received from the socket
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