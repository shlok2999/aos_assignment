////////////////////////////// Header Files ///////////////////////////////////////////////////

#include<bits/stdc++.h>
#include<stdio.h>
#include<dirent.h>
#include<unistd.h>
#include<iostream>
#include<cstring>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<termios.h>
#include<fcntl.h>
using namespace std;

//////////////////////////////// Global Variabble ////////////////////////////////////////////// 

char root[FILENAME_MAX];
char current_directory[FILENAME_MAX];
vector<string> files;
stack<string> previous;
stack<string> next;
int absolutePath;

///////////////////////////// Function Declaration ////////////////////////////////////////////

void open_directory(char *path);
void display();
struct stat get_meta(char *file);
string getfilename(string path);
void copyfile(string path, string des);
void newdir(string file,struct stat meta);
void copy_directory(string path,string des);
void delete_file(string file);
void delete_dir(string file);
void rename_file(string path, string des);
void move_file(string path, string des);
void goto_path(string path);
bool search(string file,string path);
void create_file(string file);
void create_dir(string file);

///////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////// Main Function //////////////////////////////////////////
int main()
{
    if(!getcwd(current_directory,sizeof(current_directory)))
    {
        cout<<"could not find path";
        exit(1);
    }
    current_directory[sizeof(current_directory)-1]='\0';
    strcpy(root,current_directory);
    string dest="/home/shlok";
    string path="images";
    string new_directory="/home/shlok/Demos/Linux-Terminal-based-File-Explorer-master/images/Hello";

    string filename="/home/shlok/abc1";
    create_dir(filename);
    //create_file(filename);
    //string cwd(current_directory);
    //cout<<search(filename,cwd)<<endl;
    //goto_path(dest);
    //move_file(path,dest);
    //delete_dir(path);
    //rename_file(path,dest);
    //delete_file(path);
    //copy_directory(path,dest);
    //newdir(new_directory);
    //cout<<path<<endl<<dest;
    //copyfile(path,dest);
    //cout<<current_directory<<" ";
    //cout<<root<<endl;
    //open_directory(current_directory);

}

///////////////////////////// Open Directory /////////////////////////////////////////////////

void open_directory(char *path)
{
    DIR *directory;
    if((directory= opendir(path))== NULL)
    {
        cout<<"Could notopen directory";
        return;
    }

    printf("\033[H\033[J");
    files.clear();
    struct dirent *dir;

    while((dir=readdir(directory))!= NULL)
        files.push_back(string(dir->d_name));
    
    closedir(directory);

    int total_lines=files.size();

    display(); // displays all the directory in the cuurent directory

}

////////////////////////////////// Displays the function ///////////////////////////////////////

void display()
{
    for(int i=0;i<files.size();i++)
    {
        char temp[files[i].length()+1];
        strcpy(temp,files[i].c_str());
        temp[files[i].length()]='\0';
       
        absolutePath=0;
        struct stat meta=get_meta(temp);

        //if directory
        if(S_ISDIR(meta.st_mode))
            printf("d");
        else
            printf("-");
        //read mode for user?
        if(meta.st_mode & S_IRUSR)
            printf("r");
        else
            printf("-");
        //write mode for user?
        if(meta.st_mode & S_IWUSR)
            printf("w");
        else
            printf("-");
        //executable mode for user?
        if(meta.st_mode & S_IXUSR)
            printf("x");
        else
            printf("-");
        //read mode for grp?
        if(meta.st_mode & S_IRGRP)
            printf("r");
        else
            printf("-");
        //write mode for grp?
        if(meta.st_mode & S_IWGRP)
            printf("w");
        else
            printf("-");
        //executable mode for grp?
        if(meta.st_mode & S_IXGRP)
            printf("x");
        else
            printf("-");
        //read mode for other?
        if(meta.st_mode & S_IROTH)
            printf("r");
        else
            printf("-");
        //write mode for other?
        if(meta.st_mode & S_IWOTH)
            printf("w");
        else
            printf("-");
        //executable mode for other?
        if(meta.st_mode & S_IXOTH)
            printf("x");
        else
            printf("-");
        
        //Getting user name and group name

        struct passwd *user=getpwuid(meta.st_uid);
        if(user)
            printf("\t%-9s",user->pw_name);
        struct group *grp=getgrgid(meta.st_gid);
        if(grp)
            printf("%-9s",grp->gr_name);
        
        //Getting the modified time
        char *mod_time=ctime(&meta.st_mtime);
        mod_time[strlen(mod_time)-1]='\0';
        printf("%-10s",mod_time);

        //Display File name
        printf("\t%-10s\n",temp);
        //printf("a");
    }

    //cin.get();
}

/////////////////////////////////// Gets all the meta data about the file ///////////////////////////

