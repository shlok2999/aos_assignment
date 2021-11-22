#include<bits/stdc++.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

///////////////////////////////////// Constant Global Variables //////////////////////////////
const size_t Block_size=4*1000; //4KB size of disk ball
const size_t no_of_blocks=512*1000/4;
const size_t no_of_inode=7865;
////////////////////////////////// Class Defination //////////////////////////////

class inode_structure
{
    public:
    int file_size;
    int direct_pointer[10];
    int s_indirect_pointer;
    int d_indirect_pointer;

    inode_structure()
    {
        file_size=0;
        //memset(direct_pointer,-1,sizeof(direct_pointer));
        for(int i=0;i<10;i++)
            direct_pointer[i]=-1;
        // for(int &i:direct_pointer)
        //     i=-1;
        s_indirect_pointer=-1;
        d_indirect_pointer=-1;
    }
};

class file_inode
{
    public:
    char name[100];
    int inode;
    file_inode()
    {
        //name=new char[FILENAME_MAX];
        //memset(name,'\0',sizeof(name));
        //strcpy(name,"");
        inode=-1;
    }
};

class super_block
{
    public:
    int no_of_sup_block;
    int no_of_bitmap_block;
    int no_of_inode_block;
    int no_of_data_block;
    bool free_inode[no_of_inode];
    bool free_db[no_of_blocks];

    super_block()
    {
        no_of_sup_block=ceil(sizeof(super_block)/Block_size);
        
        no_of_bitmap_block=ceil((no_of_inode*sizeof(file_inode))/Block_size);
        no_of_inode_block=ceil((no_of_inode*sizeof(inode_structure))/Block_size);
        no_of_data_block=no_of_data_block - no_of_inode_block - no_of_sup_block - no_of_bitmap_block;
        
        for(int i=0;i<no_of_inode;i++)
            free_inode[i]=0;

        for(int i=0;i<no_of_blocks;i++)
            free_db[i]=0;

        int n=no_of_sup_block + no_of_bitmap_block + no_of_inode_block;
        //cout<<n<<" "<<no_of_blocks<<" ";
        for(int i=0;i<n;i++)
            free_db[i]=1;
    }

};

///////////////////////////////////////////////// Global Variables ///////////////////////////////////
inode_structure inode[no_of_inode];
file_inode files[no_of_inode];
super_block super;
char name[FILENAME_MAX];
unordered_map <string,int> file_to_inode;
unordered_map <int,string> inode_to_file;
queue <int> free_blocks;
queue <int> free_inodes;
int fd;
unordered_map <string , pair<int,string>> opened_files;
unordered_map <int,string> file_descriptors;


/////////////////////////////////////////// Function Defination /////////////////////////////////////////////////////////////
void create_disk();
int display_menu();
void mount_disk();
void load_disk(char *disk_name);
void mount_disk_menu();
void unload_disk();
int display_mount_menu();
void create_file();
void open_file();
void read_file();
void write_file();
void append_file();
void close_file();
void delete_file();
void list_of_files();
void list_of_open_files();
void write_content(long long int offset, string content);

/////////////////////////////////////////// Main Function ///////////////////////////////////////////////////



int main()
{
    while (1)
    {
        int op=display_menu();
        if(op == 1 )
            create_disk();
        else if(op == 2 )
            mount_disk();
        else if(op == 3)
            break;
        else
            cout<<"Wrong Option selected"<<endl;
    }
}

///////////////////////////////////////////// Create disk ////////////////////////////////////////////////////

