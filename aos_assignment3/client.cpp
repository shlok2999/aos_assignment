#include<bits/stdc++.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

using namespace std;

////////////////////////////////////////// Global Variables ////////////////////////////////////
int port=2020;
int client,client1;
struct sockaddr_in client_address,c_addr;
string my_address,my_port;
int file_buffer_size=512*1024;
unordered_map<string,string> files_shared;
unordered_map<string,string> files_downloaded;
unordered_map<string,char> files_status;

/////////////////////// Function Dfinitions /////////////////////////////////////////////////////////
void * clearing (void *arg);
void * communication(void *arg);
void * listener(void *arg);
void * communictaing(void *arg);
char * sending(int com_soc,string msg);
void initialize(string ip);
int connecting(string ip);
void download_file(int con);
void upload_file(int con);
vector<string> tokenizer(string command);
string getfilename(string path);
int getfilesize(string file);
string getbitmap(int n);

///////////////////////////// Main Function ///////////////////////////////////////////////////////////
int main(int argc,char const *argv[])
{
    string ip(argv[1]);
    initialize(ip);

    //Connecting with a tracker
    string server_ip(argv[2]);
    int newconnect=connecting(server_ip);
    // pthread_t quitting_thread;
    // int check=pthread_create(&quitting_thread , NULL , clearing ,NULL);
    // if(check< 0)
    // {
    //     cout<<"Error in creating thread\n";
    //     exit(0);
    // }
    pthread_t new_thread;
    int check=pthread_create(&new_thread , NULL , communication ,(void*)&newconnect);
    if(check< 0)
    {
        cout<<"Error in creating thread\n";
        exit(0);
    }
    pthread_t listener_thread;
    check=pthread_create(&listener_thread , NULL , listener ,(void*)&client);
    if(check < 0)
    {
        cout<<"Error in creating thread\n";
        exit(0);
    }

    // pthread_join( quitting_thread , NULL);
    // communication(newconnect);
    // //cout<<hello;
    // cout<<"Enter 1 for upload 0 for download:";
    // int op;
    // cin>>op;
    // //cout<<"hello";
    // if(op)
    // {
    //     cout<<op;
    //     socklen_t cl=sizeof(c_addr);
    //     int nc=accept(client,(sockaddr*)&c_addr,&cl);
    //     cout<<nc<<endl;
    //     upload_file(nc);
    // }
    // else
    // {
    //     download_file(newconnect);
    // }
    // cout<<"Do you want to communicate with server(1/0)?:";
    // int n;
    // cin>>n;
    // while(n)
    // {
    //     char buffer[1024]={0};
    //     cin>>buffer;
    //     send(newconnect,buffer , strlen(buffer) , 0);
    //     char buff[1024];
    //     int rd=recv(newconnect,buff ,sizeof(buff),0);
    //     cout<<buff;
    //     memset(buff,'\0',1024);
    // }
    while(1);
    close(newconnect);
    close(client);
    close(client1);
}


