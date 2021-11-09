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

class files_shared
{
    public:
    string name;
    unordered_map<string,string> chunks; //This unordered maps user name to file chunks in form of username/grpid to binary string
    int no_of_chunks;
    files_shared(string username,string filename,string noc,string bitmap="1")
    {
        name=filename;
        no_of_chunks=stoi(noc);
        chunks[username]=bitmap;
    }
    void delete_entry(string usrid)
    {
        if(chunks.find(usrid)!=chunks.end())
            chunks.erase(usrid);
    }
    bool isEmpty()
    {
        if(chunks.size())
            return false;
        return true;
    } 

}; 

class group
{
    public:
    string grpid;
    string owner;
    unordered_map<string,string> users;
    //unordered_map<string,vector< set<sockaddr_in> >> files;
    unordered_map<string,string> pending_list;
    

    unordered_map<string,files_shared *> files;

    group(string usr,string group_id,string owner_det)
    {
        owner=usr;
        grpid=group_id;
        users[owner]=owner_det;
    }

    void remove_user(string usrid)
    {
        vector<string> temp;
        users.erase(usrid);
        for(auto i=files.begin(); i!=files.end() ; i++)
        {
            i->second->delete_entry(usrid);
            if(i->second->isEmpty())
                temp.push_back(i->first);
        }

        for(string s:temp)
        {
            delete files[s];
            files.erase(s);
        }
    }

    void remove_user_temp(string usrid)
    {
        vector<string> temp;
        //users.erase(usrid);
        for(auto i=files.begin(); i!=files.end() ; i++)
        {
            i->second->delete_entry(usrid);
            if(i->second->isEmpty())
                temp.push_back(i->first);
        }

        for(string s:temp)
        {
            delete files[s];
            files.erase(s);
        }
    }

    void stop_sharing(string username,string filename)
    {
        files[filename]->delete_entry(username);
        if(files[filename]->isEmpty())
        {
            delete files[filename];
            files.erase(filename);
        }
    }

    void sharefile(string username,string filename,string chunk_no,string bitmap="1")
    {
        if(files.find(filename)==files.end())
        {
            files_shared *file1= new files_shared(username,filename,chunk_no,bitmap);
            files[filename]=file1;
        }
        else
        {
            files[filename]->chunks[username]=bitmap;
        }
    }

    vector<string> getfiledetails(string filename)
    {
        vector<string> ans;
        if(files.find(filename)==files.end())
        {
            cout<<"Empty"<<endl;
            return ans;
        }
        //cout<<files[filename]->chunks.size()<<endl;
        for(auto i=files[filename]->chunks.begin();i!=files[filename]->chunks.end();i++)
        {
            string s=users[i->first];
            //s=s+" "+i->second;
            //cout<<s<<endl;
            ans.push_back(s);
        }

        return ans;
    }

