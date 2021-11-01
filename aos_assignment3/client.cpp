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

/////////////////////// Function Dfinitions /////////////////////////////////////////////////////////
void communication(int client);
void initialize(string ip);
int connecting(string ip);
void download_file(int con);
void upload_file(int con);
vector<string> tokenizer(string command);
string getfilename(string path);

///////////////////////////// Main Function ///////////////////////////////////////////////////////////
int main(int argc,char const *argv[])
{
    string ip(argv[1]);
    initialize(ip);

    //Connecting with a tracker
    string server_ip(argv[2]);
    int newconnect=connecting(server_ip);
     communication(newconnect);
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
    close(newconnect);
    close(client);
    close(client1);
}


///////////////////////////// Function to handle chat with tracker ///////////////////////////////////////////
void communication(int client)
{
    // char hello[1024]="Hello Server";
    // 
    while(1)
    {
        char buffer[1024]={0};
        string s;
        char msg[256];
        getline(cin >> ws,s);
        //cin >> s;
        // Checking if user wants to logout
        if(s=="logout")
            continue;
        
        vector<string> tokens=tokenizer(s);

        // if user wants to create user id
        if(tokens[0]=="create_user")
        {
            s=s+ " " + my_address+ " " + my_port;

            strcpy(msg,s.c_str());
            msg[s.length()]='\0';
            send(client , msg , strlen(msg) , 0 );

            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="upload_file")
        {
            //This code is not complete
            //Need to calculate number of fragment and send
            //Need to make this modular
            string file_name=getfilename(tokens[1]);
            files_shared[file_name]=tokens[1];
            s=tokens[0]+" "+file_name+" "+tokens[2];
            strcpy(msg,s.c_str());
            msg[s.length()]='\0';
            send(client , msg , strlen(msg) , 0 );

            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else if(tokens[0]=="list_groups")
        {
            strcpy(msg,s.c_str());
            msg[s.length()]='\0';
            send(client , msg , strlen(msg) , 0 );
            vector<string> log;
            char groups[125];
            while(1)
            {
                memset(groups,'\0',sizeof(groups));
                int size=recv(client ,groups , sizeof(groups),0);
                if(strcmp(groups,"stop")==0)
                    break;
                cout<<groups<<endl;
                
            }
        }
        else if(tokens[0]=="create_group")
        {
            strcpy(msg,s.c_str());
            msg[s.length()]='\0';
            send(client , msg , strlen(msg) , 0 );
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
        else
        {
            strcpy(msg,s.c_str());
            msg[s.length()]='\0';
            send(client , msg , strlen(msg) , 0 );
            int recieve=read(client,buffer,sizeof(buffer));
            //cout<<recieve;
            cout<<buffer<<endl;
        }
    }
}


////////////////////////////////////////////////Initializing server socket////////////////////////////////

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
    cout<<endl;
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