struct stat get_meta(char *file)
{
    struct stat meta;
    char cwd[FILENAME_MAX];
    char path[FILENAME_MAX];
    if(absolutePath==0)
    {
        strcpy(cwd,current_directory);
        strcat(cwd,"/");
        strcat(cwd,file);
        strcpy(path,cwd);
    }
    else
        strcpy(path,file);
    //cout<<cwd<<endl;
    //cout<<path;
    if(stat(path,&meta)==-1)
    {
        perror("Error in stat");
    }

    return meta;
};

/////////////////////////// Teriminal Window Cursor Handling////////////////////////////////////



///////////////////////////////// Get file name ////////////////////////////////////////////////

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


/////////////////////////// Copy Files ////////////////////////////////////////////////////////

void copyfile(string path, string des)
{
    absolutePath=1;
    
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    
    
    
    if(des[0]!='/')
    {
        if(des[0]=='.')
            des=des.substr(2);
        //cwd=cwd+"/";
        des=cwd+"/"+des;
    }
    
    //cout<<path<<endl<<des<<endl;
    //cout<<path;
    
    
    string temp2=getfilename(path);
    temp2="/"+temp2;
    //cout<<temp2<<endl;
    //char file[FILENAME_MAX];
    //strcpy(file,temp2.c_str());
    des=des+temp2;
    //cout<<des;

    char source[FILENAME_MAX];
    char destination[FILENAME_MAX];

    strcpy(source,path.c_str());
    source[path.length()]='\0';
    
    strcpy(destination,des.c_str());
    destination[des.length()]='\0';
    
    struct stat s=get_meta(source);
    int src=open(source, O_RDONLY);
    int dest=open(destination, O_WRONLY | O_CREAT,s.st_mode);
    if(dest<1)
    {    
        close(src);
        cout<<"Unable to create";
        return;
    }
    char buf[s.st_size];
    while(read(src,buf,sizeof(buf))!=0)
    {
        if(write(dest,buf,sizeof(buf))<0)
        {
                cout<<"Writing Error";
        }
    }
    
    fchown(dest,s.st_uid,s.st_gid);
     
    close(src);
    close(dest);
}

////////////////////////////////// Creating a Directory ////////////////////////////////////////////
void newdir(string file,struct stat meta)
{
    char path[FILENAME_MAX];
    strcpy(path,file.c_str());
    path[file.length()]='\0';
    if(mkdir(path,meta.st_mode)==-1)
    {
        cout<<"Unable to create file";
    }
}

//////////////////////////////// Copy a directory //////////////////////////////////////////////////
void copy_directory(string path,string des)
{
    absolutePath=1;
    
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    
    
    
    if(des[0]!='/')
    {
        if(des[0]=='.')
            des=des.substr(2);
        //cwd=cwd+"/";
        des=cwd+"/"+des;
    }
    string temp2=getfilename(path);
    temp2="/"+temp2;
    //cout<<temp2<<endl;
    //char file[FILENAME_MAX];
    //strcpy(file,temp2.c_str());
    des=des+temp2;

    char source[FILENAME_MAX];
    char destination[FILENAME_MAX];

    strcpy(source,path.c_str());
    source[path.length()]='\0';
    
    strcpy(destination,des.c_str());
    destination[des.length()]='\0';
    
    

    DIR *directory;
    if((directory=opendir(source))==NULL) // Opening source directory
    {
        cout<<"Could not access file";
        return;
    }
    dirent *dir;
    vector<string> temp_files;
    while((dir=readdir(directory))!=NULL) //Reading Files name;
    {
        temp_files.push_back(string(dir->d_name));
    }
    closedir(directory);

    struct stat s=get_meta(source);
    newdir(destination,s); // Creating the folder

    for(int i=0;i<temp_files.size();i++)
    {
        if(temp_files[i]=="." || temp_files[i]=="..")
            continue;
        char temp[FILENAME_MAX];
        temp_files[i]= path + "/" + temp_files[i];
        strcpy(temp,temp_files[i].c_str());
        temp[temp_files[i].length()]='\0';
        struct stat ts=get_meta(temp);
        if(S_ISDIR(ts.st_mode))
        {
            copy_directory(temp_files[i],des);
        }
        else
        {
            copyfile(temp_files[i],des);
        }
    }

}

///////////////////////////////////// Delete File ///////////////////////////////////////////////////

void delete_file(string file)
{
    string cwd(current_directory);
    if(file[0]!='/')
    {
        if(file[0]=='.')
            file=file.substr(2);
        file=cwd+"/"+file;
    }
    char path[FILENAME_MAX];
    strcpy(path,file.c_str());
    path[file.length()]='\0';

    struct stat meta;
    if(stat(path,&meta) == -1)
    {
        return;
    }
    unlink(path);
}

//////////////////////////////// Delete Directory ///////////////////////////////////////////////////