///////////////////////////// Function to handle chat with tracker ///////////////////////////////////////////
void * communication(void * arg)
{
    // char hello[1024]="Hello Server";
    // 
    int client=*(int*)arg;
    while(1)
    {
        char buffer[1024]={0};
        string s;
        //char msg[256];
        getline(cin >> ws,s);
        //cin >> s;
        // Checking if user wants to logout
        if(s=="logout")
        {
            sending(client,s);
            continue;
        }
        if(s=="quit")
        {
            sending(client,s);
            return NULL;
        }
        if(s=="")
            continue;
        vector<string> tokens=tokenizer(s);
        int tsize=tokens.size();
        // if user wants to create user id
        if(tokens[0]=="create_user" && tsize==3)
        {
            s=s+ " " + my_address+ " " + my_port;

            sending(client,s);

            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="login" && tsize==3)
        {
            sending(client,s);
        
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="join_group" && tsize==2)
        {
            sending(client,s);
        
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="leave_group" && tsize==2)
        {
            sending(client,s);
        
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="upload_file" && tsize==3)
        {
            //This code is not complete
            //Need to calculate number of fragment and send
            //Need to make this modular
            int n=getfilesize(tokens[1]);
            if(n==-1)
                continue;
            string bitmap=getbitmap(n);
            string file_name=getfilename(tokens[1]);
            files_shared[file_name]=tokens[1];
            s=tokens[0]+" "+file_name+" "+tokens[2]+" "+to_string(n)+" "+bitmap;
            sending(client,s);
            // strcpy(msg,s.c_str());
            // msg[s.length()]='\0';
            // send(client , msg , strlen(msg) , 0 );

            int recieve=read(client,buffer,sizeof(buffer));
            // //cout<<recieve;
             cout<<buffer<<endl;
        }
        else if(tokens[0]=="download_file" && tsize==4)
        {
            files_downloaded[tokens[2]]=tokens[3]+'/'+tokens[2];
            files_status[tokens[2]]='C';
            sending(client,s);
            vector<string> list_of_peers;

            memset(buffer,'\0',sizeof(buffer));
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<buffer<<endl;
            if(strcmp(buffer,"Accepted")==0)
            {
                //cout<<buffer<<endl;
                char users[1000];
                while(1)
                {
                    memset(users,'\0',sizeof(users));
                    int size=recv(client ,users , sizeof(users),0);
                    if(strcmp(users,"stop")==0)
                        break;
                    string p(users);
                    list_of_peers.push_back(p);
                    cout<<p<<endl;
                
                }
            }
            else
                cout<<buffer<<endl;


        }
        else if(tokens[0]=="list_groups")
        {
            sending(client,s);
            vector<string> log;
            char groups[125];
            while(1)
            {
                memset(groups,'\0',sizeof(groups));
                int size=recv(client ,groups , sizeof(groups),0);
                if(strcmp(groups,"stop")==0)
                    break;
                if(size==0)
                    continue;
                cout<<groups<<endl;
                
            }
        }
        else if(tokens[0]=="create_group" && tsize==2)
        {
            sending(client,s);
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tsize==3 && tokens[0]=="requests" && tokens[1]=="list_requests")
        {
            sending(client,s);
            memset(buffer,'\0',sizeof(buffer));
            int recieve=read(client,buffer,sizeof(buffer));
            if(strcmp(buffer,"Accepted")==0)
            {
                //cout<<buffer<<endl;
                char users[125];
                while(1)
                {
                    memset(users,'\0',sizeof(users));
                    int size=recv(client ,users , sizeof(users),0);
                    if(strcmp(users,"stop")==0)
                        break;
                    cout<<users<<endl;
                
                }
            }
            else
                cout<<buffer<<endl;
        }
        else if(tokens[0]=="list_files" && tsize==2)
        {
            sending(client,s);
            memset(buffer,'\0',sizeof(buffer));
            int recieve=read(client,buffer,sizeof(buffer));
            if(strcmp(buffer,"Accepted")==0)
            {
                //cout<<buffer<<endl;
                char users[125];
                while(1)
                {
                    memset(users,'\0',sizeof(users));
                    int size=recv(client ,users , sizeof(users),0);
                    if(strcmp(users,"stop")==0)
                        break;
                    cout<<users<<endl;
                
                }
            }
            else
                cout<<buffer<<endl;   
        }
        else if(tokens[0]=="accept_request" && tsize==3)
        {
            sending(client,s);
            memset(buffer,'\0',sizeof(buffer));
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else
        {
            cout<<"Wrong command\n";
            continue;
        }
    }
}

/////////////////////////////////////////////// Sending a msg function //////////////////////////////////////////////////////
char * sending(int com_soc,string s)
{
    char msg[256]={0};
    strcpy(msg,s.c_str());
    msg[s.length()]='\0';
    send(com_soc , msg , strlen(msg) , 0 );
    return msg;
}


////////////////////////////////////////////////Initializing server socket///////////////////////////////////////////////////

void initialize(string ip)
{
    client=socket(AF_INET, SOCK_STREAM , 0);
    if((client1=socket(AF_INET, SOCK_STREAM , 0))<0)
        cout<<"Errror in soc";
    if(client<=0)
    {
        cout<<"Socket creation Failed";
        exit(1);
    }
    
    char address[30];
    int index=ip.find(':');
    my_port=ip.substr(index+1);
    int port=stoi(my_port);
    ip=ip.substr(0,index);
    my_address=ip;
    strcpy(address,ip.c_str());
    address[ip.length()]='\0';
    //cout<<address;
    //Making this client as server
    struct hostent *client_add = gethostbyname(address);
    
    client_address.sin_family=AF_INET;
    client_address.sin_port=htons(port);
    bcopy((char *)client_add->h_addr,(char *)&client_address.sin_addr.s_addr,client_add->h_length);
    int option=1;
    int socket_option=setsockopt(client,SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if(socket_option!=0)
    {
        cout<<"Not able to set socket option\n";
        exit(1);
    }
    int bind_client=bind(client,(struct sockaddr*)&client_address, sizeof(client_address)); //Binded socket to port and address
    if(bind_client<0)
    {
        cout<<"Binding failed\n";
        exit(1);
    }
    int client_listen=listen(client,5);
    if(client_listen<0)
    {
        cout<<"Couldn't Listen to server\n";
        exit(1);
    }
}


////////////////////////////////////// To connect to a peer /////////////////////////////////////////////////////
int connecting(string ip)
{
    int index=ip.find(':');
    port=stoi(ip.substr(index+1));
    ip=ip.substr(0,index);
    char address[30];
    strcpy(address,ip.c_str());
    address[ip.length()]='\0';
    //cout<<address<<endl;
    struct hostent *tracker = gethostbyname(address);
    struct sockaddr_in socket_address;
    socket_address.sin_family=AF_INET;
    socket_address.sin_port=htons(port);
    //socket_address.sin_addr.s_addr=tracker->h_addr;
    bcopy((char *)tracker->h_addr,(char *)&socket_address.sin_addr.s_addr,tracker->h_length);
    //memset(&(socket_address.sin_zero),0,8);
    
    //cout<<"";
    int newconnect=connect(client1, (struct sockaddr*)&socket_address,sizeof(socket_address));
    //cout<<errno;
    if(newconnect<0)
    {
        cout<<"Error in making connection";
        exit(1);
    }
    return client1;
}

//////////////////////////////////// Downloading file /////////////////////////////////////////////////////////////////////////

void download_file(int con)
{
    char buffer[1024]={0};
    string s;
    getline(cin >> ws,s);
    
   
    
    char msg[256];
    strcpy(msg,s.c_str());
    msg[s.length()]='\0';
    cout<<msg;
    send(con , msg , strlen(msg) , 0 );

    cout<<msg;
    int file=open(msg, O_WRONLY | O_CREAT, S_IRUSR| S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if(file < 0){
    	cout << "Could not create file" << endl;
    	exit(1);
    }
    char file_content[file_buffer_size];
    while(1)
    {
        int size=recv(con,file_content,sizeof(file_content),0);
        if(size<=0)
            break;
        write(file,file_content,size);
    }
    close(file);
}



/////////////////////////////////////// Upload File ////////////////////////////////////////////////////////

void upload_file(int con)
{
    //cout<<"In upload Section";
    char msg[FILENAME_MAX];
    int readval=recv(con , msg ,sizeof(msg), 0);
    cout<<msg<<endl;
    int file=open(msg, O_RDONLY);
    if(file < 0){
    	cout << "Invalid file" << endl;
    	exit(1);
    }
    char file_content[file_buffer_size];
    int size;
    while((size=read(file,file_content,sizeof(file_content)))>0)
    {
        send( con , file_content , size , 0);
    }
    if(size<0)
    {
        cout<<"Error in Transmitting file";
        return;
    }
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
    //cout<<endl;
    return ans;
}

///////////////////////////////////////// Function to get filename ///////////////////////////////////////////////////////
string getfilename(string path)
{
    int n=path.length();
    for(int i=n;i>=0;i--)
    {
        if(path[i]=='/')
            return path.substr(i+1);
    }
    return path;
}
//////////////////////////////////////////////// Listener function ////////////////////////////////////////////////////////
void *listener(void *arg)
{
    struct sockaddr_in client_address1;
    socklen_t client_length = sizeof(client_address);
    while(1)
    {
        int newconnect=accept(client,(struct sockaddr*) &client_address1, &client_length);
        pthread_t new_thread;
        int check=pthread_create(&new_thread , NULL , communictaing ,(void*)&newconnect);
        
    }
}

/////////////////////////////////////////////// Thread for communicating as a server //////////////////////////////////////
void * communictaing(void *arg)
{
    int client_talking=*(int*)arg;
    char buffer[1024]={0};
    while(1)
    {
        int recieve=read(client_talking,buffer,sizeof(buffer));
            //cout<<recieve;
        if(strcmp(buffer,"logout")==0)
            return NULL;
        cout<<buffer<<endl;
        cout<<"Message sent";
        send(client_talking , buffer , strlen(buffer) , 0);
        memset(buffer,'\0',sizeof(buffer));
    }
}

//////////////////////////////////////////////// Calculating file size ////////////////////////////////////

int getfilesize(string path)
{
    char file[FILENAME_MAX]={0};
    strcpy(file,path.c_str());
    file[path.length()]='\0';
    struct stat st;
    if(stat(file,&st)==-1)
        return -1;
    int num_of_chunks=st.st_size/file_buffer_size;
    if(st.st_size%file_buffer_size)
        num_of_chunks++;
    return num_of_chunks;

}

//////////////////////////////////////////////// Function to get bitmap //////////////////////////////////

string getbitmap(int n)
{
    string s="";
    for(int i=0;i<n;i++)
        s=s+'1';
    return s;
}
////////////////////////////////////////////// Exiting Thread ////////////////////////////////////////////////////////////
// void * exiting(void *s)
// {
//     //pthread_join( exit_thread , NULL);
//     while(1)
//     {
//         string s;
//         cin>>s;
//         if(s=="quit")
//             exit(0);
//     }
// }