void create_disk()
{
    cout<<"Enter the file name:"<<endl;
    char disk_name[FILENAME_MAX];
    //cout<<"Enter the file name:"<<endl;
    cin>>disk_name;
    //cout<<disk_name<<endl;

    struct stat st;
    if(stat(disk_name,&st) != -1)
    {
        cout<<"Disk Already exist can't create new disk with same name"<<endl;
        return;
    }

    FILE *disk=fopen(disk_name,"wb");
    //fclose(disk);
    if(disk == NULL)
    {
        cout<<"Unable to open file";
        return;
    }
    
    char buff[Block_size]={0};

    // for(int i=0;i<no_of_blocks;i++)
    //     fwrite(buff,Block_size,1,disk);

    fseek(disk,no_of_blocks * Block_size,SEEK_SET);
    fwrite(" ",1,1,disk);
    
    super_block meta;
    //cout<<no_of_inode;
    file_inode f[no_of_inode];
    inode_structure i[no_of_inode];

    for(int i=0;i<no_of_inode;i++)
        cout<<f[i].inode<<" "<<f[i].name<<endl;


    fseek(disk,0,SEEK_SET);
    fwrite(&meta,sizeof(meta),1,disk);

    // Writing files' meta in disk 
    long long int offset = (meta.no_of_sup_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fwrite(f,sizeof(file_inode),no_of_inode,disk);
    cout<<offset<<endl;
    //Writing inodes of the file in the disk
    offset = (meta.no_of_sup_block + meta.no_of_bitmap_block) *Block_size ;
    cout<<offset<<endl;
    fseek(disk,offset,SEEK_SET);
    fwrite(i,sizeof(inode_structure),no_of_inode,disk);

    //closing file
    fclose(disk);
    cout<<"Disk Created succesfully"<<endl;
}

////////////////////////////////////////// Display Menu //////////////////////////////////////////////

int display_menu()
{
    int option;
    cout<<"----------------------"<<endl;
    cout<<"1. Create Disk"<<endl;
    cout<<"2. Mount Disk"<<endl;
    cout<<"3. Exit"<<endl;
    cout<<"Please enter interger option for exit type any num:";
    cin>>option;
    return option;
}

///////////////////////////////////////// Mount Disk //////////////////////////////////////////////////

void mount_disk()
{
    string disk;
    char disk_name[FILENAME_MAX];
    cout<<"Enter the disk you want to mount:"<<endl;
    getline(cin >> ws,disk);
    strcpy(disk_name,disk.c_str());
    //cin.getline(disk_name,FILENAME_MAX);
    strcpy(name,disk_name);
    cout<<name<<endl;
    struct stat st;
    if(stat(disk_name,&st) == -1)
    {
        cout<<"No such disk exist"<<endl;
        return;
    }
    cout<<"Going to loading disk";
    load_disk(disk_name);
    mount_disk_menu();
    unload_disk();
}

/////////////////////////////////////////// Loading disk ///////////////////////////////////////////////

void load_disk(char *disk_name)
{
    // Reading file to main memory
    cout<<disk_name<<endl;
    FILE *disk=fopen( disk_name, "rb+");
    if(disk == NULL)
    {
        cout<<"Unable to open file"<<endl;
        exit(0);
    }

    fread(&super,sizeof(super_block),1,disk);
    cout<<"Super block Read"<<endl;

    // files=new file_inode[no_of_inode];
    cout<<"New file array is created"<<endl;
    long long int offset = (super.no_of_sup_block) *Block_size ;
    cout<<"Offset Calcultaed"<<endl;
    fseek(disk,offset,SEEK_SET);
    cout<<"File pointer is set"<<endl;
    fread(files,sizeof(file_inode),no_of_inode,disk);
    cout<<"Files data is read"<<endl;
    //inode=new inode_structure[no_of_inode]; 
    offset = (super.no_of_sup_block + super.no_of_bitmap_block) *Block_size ;
    cout<<"Offset calculated again"<<endl;
    fseek(disk,offset,SEEK_SET);
    cout<<"File pointer is set again to correct place"<<endl;
    fread(inode,sizeof(inode_structure),no_of_inode,disk);
    cout<<"Inode is updated successfully"<<endl;
    fclose(disk);

    cout<<"File is closed"<<endl;
    for(int i=0;i<no_of_inode;i++)
    {
        cout<<i<<" "<<files[i].inode<<" "<<files[i].name<<endl;
        // if(files[i].inode !=-1)
        // {
        //     cout<<files[i].name<<endl;
        //     string filename(files[i].name);
        //     file_to_inode[filename]=files[i].inode;
        //     inode_to_file[files[i].inode]=filename;
        // }
    }
    cout<<"Maps is initialized"<<endl;
    // Making bitmap
    for(int i=0;i<no_of_inode;i++)
        if(!super.free_inode[i])
            free_inodes.push(i);
    
    for(int i=0;i<no_of_blocks;i++)
        if(!super.free_db[i])
            free_blocks.push(i);

}

//////////////////////////////// Unloading Disk /////////////////////////////////////////////////////////

void unload_disk()
{
    FILE *disk=fopen( name, "rb+");

    if(disk == NULL)
    {
        cout<<"Unable to modify disk....exiting";
        exit(0);
    }

    fseek(disk,0,SEEK_SET);
    fwrite(&super,sizeof(super_block),1,disk);

    // Writing files' meta in disk 
    long long int offset = (super.no_of_sup_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fwrite(files,sizeof(file_inode),no_of_inode,disk);

    //Writing inodes of the file in the disk
    offset = (super.no_of_sup_block + super.no_of_bitmap_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fwrite(inode,sizeof(inode_structure),no_of_inode,disk);

    //closing file
    fclose(disk);

    while(!free_blocks.empty())
        free_blocks.pop();

    while(!free_inodes.empty())
        free_inodes.pop();
    
    file_to_inode.clear();
    inode_to_file.clear();

    cout<<"Disk Unmounted Succesfully"<<endl;
}


//////////////////////////Mount disk menu ///////////////////////////////////////////////////////////

void mount_disk_menu()
{
    fd=open(name,O_RDWR);
    if(fd<0)
    {
        cout<<"Unable to mount file";
        return;
    }


    while(1)
    {
        int op=display_mount_menu();
        if(op == 1)
        {
            create_file();
        }
        else if(op == 2)
        {
            open_file();   
        }
        else if(op==3)
        {
            read_file();
        }
        else if(op == 4)
        {
            write_file();
        }
        else if(op == 5)
        {
            append_file();
        }
        else if(op == 6)
        {
            close_file();   
        }
        else if(op == 7)
        {
            delete_file();
        }
        else if(op == 8)
        {
            list_of_files();
        }
        else if(op == 9)
        {
            list_of_open_files();
        }
        else if(op == 10)
        {
            return;
        }
        else
        {
            cout<<"Wong option selected"<<endl;
        }
    }

    close(fd);
}

///////////////////////////////////////// Dsiplay Mount Menu //////////////////////////////////////

int display_mount_menu()
{
    int op;
    cout<<"--------------------------"<<endl;
    cout<<"1. Create File"<<endl;
    cout<<"2. Open File"<<endl;
    cout<<"3. Read File"<<endl;
    cout<<"4. Write File"<<endl;
    cout<<"5. Append File"<<endl;
    cout<<"6. Close File"<<endl;
    cout<<"7. Delete File"<<endl;
    cout<<"8. List of files"<<endl;
    cout<<"9. List of open files"<<endl;
    cout<<"10. Unmount Disk"<<endl;
    cout<<"Enter Your Choice"<<endl;
    cin>>op;
    return op;
}


////////////////////////////// Create File ///////////////////////////////////////////////////////////

void create_file()
{
    if( free_inodes.empty() || free_blocks.empty())
    {
        cout<<"Cannot create more file files in this disk"<<endl;
        return;
    }
    char file_name[FILENAME_MAX];
    cout<<"Enter the name of file you want to create: "<<endl;
    cin>>file_name;
    string fn(file_name);

    if(file_to_inode.find(fn)!=file_to_inode.end())
    {
        cout<<"This file already exist so not creating"<<endl;
        return;
    }

    file_inode new_file;
    strcpy(new_file.name,file_name);
    int node=free_inodes.front();
    free_inodes.pop();
    new_file.inode=node;
    files[node]=new_file;
    super.free_inode[node]=1;

    file_to_inode[fn]=node;
    inode_to_file[node]=fn;
    cout<<"File created successfully."<<endl;
    return;

}

////////////////////////////////// Open file ///////////////////////////////////////////////////////////

void open_file()
{
    string filename;
    cout<<"Enter the filename to be opened :"<<endl;
    getline(cin >> ws,filename);
    
    if(file_to_inode.find(filename)==file_to_inode.end())
    {
        cout<<"No such file exists"<<endl;
        return;
    }

    if(opened_files.find(filename) != opened_files.end())
    {
        cout<<"File already opened"<<endl;
        return;
    }

    while(1)
    {
        int op;
        cout<<"Choose in which mode to open file"<<endl;
        cout<<"0. Read Mode"<<endl;
        cout<<"1. Write Mode"<<endl;
        cout<<"2. Append Mode"<<endl;
        cout<<"Enter the option:"<<endl;
        cin>>op;

        if(op==0)
        {
            opened_files[filename] = {file_to_inode[filename], "read"};
            file_descriptors[file_to_inode[filename]]="read";
            break;
        }
        else if(op==1)
        {
            opened_files[filename] = {file_to_inode[filename], "write"};
            file_descriptors[file_to_inode[filename]]="write";

            break;
        }
        else if(op==2)
        {
            opened_files[filename] = {file_to_inode[filename], "append"};
            file_descriptors[file_to_inode[filename]]="append";
            break;
        }
        else
        {
            cout<<"Wrong input try again ";
        }
    }
    

    cout<<"File opened successfully"<<endl;
    return ;

}

////////////////////////////////////// Read File /////////////////////////////////////////////////////

void read_file()
{
    int file;
    cout<<"Enter the file descriptor:"<<endl;
    cin>>file;
    if(file_descriptors.find(file)==file_descriptors.end())
    {
        cout<<"Invalid File Descriptor"<<endl;
        return;
    }

    if(file_descriptors[file]!= "read")
    {
        cout<<"File doesnot have read operations"<<endl;
        return;
    }

    inode_structure node=inode[file];
    if(node.file_size==0)
        return;
    for(int i=0;i<10;i++)
    {
        if(node.direct_pointer[i]!=-1)
        {
            char buffer[Block_size]={0};
            long long int offset=node.direct_pointer[i] * Block_size;
            pread(fd,buffer,sizeof(buffer),offset);
            cout<<buffer;
        }
    }
}

///////////////////////////////////// Write File //////////////////////////////////////////////////////

void write_file()
{
    int file;
    cout<<"Enter the file descriptor:"<<endl;
    cin>>file;
    if(file_descriptors.find(file)==file_descriptors.end())
    {
        cout<<"Invalid File Descriptor"<<endl;
        return;
    }

    if(file_descriptors[file]!= "write")
    {
        cout<<"File doesnot have read operations"<<endl;
        return;
    }
    inode_structure &node=inode[file];

    for(int i=0;i<10;i++)
    {
        if(node.direct_pointer[i]!=-1)
        {
            free_blocks.push(node.direct_pointer[i]);
            super.free_db[node.direct_pointer[i]]=0;
            node.direct_pointer[i]=-1;
            
        }
    }

    node.file_size=0;
    string content="";
    bool flag=false;
    while(1)
    {
        string s="";
        getline(cin >> ws,s);
        if(s==":q")
            break;
        flag=true;
        s=s+'\n';
        content=content + s;
    }
    if(flag)
        content.pop_back();
    int j=0;
    while( !free_blocks.empty() && content!="")
    {
        long long int block_no=free_blocks.front();
        free_blocks.pop();
        super.free_db[block_no]=1;
        long long int offset=block_no * Block_size;
        if(content.size()<Block_size)
        {
            node.direct_pointer[j]=block_no;
            j++;
            node.file_size+=content.size();
            write_content(offset,content);
            content="";
        }
        else
        {
            node.direct_pointer[j]=block_no;
            j++;
            node.file_size+=Block_size;
            write_content(offset,content.substr(0,Block_size));
            content=content.substr(Block_size);
        }
    }

    if(content!="")
    {
        cout<<"File was Partially written as disk space is now full"<<endl;
    }
    else
    {
        cout<<"Written to file successfully"<<endl;
    }

    return;
}

///////////////////////////////////// Write Content //////////////////////////////////////////////////

void write_content(long long int offset, string content)
{
    char buffer[Block_size]={0};
    memset(buffer,'\0',sizeof(buffer));
    pwrite(fd,buffer,Block_size,offset);

    strcpy(buffer,content.c_str());

    pwrite(fd,buffer,strlen(buffer),offset);
    return;
}

//////////////////////////////////// Append File //////////////////////////////////////////////////////

void append_file()
{
    int file;
    cout<<"Enter the file descriptor:"<<endl;
    cin>>file;
    if(file_descriptors.find(file)==file_descriptors.end())
    {
        cout<<"Invalid File Descriptor"<<endl;
        return;
    }

    if(file_descriptors[file]!= "write")
    {
        cout<<"File doesnot have read operations"<<endl;
        return;
    }
    inode_structure &node=inode[file];
    int i=0;

    for(;i<10;i++)
    {
        if(node.direct_pointer[i]==-1)
            break;
    }
    i--;
    if(i==-1)
        i=0;
    

    char buffer[Block_size]={0};
    long long int offset=node.direct_pointer[i] * Block_size;
    pread(fd,buffer,sizeof(buffer),offset);

    string orig(buffer);
    string content="";

    if(orig.size()==Block_size)
        i++;
    else
    {
        content=orig;
        free_blocks.push(node.direct_pointer[i]);
        super.free_db[node.direct_pointer[i]]=0;
    }

    bool flag=false;
    while(1)
    {
        string s="";
        getline(cin >> ws,s);
        if(s==":q")
            break;
        flag=true;
        s=s+'\n';
        content=content + s;
    }
    if(flag)
        content.pop_back();


    while( !free_blocks.empty() && content!="")
    {
        long long int block_no=free_blocks.front();
        free_blocks.pop();
        super.free_db[block_no]=1;
        long long int offset=block_no * Block_size;
        if(content.size()<Block_size)
        {
            node.direct_pointer[i]=block_no;
            i++;
            node.file_size+=content.size();
            write_content(offset,content);
            content="";
        }
        else
        {
            node.direct_pointer[i]=block_no;
            i++;
            node.file_size+=Block_size;
            write_content(offset,content.substr(0,Block_size));
            content=content.substr(Block_size);
        }
    }

    if(content!="")
    {
        cout<<"File was Partially appended as disk space is now full"<<endl;
    }
    else
    {
        cout<<"Appended to file successfully"<<endl;
    }

}

/////////////////////////////////// Close File ////////////////////////////////////////////////////////

void close_file()
{
    int file;
    cout<<"Enter the file descriptor:"<<endl;
    cin>>file;
    if(file_descriptors.find(file)==file_descriptors.end())
    {
        cout<<"Invalid File Descriptor"<<endl;
        return;
    }

    file_descriptors.erase(file);
    opened_files.erase(inode_to_file[file]);

    cout<<"File closed successfully"<<endl;

    return;
}

//////////////////////////////////// Delete File ///////////////////////////////////////////////////////

void delete_file()
{
    string filename;
    cout<<"Enter the filename";
    getline(cin >> ws,filename);

    if(opened_files.find(filename)!=opened_files.end())
    {
        cout<<"This file is opened so it can't be deleted"<<endl;
        return;
    }

    if(file_to_inode.find(filename)==file_to_inode.end())
    {
        cout<<"No such file exists"<<endl;
        return;
    }

    int node=file_to_inode[filename];
    files[node].inode=-1;
    strcpy(files[node].name,"");
    free_inodes.push(node);
    super.free_inode[node]=0;
    inode[node].file_size=0;

    for(int i=0;i<10;i++)
        inode[node].direct_pointer[i]=-1;

    file_to_inode.erase(filename);
    inode_to_file.erase(node);    
    cout<<"File Deleted Sucessfully"<<endl;
}

///////////////////////////////// List of Files ////////////////////////////////////////////////////////

void list_of_files()
{
    for(auto i=file_to_inode.begin(); i!=file_to_inode.end() ; i++)
    {
        cout<<i->first<<"\t, node number:"<<i->second<<endl;

    }
}


////////////////////////////////// List of open Files /////////////////////////////////////////////////

void list_of_open_files()
{
    for(auto i=opened_files.begin(); i!=opened_files.end();i++)
    {
        cout<<i->first<<"\t, file descriptor:"<<i->second.first<<"\t, mode:"<<i->second.second<<endl;
    }
}