void delete_dir(string file)
{
    string cwd(current_directory);
    if(file[0]!='/')
    {
        if(file[0]=='.')
            file=file.substr(2);
        file=cwd+"/"+file;
    }
    char path[FILENAME_MAX];
    strcpy(path,file.c_str());
    path[file.length()]='\0';
    //cout<<path;
    struct stat meta;
    if(stat(path,&meta) == -1)
    {
        return;
    }

    DIR *directory;
    if((directory=opendir(path))==NULL) // Opening source directory
    {
        cout<<"Could not access file";
        return;
    }
    dirent *dir;
    vector<string> temp_files;
    while((dir=readdir(directory))!=NULL) //Reading Files name;
    {
        temp_files.push_back(string(dir->d_name));
    }
    closedir(directory);

    for(int i=0;i<temp_files.size();i++)
    {
        absolutePath=1;
        
        if(temp_files[i]=="." || temp_files[i]=="..")
            continue;
        char temp[FILENAME_MAX];
        temp_files[i]= file + "/" + temp_files[i];
        //cout<<temp_files[i]<<endl;
        strcpy(temp,temp_files[i].c_str());
        temp[temp_files[i].length()]='\0';
        //cout<<temp;
        struct stat ts=get_meta(temp);
        if(S_ISDIR(ts.st_mode))
        {
            //cout<<"hello";
            delete_dir(temp_files[i]);
        }
        else
        {
            delete_file(temp_files[i]);
        }
    }

    rmdir(path);
}

///////////////////////////////// Renaming thing /////////////////////////////////////////////////

void rename_file(string path,string des)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    
    
    
    if(des[0]!='/')
    {
        if(des[0]=='.')
            des=des.substr(2);
        //cwd=cwd+"/";
        des=cwd+"/"+des;
    }

    char source[FILENAME_MAX];
    char destination[FILENAME_MAX];

    strcpy(source,path.c_str());
    source[path.length()]='\0';
    
    strcpy(destination,des.c_str());
    destination[des.length()]='\0';
    cout<<source<<endl;
    cout<<destination;
    rename(source,destination);
}

////////////////////////// Moving files /////////////////////////////////////////////////////////

void move_file(string path, string des)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    
    
    
    if(des[0]!='/')
    {
        if(des[0]=='.')
            des=des.substr(2);
        //cwd=cwd+"/";
        des=cwd+"/"+des;
    }
    string temp2=getfilename(path);
    temp2="/"+temp2;
    //cout<<temp2<<endl;
    //char file[FILENAME_MAX];
    //strcpy(file,temp2.c_str());
    des=des+temp2;

    char source[FILENAME_MAX];
    char destination[FILENAME_MAX];

    strcpy(source,path.c_str());
    source[path.length()]='\0';
    
    strcpy(destination,des.c_str());
    destination[des.length()]='\0';

    cout<<source<<endl;
    cout<<destination;


    rename(source,destination);

}

///////////////////////////////////////// Goto  /////////////////////////////////////////////////////


void goto_path(string path)
{
    string cwd(current_directory);
    previous.push(cwd);

    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;     
    }

    strcpy(current_directory,path.c_str());
    current_directory[path.length()]='\0';
    //cout<<current_directory;

}

///////////////////////////// Search Function /////////////////////////////////////////////////////

bool search(string file,string path)
{
    
    char p[FILENAME_MAX];
    strcpy(p,path.c_str());
    p[path.length()]='\0';

    DIR *directory;
    if((directory=opendir(p))==NULL) // Opening source directory
    {
        cout<<"Could not access file";
        return false;
    }
    dirent *dir;
    vector<string> temp_files;
    while((dir=readdir(directory))!=NULL) //Reading Files name;
    {
        temp_files.push_back(string(dir->d_name));
    }
    closedir(directory);


    for(int i=0;i<temp_files.size();i++)
    {
        absolutePath=1;
        if(temp_files[i]=="." || temp_files[i]=="..")
            continue;
        if(temp_files[i]==file)
            return true;
        
        char temp[FILENAME_MAX];
        temp_files[i]= path + "/" + temp_files[i];
        //cout<<temp_files[i]<<endl;
        strcpy(temp,temp_files[i].c_str());
        temp[temp_files[i].length()]='\0';
        //cout<<temp;
        struct stat ts=get_meta(temp);
        if(S_ISDIR(ts.st_mode))
        {
            //cout<<"hello";
            bool ans=search(file,temp_files[i]);
            if(ans)
                return true;
        }
        
    }
    return false;

}

////////////////////////////////// Create File //////////////////////////////////////////////////////

void create_file(string path)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    char file[FILENAME_MAX];
    strcpy(file,path.c_str());
    file[path.length()]='\0';
    if(creat(file,S_IRUSR| S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH )==-1)
    {
        perror("File Could not be created");
        return;
    }
}
/////////////////////////// Creating Directory ///////////////////////////////////////////////////////
void create_dir(string path)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    char file[FILENAME_MAX];
    strcpy(file,path.c_str());
    file[path.length()]='\0';

    if (mkdir(file,S_IRUSR| S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH ) == -1)
    {
        perror("Error Creating Directory");
        return;
    }
}