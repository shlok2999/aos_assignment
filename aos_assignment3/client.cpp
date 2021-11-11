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

/////////////////////////////////////////// Classess For files //////////////////////////////////
class chunk_detail
{
    public:
    int num;
    string ip;
    string file;
    string username;
    int fd;
    chunk_detail(int no,string add,string name,int des,string usrname)
    {
        num=no;
        ip=add;
        file=name;
        fd=des;
        username=usrname;
    }
};

class files_downloaded
{
    public:
    string name;
    string destination;
    string group_id;
    char status;
    files_downloaded(string filename,string dest,string grp_id)
    {
        name=filename;
        destination=dest;
        status='D';
        group_id=grp_id;
    }
};

class download_file_args
{
    public:
    vector<string>peers;
    string username;
    files_downloaded* d_data;
    download_file_args(vector<string> list,string usr, files_downloaded* d)
    {
        // cout<<"Object Created";
        peers=list;
        username=usr;
        d_data = d;
    }
};


class files_sharable
{
    public:
    string name;
    string destination;
    string bitmap;
    files_sharable(string filename,string dest,string bits)
    {
        name=filename;
        destination=dest;
        bitmap=bits;
    }
    void update_bitmap(int k)
    {
        bitmap[k]='1';
    }
};
////////////////////////////////////////// Global Variables ////////////////////////////////////
int port=2020;
int client,client1;
struct sockaddr_in client_address,c_addr;
string my_address,my_port;
string tracker_ip;
const int file_buffer_size=512*1024;
unordered_map<string,files_sharable *> files_shared;
vector<files_downloaded *> files_down;
//unordered_map<string,char> files_status;

/////////////////////// Function Dfinitions /////////////////////////////////////////////////////////
void * clearing (void *arg);
void * communication(void *arg);
void * listener(void *arg);
void * communictaing(void *arg);
void sending(int com_soc,string msg);
void initialize(string ip);
int connecting(int client_con,string ip);
void * download_file(void *arg);
void download_chunk(chunk_detail args);
void * upload_file(void *arg);
vector<string> tokenizer(string command);
string getfilename(string path);
int getfilesize(string file);
string getbitmap(int n);
string get0bitmap(int n);
void clearing(int sig);

///////////////////////////// Main Function ///////////////////////////////////////////////////////////
int main(int argc,char const *argv[])
{
    signal(SIGINT, clearing);
    //atexit(clearing);
    string ip(argv[1]);
    initialize(ip);

    //Connecting with a tracker
    string server_ip(argv[2]);
    tracker_ip=server_ip;
    int newconnect=connecting(client1,server_ip);
    
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
    // string conn="Connect";
    // sending(client,conn);
    string username;
    //usleep(2);
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
            username=tokens[1];
            s=s+ " " + my_address+ " " + my_port;
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
            string file_name=getfilename(tokens[1]);
            

            int n=getfilesize(tokens[1]);
            if(n==-1)
                continue;
            string bitmap=getbitmap(n);
            //string file_name=getfilename(tokens[1]);

            if(files_shared.find(file_name)==files_shared.end())
            {
                files_shared[file_name]= new files_sharable(file_name,tokens[1],bitmap);
            }
            //cout<<files_shared[file_name]->bitmap<<endl;
            s=tokens[0]+" "+file_name+" "+tokens[2]+" "+to_string(n);
            sending(client,s);

            int recieve=read(client,buffer,sizeof(buffer));
            
             cout<<buffer<<endl;
        }
        else if(tokens[0]=="download_file" && tsize==4)
        {
            string dest=tokens[3]+'/'+tokens[2];
            
            
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
                    //cout<<p<<endl;
                
                }
                
                if(files_shared.find(tokens[2])!=files_shared.end())
                {
                    cout<<"You already have this file";
                    continue;
                }
                files_downloaded* new_d =new files_downloaded(tokens[2],dest,tokens[1]);
                vector<string> header=tokenizer(list_of_peers[0]);
                string bitmap=get0bitmap(stoi(header[1]));
                //cout<<"Files data fetched"<<endl;
                    
                files_shared[tokens[2]]=new files_sharable(tokens[2],dest,bitmap);

                //cout<<"After new"<<endl;
                pthread_t p;
                // for(string s:list_of_peers)
                //     cout<<s<<endl;
                // cout<<username<<endl;
                download_file_args *args=new download_file_args(list_of_peers,username, new_d);
                // cout<<args.username<<endl;
                // for(string s1:args.peers)
                //     cout<<s1<<endl;
                cout<<"File will start to dowload"<<endl;
                files_down.push_back(new_d);
                pthread_create(&p,NULL,download_file,args);
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
        else if(tokens[0]=="stop_share" && tsize==3)
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
        else if(tokens[0]=="show_downloads")
        {
            for(auto i : files_down)
            {
                cout<<"["<<i->status<<"]["<<i->group_id<<"]"<<i->name<<endl;
            }
        }
        else
        {
            cout<<"Wrong command\n";
            continue;
        }
    }
}

