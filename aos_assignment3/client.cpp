#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include <arpa/inet.h>
#include<netdb.h>

using namespace std;
int port=2020;


int main(int argc,char const *argv[])
{
    int client=socket(AF_INET, SOCK_STREAM , 0);
    if(client<=0)
    {
        cout<<"Socket creation Failed";
        exit(1);
    }
    char* address;
 
    struct hostent *server = gethostbyname(argv[1]);
    struct sockaddr_in socket_address;
    socket_address.sin_family=AF_INET;
    socket_address.sin_port=htons(port);
    //socket_address.sin_addr.s_addr=server->h_addr;
    bcopy((char *)server->h_addr,(char *)&socket_address.sin_addr.s_addr,server->h_length);
    //memset(&(socket_address.sin_zero),0,8);
    char hello[1024]="Hello Server";
    cout<<"";
    int newconnect=connect(client, (struct sockaddr*)&socket_address,sizeof(socket_address));
    if(newconnect<0)
    {
        cout<<"Error in making connection";
        exit(1);
    }
    
    //cout<<hello;
    send(client , hello , strlen(hello) , 0 );
}