    ~group()
    {
        for(auto i=files.begin(); i!=files.end() ; i++)
        {
            delete i->second;
        }
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
    unordered_map<string,files_shared *> files; //The file name will be mapped to binary string which tell which peices are there
    //unordered_map<string,string> file_group; //This helps to map file name to grpid
    unordered_set<string> usr_group; //All the groups in which this peer is
    peers(int soc,string ip,string port,string usr, string pass)
    {
        peer_fd=soc;
        peer_ip=ip;
        peer_port=port;
        username=usr;
        password=pass;
    }
    void remove_group(string group_id)
    {
        usr_group.erase(group_id);
        vector<string> temp;
        for(auto i=files.begin(); i!=files.end() ; i++)
        {
            i->second->delete_entry(group_id);
            if(i->second->isEmpty())
                temp.push_back(i->first);
        }

        for(string s:temp)
        {
            delete files[s];
            files.erase(s);
        }
    }
    void stop_sharing(string grpid,string filename)
    {
        files[filename]->delete_entry(grpid);
        if(files[filename]->isEmpty())
        {
            delete files[filename];
            files.erase(filename);
        }
    }
    void sharefile(string group_id,string filename,string chunk_no,string bitmap="1")
    {
        if(files.find(filename)==files.end())
        {
            files_shared *file1= new files_shared(group_id,filename,chunk_no,bitmap);
            files[filename]=file1;
        }
        else
        {
            files[filename]->chunks[group_id]=bitmap;
        }
    }

    ~peers()
    {
        for(auto i=files.begin(); i!=files.end() ; i++)
        {
            delete i->second;
        }
    }
};

struct arguments //No londer required
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
void * handler(void *args);
char * create_account(int fd,string ip,string port,string username,string password);
bool authenticate(string username,string password);
void clearing();
vector<string> tokenizer(string command);
char * create_group(string username,string group_id);
char * join_group(string username,string grpid);
char * accept_request(string owner,string group_id,string username);
char * requests(int com_soc,string owner,string group_id);
char * leave_group(string username,string group_id);
char * list_files(int com_soc,string username,string group_id);
char * upload_file(int com_soc,string username,string filename,string group_id,string chunk_no);
char * download_file(int com_soc,string username,string group_id,string filename);
char * stop_share(string username,string group_id,string filename);
void logout(string username);

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
        if(strcmp(buffer,"quit")==0)
        {
            ans=false;
            return NULL;
        }
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
            cout<<"In checking phase\n";
            char buff[1024]={0};
            int valread = read( com_soc , buff, 1024);
            if(strcmp(buff,"quit")==0)
            {
                ans=false;
                return NULL;
            }
            string data(buff);
            vector<string> tokens=tokenizer(data);
            if(tokens[0]!="login")
                reply="Login first\n";
            else
            {
                username=tokens[1];
                string password=tokens[2];
            //char *reply;
                ans=authenticate(username,password);
                if(ans)
                    reply="Log in sucessful";
                else
                    reply="Login Failed .... try again";
            }
            send(com_soc , reply , strlen(reply) , 0 );
            memset(buff,'\0',1024);
        }

        //From this point onwards we will recieve commands
        
        char buffer[1024]={0};
        int valread = recv( com_soc , buffer, 1024, 0); ///Replace in place of read
        if(strcmp(buffer,"logout")==0)
        {
            ans=false;
            logout(username);
            continue;
        }
        if(strcmp(buffer,"quit")==0)
        {
            ans=false;
            return NULL;
        }
        //cout<<buffer;
        string data(buffer);
        vector<string> tokens=tokenizer(data);
        //cout<<tokens[0]<<endl<<flush;
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
            reply=leave_group(username,tokens[1]);

        }
        else if(tokens[0]=="stop_share")
        {
            reply=stop_share(username,tokens[1],tokens[2]);
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
            reply=list_files(com_soc,username,tokens[1]);
        }
        else if(tokens[0]=="upload_file")
        {
            //code here
            //cout<<data<<endl;
            reply=upload_file(com_soc,username,tokens[1],tokens[2],tokens[3]);
        }
        else if(tokens[0]=="download_file")
        {
            //code here
            reply=download_file(com_soc,username,tokens[1],tokens[2]);
        }
        else if(tokens[0]=="accept_request")
        {
            reply=accept_request(username,tokens[1],tokens[2]);
        }
        else if(tokens[0]=="requests" && tokens[1]=="list_requests")
        {
            reply=requests(com_soc,username,tokens[2]);
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

////////////////////////////////////////////////////// Leaving Group ///////////////////////////////////////////////////////////

char * leave_group(string username,string group_id)
{
    //cout<<"In leave_group phase"<<endl;
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->users.find(username)==groups[group_id]->users.end())
        return "You are not part of the group so no need to leave";
    if(groups[group_id]->owner==username)
    {
        //cout<<"Going to go inside loop"<<endl;
        vector<string> users;
        for(auto i=groups[group_id]->users.begin(); i!=groups[group_id]->users.end() ; i++)
        {
            users.push_back(i->first);
        }
        for(string s:users)
        {
            groups[group_id]->remove_user(s);
            //cout<<"Removed user from group"<<endl;
            clients[s]->remove_group(group_id);
            //cout<<"remove group from user"<<endl;
        }
        //cout<<"Outside the loop"<<endl;
        delete groups[group_id];
        groups.erase(group_id);
        return "As you were owner so group was deleted";
    }
    else
    {
        groups[group_id]->remove_user(username);
        clients[username]->remove_group(group_id);
        return "You are no longer part of this group";
    }
}

/////////////////////////////////////// Authenticate User //////////////////////////////////////////////////

bool authenticate(string username,string password)
{
    if(clients.find(username)==clients.end())
        return false;
    if( clients[username]->password== password)
    {
        for(auto i=clients[username]->files.begin();i!=clients[username]->files.end() ; i++)
        {
            for(auto j=i->second->chunks.begin(); j!=i->second->chunks.end();j++)
            {
                //j->first is grp id
                groups[j->first]->sharefile(username,i->first,to_string(i->second->no_of_chunks),j->second);
            }
        }
        return true;
    }
    return false;
}
//////////////////////////////////////////// Accepting a request //////////////////////////////////////////////

char * accept_request(string owner,string group_id,string username)
{
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->owner!=owner)
        return "You are authorized to use this command";
    if(groups[group_id]->pending_list.find(username)==groups[group_id]->pending_list.end())
        return "Wrong userid accepted";
    groups[group_id]->users[username]=groups[group_id]->pending_list[username];
    groups[group_id]->pending_list.erase(username);
    clients[username]->usr_group.insert(group_id);
    username="Accepted "+username;
    char msg[256];
    strcpy(msg,username.c_str());
    msg[username.length()]='\0';
    return "Accepted user";
}