/////////////////////////////////////////////// Sending a msg function //////////////////////////////////////////////////////
void sending(int com_soc,string s)
{
    char msg[256]={0};
    strcpy(msg,s.c_str());
    msg[s.length()]='\0';
    int n=send(com_soc , msg , strlen(msg) , 0 );
    if(n<0)
        cout<<"Not able to send"<<endl;
    //return msg;
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
    int socket_option=setsockopt(client,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));
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
    int client_listen=listen(client,1000);
    if(client_listen<0)
    {
        cout<<"Couldn't Listen to server\n";
        exit(1);
    }
}


////////////////////////////////////// To connect to a peer /////////////////////////////////////////////////////
int connecting(int client_con,string ip)
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
    int newconnect=connect(client_con, (struct sockaddr*)&socket_address,sizeof(socket_address));
    //cout<<errno;
    if(newconnect<0)
    {
        cout<<"Error in making connection";
        exit(1);
    }
    return client_con;
}

//////////////////////////////////// Downloading file /////////////////////////////////////////////////////////////////////////

bool comparator(vector<pair<int,string>> a,vector<pair<int,string>>b)
{
    return a.size()<b.size();
}

void * download_file(void *arg)
{

    // cout<<"In download File"<<endl;
    download_file_args args=*((download_file_args*)arg);
    
    download_file_args *temp=(download_file_args*)arg;
    
    vector<string> peers=args.peers;
    // cout<<"retrieveing data"<<endl;
    //string username=args.username;
    vector<string> header=tokenizer(peers[0]);
    // cout<<"Before erase"<<endl;
    // for(string s:peers)
    //     cout<<s<<endl;
    
    peers.erase(peers.begin());
    // cout<<"After erase:"<<endl;
    // for(string s:peers)
    //     cout<<s<<endl;
    vector<string> details;
    //cout<<header[0]<<endl;
    for(string peer:peers)
    {
        int newconnect=socket(AF_INET, SOCK_STREAM , 0);
        if(newconnect<0)
        {
            cout<<"Error in creating socket"<<endl;
        }
        connecting(newconnect,peer);
        //cout<<"connected to "<<peer<<endl;
        string s="bitmap "+header[0];
        sending(newconnect,s);
        char* buffer = new char[1024];
        int readval=recv(newconnect,buffer,sizeof(buffer),0);
        if(readval<0)
        {
            cout<<readval;
            details.push_back("");
            continue;
        }
        string bitmap(buffer);
        details.push_back(bitmap);
        //cout<<"Bitmap recieved is:"<<bitmap<<endl;
        close(newconnect);
        delete buffer;

    }

    //cout<<"Bitmap Retreived "<<endl;
    //cout<<header[1]<<endl;
    int num_of_chunks=stoi(header[1]);
    // vector<pair<int,string>> chunks[num_of_chunks+2];
    // for(int j=0;j<details.size();j++)
    // {
    //     for(int i =0;i<details[j].length();i++)
    //     {
    //         if(details[j][i]=='1')
    //         {
    //             chunks[i+1].push_back({i+1,peers[j]});
    //             //cout<<peers[j];
    //         }
                
    //     }
    // }

    // sort(chunks,chunks+num_of_chunks+1,comparator);
    
    // cout<<"All chunks detail printed"<<endl;
    //pthread_t threads[num_of_chunks];

    string file=temp->d_data->destination;
    char file_desc[FILENAME_MAX];
    strcpy(file_desc,file.c_str());
    file_desc[file.length()]='\0';
    //cout<<"Converted to char array"<<endl;
    int fd=open(file_desc,O_WRONLY | O_CREAT, S_IRUSR| S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);

    for(int i=0;i<num_of_chunks;i++)
    {
        //cout<<"Creating a thread"<<endl;
        chunk_detail c(i,peers[peers.size()-1],header[0],fd,"");
        //pthread_create(&threads[i-1],NULL,download_chunk,c);
        download_chunk(c);
        //usleep(100);
    }

    // for(int i=0;i<num_of_chunks;i++)
    // {
    //     pthread_join(threads[i],NULL);
    // }

    close(fd);
    temp->d_data->status='C';
    files_shared[header[0]]->bitmap=getbitmap(num_of_chunks);
    //cout<<"Download Completed"<<endl;
    delete temp;
    return NULL;
}

