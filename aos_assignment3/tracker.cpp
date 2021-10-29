//Use this command to compile: g++ tracker.cpp -lpthread -o tracker

#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>

using namespace std;
int port=2020;

void * communication(void *connection);

int main(int argc,char const *argv[])
{
    //Creating a socket
    int server=socket(AF_INET, SOCK_STREAM , 0);
    if(server<=0)
    {
        cout<<"Socket creation Failed";
        exit(1);
    }
    else
        cout<<"Socket established\n";
    
    //Giving address and port to socket to which it has to bind to
    struct sockaddr_in socket_address,client_address;
    socket_address.sin_family=AF_INET;
    socket_address.sin_port=htons(port);
    socket_address.sin_addr.s_addr=INADDR_ANY;
    memset(&(socket_address.sin_zero),0,8);
    //Setting socket options so that if by 
    //chance if someother socket is uding that port then our socket should also able to use that port
    int option=1;
    int socket_option=setsockopt(server,SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if(!socket_option)
    {
        cout<<"Now the socket will be easily bounded\n";
    }
    else
    {
        cout<<"Not able to set socket option\n";
        exit(1);
    }
    int bind_server=bind(server,(struct sockaddr*)&socket_address, sizeof(socket_address)); //Binded socket to port and address
    if(bind_server<0)
    {
        cout<<"Binding failed\n";
        exit(1);
    }
    else
    {
        cout<<"Binding Done\n";
    }
    int server_listen=listen(server,5);
    if(server_listen<0)
    {
        cout<<"Couldn't Listen to server\n";
        exit(1);
    }
    else
        cout<<"Server has started to listen\n";
    
    memset((char*)&(client_address),'\0',sizeof(client_address));
    socklen_t client_length = sizeof(client_address);
    while(1)
    {
        int newconnect=accept(server,(struct sockaddr*) &client_address, &client_length);
        pthread_t new_thread;
        int check=pthread_create(&new_thread , NULL , communication ,(void*)&newconnect);
    }
    
    //cout<<valread;
    
}

void * communication(void *connection)
{
    int com_soc=*(int*)connection;
    while(1)
    {
        char buffer[1024]={0};
        int valread = read( com_soc , buffer, 1024);
        cout<<buffer;
        if(strcmp(buffer,"logout")==0)
            return 0;
        cout<<"\nMessage sent\n";
        
        send(com_soc , buffer , strlen(buffer) , 0 );
    }
}
