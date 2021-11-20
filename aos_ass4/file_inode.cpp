#include<bits/stdc++.h>
#include <sys/stat.h>


using namespace std;

///////////////////////////////////// Constant Global Variables //////////////////////////////
const size_t Block_size=4*1000; //4KB size of disk ball
const size_t no_of_blocks=512*1000/4;
const size_t no_of_inode=78654;
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
        // for(int &i:direct_pointer)
        //     i=-1;
        s_indirect_pointer=-1;
        d_indirect_pointer=-1;
    }
};

class file_inode
{
    public:
    char *name;
    int inode;
    file_inode()
    {
        name=new char[FILENAME_MAX];
        //memset(name,'\0',sizeof(name));
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
inode_structure *inode;
file_inode *files;
super_block super;
char *name;
unordered_map <string,int> file_to_inode;
unordered_map <int,string> inode_to_file;
queue <int> free_blocks;
queue <int> free_inodes;


/////////////////////////////////////////// Function Defination /////////////////////////////////////////////////////////////
void create_disk();
int display_menu();
void mount_disk();
void load_disk(char *disk_name);
void mount_disk_menu();
void unload_disk();


/////////////////////////////////////////// Main Function ///////////////////////////////////////////////////



int main()
{
    while (1)
    {
        int op=display_menu();
        if(op ==1)
            create_disk();
        else if(op == 2)
            mount_disk();
        else
            break;
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
    file_inode files[no_of_inode];
    inode_structure inode[no_of_inode];

    fseek(disk,0,SEEK_SET);
    fwrite(&meta,sizeof(meta),1,disk);

    // Writing files' meta in disk 
    long long int offset = (meta.no_of_sup_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fwrite(files,sizeof(file_inode),no_of_inode,disk);

    //Writing inodes of the file in the disk
    offset = (meta.no_of_sup_block + meta.no_of_bitmap_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fwrite(inode,sizeof(inode_structure),no_of_inode,disk);

    //closing file
    fclose(disk);
    cout<<"Disk Created succesfully"<<endl;
}

////////////////////////////////////////// Display Menu //////////////////////////////////////////////

int display_menu()
{
    int option=1;
    cout<<"----------------------"<<endl;
    cout<<"1. Open Disk"<<endl;
    cout<<"2. Mount Disk"<<endl;
    cout<<" Exit"<<endl;
    cout<<"Please enter interger option for exit type any num:";
    cin>>option;
    return option;
}

///////////////////////////////////////// Mount Disk //////////////////////////////////////////////////

void mount_disk()
{
    char disk_name[FILENAME_MAX];
    cout<<"Enter the disk you want to mount:"<<endl;
    cin>>disk_name;
    strcpy(name,disk_name);
    struct stat st;
    if(stat(disk_name,&st) == -1)
    {
        cout<<"No such disk exist"<<endl;
        return;
    }

    load_disk(disk_name);
    //mount_disk_menu();
    unload_disk();
}

/////////////////////////////////////////// Loading disk ///////////////////////////////////////////////

void load_disk(char *disk_name)
{
    // Reading file to main memory
    FILE *disk=fopen( disk_name, "rb");
    if(disk == NULL)
    {
        cout<<"Unable to open file"<<endl;
        exit(0);
    }

    fread(&super,sizeof(super_block),1,disk);

    files=new file_inode[no_of_inode];
    long long int offset = (super.no_of_sup_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fread(files,sizeof(file_inode),no_of_inode,disk);

    inode=new inode_structure[no_of_inode]; 
    offset = (super.no_of_sup_block + super.no_of_bitmap_block) *Block_size ;
    fseek(disk,offset,SEEK_SET);
    fread(inode,sizeof(inode_structure),no_of_inode,disk);

    fclose(disk);


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
    cout<<"Disk Unmounted Succesfully"<<endl;
}