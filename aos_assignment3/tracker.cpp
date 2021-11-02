#include<bits/stdc++.h>
#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>

using namespace std;
///////////////////////////////// User Defined Classes ///////////////////////////////////////////////////
class group
{
    public:
    string grpid;
    string owner;
    unordered_map<string,string> users;
    unordered_map<string,vector< set<sockaddr_in> >> files;
    unordered_map<string,string> pending_list; 

    group(string usr,string group_id,string owner_det)
    {
        owner=usr;
        grpid=group_id;
        users[owner]=owner_det;
    }
};

class peers
{
    public:
    int peer_fd;
    string peer_ip;
    string peer_port;
    string username;
    string password;
    unordered_map<string,string> sharable_files; //The file name will be mapped to binary string which tell which peices are there
    unordered_map<string,string> file_group; //This helps to map file name to grpid
    peers(int soc,string ip,string port,string usr, string pass)
    {
        peer_fd=soc;
        peer_ip=ip;
        peer_port=port;
        username=usr;
        password=pass;
    }
};

struct arguments
{
    int fd;
    sockaddr_in client_address;   
};
///////////////////////////////////// Global variables ////////////////////////////////////////////////////////

int port=2020;
pthread_t exit_thread;
unordered_map<string,group *> groups;
unordered_map<string,peers *> clients;

/////////////////////////////////// Function Declaration //////////////////////////////////////////////////////
void * communication(void *connection);
void * exiting(void *s);
char * create_account(int fd,string ip,string port,string username,string password);
bool authenticate(string username,string password);
void clearing();
vector<string> tokenizer(string command);
char * create_group(string username,string group_id);
char * join_group(string username,string grpid);

///////////////////////////////////// Main Function ///////////////////////////////////////////////////////////
int main(int argc,char const *argv[])
{
    //Creating a socket
    atexit(clearing);
    int server=socket(AF_INET, SOCK_STREAM , 0);
    if(server<=0)
    {
        cout<<"Socket creation Failed";
        exit(1);
    }
    else
        cout<<"Socket established\n";
    
    //port=atoi(argv[1]);
    
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
    
    pthread_create(&exit_thread , NULL , exiting ,NULL);
    
    while(1)
    {
        int newconnect=accept(server,(struct sockaddr*) &client_address, &client_length);
        pthread_t new_thread;
        arguments args;
        args.fd=newconnect;
        args.client_address=client_address;
        int check=pthread_create(&new_thread , NULL , communication ,(void*)&args);
        
    }
    
    //cout<<valread;
    
}
//////////////////////////////////// Thread to handle the client //////////////////////////////////////////////
void * communication(void *connection)
{
    arguments args=*(arguments*)connection;
    int com_soc=args.fd;
    string username;
    bool ans;
    char *reply;
    while(1)
    {
        //char *reply;
        ans=false;
        cout<<"In checking phase\n";
        char buffer[1024]={0};
        int valread = read( com_soc , buffer, 1024);
        string data(buffer);
        vector<string> tokens=tokenizer(data);
        if(tokens[0]=="create_user")
        {
            username=tokens[1];
            string password=tokens[2];
            reply=create_account(com_soc,tokens[3],tokens[4],username,password);
        }
        else if(tokens[0]=="login")
        {
            username=tokens[1];
            string password=tokens[2];
            ans=authenticate(username,password);
            if(ans)
                reply="Log in sucessful";
            else
                reply="Login Failed .... try again";
        }
        else
            reply="First create / log in first";
        send(com_soc , reply , strlen(reply) , 0 );

        if(ans)
            break;
        
    }
    while(1)
    {
        cout<<"In command phase\n";
        //This is for checking if the user is login or not?
        while(!ans)
        {
            char buff[1024]={0};
            int valread = read( com_soc , buff, 1024);
            string data(buff);
            vector<string> tokens=tokenizer(data);
            username=tokens[1];
            string password=tokens[2];
            //char *reply;
            ans=authenticate(username,password);
            if(ans)
                reply="Log in sucessful";
            else
                reply="Login Failed .... try again";
            send(com_soc , reply , strlen(reply) , 0 );
            memset(buff,'\0',1024);
        }

        //From this point onwards we will recieve commands
        
        char buffer[1024]={0};
        int valread = recv( com_soc , buffer, 1024, 0); ///Replace in place of read
        if(strcmp(buffer,"logout")==0)
        {
            ans=false;
            continue;
        }
        //cout<<buffer;
        string data(buffer);
        vector<string> tokens=tokenizer(data);
        if(tokens[0]=="create_group")
        {
            //code here
            reply=create_group(username,tokens[1]);
        }
        else if(tokens[0]=="join_group")
        {
            //code here
            reply=join_group(username,tokens[1]);
        }
        else if(tokens[0]=="leave_group")
        {
            //code here

        }
        else if(tokens[0]=="list_groups")
        {
            //code here
            char temp[20];
            for(auto i=groups.begin();i!=groups.end() ; i++)
            {
                   
                memset(temp,'\0',sizeof(temp));
                strcpy(temp,i->first.c_str());
                temp[i->first.length()]='\0';
                send(com_soc , temp , strlen(temp) , 0 );
                usleep(1);
            }
            reply="stop\0";
        }
        else if(tokens[0]=="list_files")
        {
            //code here
        }
        else if(tokens[0]=="upload_file")
        {
            //code here
        }
        else if(tokens[0]=="download_file")
        {
            //code here
        }
        else
        {
            reply=buffer;
        }
        cout<<"\nMessage sent\n";
        send(com_soc , reply , strlen(reply) , 0 );
    }
}
/////////////////////////////////////////////// Creating a user id /////////////////////////////////