///////////////////////////////////////////// Getting the pending list ///////////////////////////////////////
char * requests(int com_soc,string owner,string group_id)
{
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->owner!=owner)
        return "You are authorized to use this command";  
    
    char temp[20]="Accepted";
    
    send(com_soc , temp , strlen(temp) , 0 );
    usleep(200);
    for(auto i=groups[group_id]->pending_list.begin();i!=groups[group_id]->pending_list.end() ; i++)
    {
        memset(temp,'\0',sizeof(temp));
        strcpy(temp,i->first.c_str());
        temp[i->first.length()]='\0';
        send(com_soc , temp , strlen(temp) , 0 );
        usleep(200);
    }
    usleep(200);
    return "stop";
}

////////////////////////////////////////////// Lists All files ///////////////////////////////////////////////
char * list_files(int com_soc,string username,string group_id)
{
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->users.find(username)==groups[group_id]->users.end())
        return "You are not part of this group";
    char temp[20]="Accepted";
    
    send(com_soc , temp , strlen(temp) , 0 );
    usleep(200);
    for(auto i=groups[group_id]->files.begin();i!=groups[group_id]->files.end() ; i++)
    {
        memset(temp,'\0',sizeof(temp));
        strcpy(temp,i->first.c_str());
        temp[i->first.length()]='\0';
        send(com_soc , temp , strlen(temp) , 0 );
        usleep(200);
    }
    usleep(200);
    return "stop";
}

///////////////////////////////////////////// Stop Share /////////////////////////////////////////////////////

char * stop_share(string username,string group_id,string filename)
{
     if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->users.find(username)==groups[group_id]->users.end())
        return "You are not part of this group";
    groups[group_id]->stop_sharing(username,filename);
    clients[username]->stop_sharing(group_id,filename);
    return "Stop Sharing file successfully";
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

//////////////////////////////////////////////// Create Group ///////////////////////////////////////////////////////////////////

char * create_group(string username,string group_id)
{
    if(groups.find(group_id)!=groups.end())
        return "This group already exist";
    string usr_det=clients[username]->peer_ip+":"+clients[username]->peer_port;
    group *new_group= new group(username,group_id,usr_det);
    groups[group_id]=new_group;
    clients[username]->usr_group.insert(group_id);
    return "Group Created";
}

/////////////////////////////////////////////// Join a Group ////////////////////////////////////////////////////////////////////

char * join_group(string username,string group_id)
{
    if(groups.find(group_id)==groups.end())
        return "No such group exist";
    if(groups[group_id]->pending_list.find(username)!=groups[group_id]->pending_list.end())
        return "Already applied.. still in waiting list";
    if(groups[group_id]->users.find(username)!=groups[group_id]->users.end())
        return "Already part of group";
    string usr_det=clients[username]->peer_ip+":"+clients[username]->peer_port;
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

///////////////////////////////////// Upload File ////////////////////////////////////////////////////////

char * upload_file(int com_soc,string username,string filename,string group_id,string chunk_no)
{
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->users.find(username)==groups[group_id]->users.end())
        return "You are not part of this group";
    groups[group_id]->sharefile(username,filename,chunk_no);
    clients[username]->sharefile(group_id,filename,chunk_no);
    //cout<<groups[group_id]->files[filename]->chunks[username]<<endl;
    return "Uploaded Successfully";
}

//////////////////////////////////////// Download File //////////////////////////////////////////////////

char * download_file(int com_soc,string username,string group_id,string filename)
{
    //cout<<"In download_file phase"<<endl;
    if(groups.find(group_id)==groups.end())
        return "Group Doesn't Exist";
    if(groups[group_id]->users.find(username)==groups[group_id]->users.end())
        return "You are not part of this group";
    if(groups[group_id]->files.find(filename)==groups[group_id]->files.end())
        return "No such file exist";
    //cout<<"Before call"<<endl;
    
    char temp[1000]="Accepted";
    //cout<<"Aftercall"<<endl;    
    send(com_soc , temp , strlen(temp) , 0 );
    vector<string> det=groups[group_id]->getfiledetails(filename);
    usleep(200);
    string header=filename+" "+to_string(groups[group_id]->files[filename]->no_of_chunks);
    det.insert(det.begin(),header);
    for(int i=0;i<det.size();i++)
    {
        memset(temp,'\0',sizeof(temp));
        strcpy(temp,det[i].c_str());
        temp[det[i].length()]='\0';
        send(com_soc , temp , strlen(temp) , 0 );
        usleep(200);
    }
    usleep(200);
    upload_file(com_soc,username,filename,group_id,to_string(groups[group_id]->files[filename]->no_of_chunks));
    return "stop";
}


/////////////////////////////////////////////////// Logout //////////////////////////////////////////////////

void logout(string username)
{
    vector<string> grps;
    for(auto i=clients[username]->usr_group.begin();i!=clients[username]->usr_group.end() ; i++)
        grps.push_back(*i);
    for(string grp:grps)
    {
        groups[grp]->remove_user_temp(username);
    }
}