/////////////////////////////////////// Upload File ////////////////////////////////////////////////////////

void * upload_file(void *arg)
{
//    cout<<"In upload phase"<<endl;
   int com_soc=*(int *)arg;
//    cout<<com_soc<<endl;
   char buffer[1024]={0};
   int recieve=recv(com_soc,buffer,sizeof(buffer),0);
   string msg(buffer);
   vector<string> tokens=tokenizer(msg);
//    cout<<msg<<endl;
   if(tokens[0]=="bitmap")
   {
       string s=files_shared[tokens[1]]->bitmap;
       //cout<<s;
       sending(com_soc,s);
       close(com_soc);
       return NULL;
   }
    char file[FILENAME_MAX];
    
    strcpy(file,files_shared[tokens[1]]->destination.c_str());
    file[files_shared[tokens[1]]->destination.length()]='\0';
    //cout<<file<<flush;
    int chunk_num=stoi(tokens[0]);
    int chunk_offset=chunk_num*file_buffer_size;
    char buff[file_buffer_size];
    //cout << "Opening " << file << endl;
    int fd=open(file,O_RDONLY);
    // cout << "FD: " << fd << endl;
    int n=pread(fd,buff,file_buffer_size,chunk_offset);
    // cout << n << " chars read" << endl;
    send(com_soc,buff,n,0);
    memset(buffer,'\0',sizeof(buffer));
    // recv(com_soc,buffer , sizeof(buffer) , 0);
    // cout<<"Buffer";
    close(fd);
    close(com_soc);
    return NULL;
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
        int check=pthread_create(&new_thread , NULL , upload_file , &newconnect);
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

string get0bitmap(int n)
{
    string s="";
    for(int i=0;i<n;i++)
        s=s+'0';
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


/////////////////////////////////////////////// Downloading a Chunk //////////////////////////////////////////
void download_chunk(chunk_detail args)
{
    //chunk_detail args=*(chunk_detail*)arg;
    //chunk_detail *temp=(chunk_detail*)arg;
    
    // cout<<"In downloading phase"<<endl;
    int newconnect=socket(AF_INET, SOCK_STREAM , 0);
    if(newconnect<0)
        return;
    
    connecting(newconnect,args.ip);

    string s_msg=to_string(args.num) + " " + args.file;
    // cout<<"Sending Request for chunk"<<args.num<<endl;
    sending(newconnect,s_msg);
    char* buff = new char[file_buffer_size];
    int file_pointer=0;
    while(1)
    {
        int n=recv(newconnect,buff+file_pointer,file_buffer_size,0);
        if(n<=0)
            break; 
        file_pointer+=n;
    }
    // cout<<"file recieved in buffer for chunk"<<args.num<<endl;
    int file_offset=(args.num)*file_buffer_size;
    // cout<<"File Pointer:"<<file_pointer;
    pwrite(args.fd,buff,file_pointer,file_offset);
    // cout<<"Written in file"<< endl;
    // close(fd);
    // cout<<"Closed file desc"<<endl;
    //char msg[256]="Completed";
    //send(newconnect,msg,strlen(msg),0);
    //files_shared[args.file]->update_bitmap(args.num-1);
    // string s="Completed";
    // sending(newconnect,s);
    // cout<<"Updated Bitmap"<<endl;
    close(newconnect);
    // cout<<"Closed Socket"<<endl;
    // connecting(newconnect,args.fd);
    // string s="upload_file "+args.file+" ";
    // cout<<"Downloaded chunk"<<file_offset<<endl;
    //delete temp;
    // cout<<"Deleting temp"<<endl;
}
//////////////////////////////// Clearing out ////////////////////////////////////////////////////////////////
void clearing(int sig)
{
    // cout<<client1<<endl;
    string s="quit";
    sending(client1,s);
    for(auto i=files_shared.begin(); i!=files_shared.end();i++)
    {
        delete i->second;
    }

    for(auto i : files_down)
        delete i;
    
    exit(0);

}