char * create_account(int fd, string ip, string port,string username,string password)
{
    if(clients.find(username)!=clients.end())
    {   
        char *a="Username already exist";
            
        return a ;
    }
    char *a="New user created";
    peers *peer= new peers(fd,ip,port,username,password);
    clients[username]=peer;
    return a;
}

/////////////////////////////////////// Authenticate User //////////////////////////////////////////////////

bool authenticate(string username,string password)
{
    if(clients.find(username)==clients.end())
        return false;
    if( clients[username]->password== password)
        return true;
    return false;
}

////////////////////////////////////////////  A functions to tokenize command /////////////////////////////////
vector<string> tokenizer(string command)
{
    stringstream s(command);
    string word;
    vector<string> ans;
    while(s >> word)
    {
        ans.push_back(word);
    }
    // for(string w:ans)
    //     cout<<w<<" ";
    cout<<endl;
    return ans;
}

//////////////////////////////////////////////// Create Group ///////////////////////////////////////////////////////////////////

char * create_group(string username,string group_id)
{
    if(groups.find(group_id)!=groups.end())
        return "This group already exist";
    string usr_det=clients[username]->peer_ip+" "+clients[username]->peer_port;
    group *new_group= new group(username,group_id,usr_det);
    groups[group_id]=new_group;
    return "Group Created";
}

/////////////////////////////////////////////// Join a Group ////////////////////////////////////////////////////////////////////

char * join_group(string username,string group_id)
{
    if(groups.find(group_id)==groups.end())
        return "No such group exist";
    string usr_det=clients[username]->peer_ip+" "+clients[username]->peer_port;
    groups[group_id]->pending_list[username]=usr_det;
    return "Join Request sent"; 
}

/////////////////////////////////////////////// Thread handler to stop the server /////////////////////////////

void * exiting(void *s)
{
    //pthread_join( exit_thread , NULL);
    while(1)
    {
        string s;
        cin>>s;
        if(s=="quit")
            exit(0);
    }
}
////////////////////////////////////// Exiting Code ///////////////////////////////////////////////////////
void clearing()
{
    for(auto i=clients.begin(); i!=clients.end();i++)
    {
        delete i->second;
    }

    for(auto i=groups.begin() ; i!=groups.end() ; i++)
        delete i->second;

}