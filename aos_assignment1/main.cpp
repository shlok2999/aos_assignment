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
using namespace std;

//////////////////////////////// Global Variabble ////////////////////////////////////////////// 

char root[FILENAME_MAX];
char current_directory[FILENAME_MAX];
vector<string> files;

///////////////////////////// Function Declaration ////////////////////////////////////////////

void open_directory(char *path);
void display();
struct stat get_meta(char *file);
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
    //cout<<current_directory<<" ";
    //cout<<root<<endl;
    open_directory(current_directory);
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

/////////////////////////////////// Gets all the meta data about the ///////////////////////////

struct stat get_meta(char *file)
{
    struct stat meta;
    char cwd[FILENAME_MAX];
    
    strcpy(cwd,current_directory);
    strcat(cwd,"/");
    strcat(cwd,file);
    //cout<<cwd<<endl;
    
    if(stat(cwd,&meta)==-1)
    {
        perror("Error in stat");
    }

    return meta;
};
