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
stack<string> pevious;
stack<string> next;
int absolutePath;

///////////////////////////// Function Declaration ////////////////////////////////////////////

void open_directory(char *path);
void display();
struct stat get_meta(char *file);
string getfilename(string path);
void copyfile(char *path, char *des);
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
    char dest[]="/home/shlok/Demos/Linux-Terminal-based-File-Explorer-master/images";
    char path[]="/home/shlok/Demos/Linux-Terminal-based-File-Explorer-master/abc.txt";
    //cout<<path<<endl<<dest;
    copyfile(path,dest);
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

void copyfile(char *path, char *des)
{
    absolutePath=1;
    //cout<<des;
    char cwd[FILENAME_MAX];
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path++;
        strcpy(cwd,current_directory);
        strcat(cwd,"/");
        strcat(cwd,path);
        strcpy(path,cwd);      
    }
    if(des[0]!='/')
    {
        strcpy(cwd,current_directory);
        strcat(cwd,"/");
        strcat(cwd,des);
        strcpy(des,cwd);      
    }
    
    string temp(path);
    string temp2=getfilename(temp);
    temp2="/"+temp2;
    //cout<<temp2<<endl;
    char file[FILENAME_MAX];
    strcpy(file,temp2.c_str());
    
    file[temp2.length()]='\0';
    strcat(des,file);
    //cout<<des<<endl;
    //cout<<path;
    struct stat s=get_meta(path);
    int src=open(path, O_RDONLY);
    int dest=open(des, O_WRONLY | O_CREAT,s.st_mode);
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