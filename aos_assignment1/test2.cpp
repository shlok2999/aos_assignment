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
#include <sys/ioctl.h>
#include<string.h>

using namespace std;

//////////////////////////////// Global Variabble ////////////////////////////////////////////// 
string sys_root="/home/";
char esc='\x1b';
char root[FILENAME_MAX];
char current_directory[FILENAME_MAX];
vector<string> files;
stack<string> previous;
stack<string> next_f;
int absolutePath;
struct termios original_setting,new_setting;
long long int start , starty;
long long int rows , cols;
long long int x , y;
int mode ;
#define EXECL_PATH "/usr/bin/xdg-open"
#define EXECL_NAME "xdg-open"

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
void create_file(string filename,string file);
void create_dir(string filename,string file);
void screen();
string showfile(char *path);
void getWindowSize();
void DisableScreenMode();
void pos_cursor(int x);
void pos_cursor(int x,int y);
void clear();
void displayn(int n);
string goback();
void commandmode();
void refresh();
void command_processing(string command);
void display_linen(string line,int n);
void display_line(string line);
static void resizing(int sig);

///////////////////////////////////////////////////////////////////////////////////////////////
static void resizing(int sig) //For handling when window is resized
{
    if(sig==SIGWINCH)
    {
        //Do something
        clear();
        open_directory(current_directory);
        if(mode==0)
            pos_cursor(x);
        else
            refresh();
    }
}


////////////////////////////////////// Main Function //////////////////////////////////////////
int main()
{
    signal(SIGWINCH, resizing); //For getting signal when window is resized
    if(!getcwd(current_directory,sizeof(current_directory)))
    {
        cout<<"could not find path";
        exit(1);
    }
    current_directory[sizeof(current_directory)-1]='\0';
    string username=string(getlogin());
    sys_root=sys_root+username;
    strcpy(root,sys_root.c_str());
    root[sys_root.length()]='\0';
    
    open_directory(current_directory);
    screen(); //Going to normal mode
    clear();
    

}
/////////////////////////////// Clear the screen ////////////////////////////////////////////////

void clear()
{
    cout<<"\x1b[2J";
    cout<<"\x1b[1;1H";
}

///////////////////////////// Open Directory /////////////////////////////////////////////////

void open_directory(char *path)
{
    x=1;
    y=1;
    DIR *directory;
    if((directory= opendir(path))== NULL)
    {
        cout<<"Could notopen directory";
        return;
    }

    
    files.clear();
    struct dirent *dir;

    while((dir=readdir(directory))!= NULL)
    {

        files.push_back(string(dir->d_name));
        //cout<<string(dir->d_name)<<endl;
    }
    
    closedir(directory);


    

    getWindowSize(); //Getting details of terminal size
    
    start=0;
    starty=0;
    display(); // displays all the directory in the cuurent directory

}
////////////////////////////////// Displays the function ///////////////////////////////////////

void display()
{
    clear();
    //cout<<start<<endl;
    for(int i=0;i<files.size()&& i<rows;i++)
    {
        char temp[files[i].length()+1];
        strcpy(temp,files[i].c_str());
        temp[files[i].length()]='\0';
        //cout<<temp<<endl;
        
        string line=showfile(temp);
        display_line(line);
        cout<<"\n";  
        //cout<<files[i]<<endl;
    }

    pos_cursor(rows+1);
    cout<<"----NORMAL MODE";

    
}


string showfile(char *path) //It is used to get information which needs to be displayed AND return it in form of string
{
        absolutePath=0;
        struct stat meta=get_meta(path);
        string line="";
        //if directory
        if(S_ISDIR(meta.st_mode))
            line+="d";
        else
            line+="-";
        //read mode for user?
        if(meta.st_mode & S_IRUSR)
            line+="r";
        else
            line+="-";
        //write mode for user?
        if(meta.st_mode & S_IWUSR)
            line+="w";
        else
            line+="-";
        //executable mode for user?
        if(meta.st_mode & S_IXUSR)
            line+="x";
        else
            line+="-";
        //read mode for grp?
        if(meta.st_mode & S_IRGRP)
            line+="r";
        else
            line+="-";
        //write mode for grp?
        if(meta.st_mode & S_IWGRP)
            line+="w";
        else
            line+="-";
        //executable mode for grp?
        if(meta.st_mode & S_IXGRP)
            line+="x";
        else
            line+="-";
        //read mode for other?
        if(meta.st_mode & S_IROTH)
            line+="r";
        else
            line+="-";
        //write mode for other?
        if(meta.st_mode & S_IWOTH)
            line+="w";
        else
            line+="-";
        //executable mode for other?
        if(meta.st_mode & S_IXOTH)
            line+="x";
        else
            line+="-";
        
        //Getting user name and group name
        line+="  ";
        struct passwd *user=getpwuid(meta.st_uid);
        if(user)
            line+=user->pw_name;
        line+="  ";
        struct group *grp=getgrgid(meta.st_gid);
        if(grp)
        {    line+=grp->gr_name;
             line+="  ";
        }
        
        //Getting the modified time
        char *mod_time=ctime(&meta.st_mtime);
        mod_time[strlen(mod_time)-1]='\0';
        string time(mod_time);
        line+=time;
        line+="  ";
        //printf("%10.2fK", ((double)meta.st_size) / 1024);
        float temp=((float)meta.st_size) / 1024;
        string stemp=to_string(temp);
        int pos=stemp.find('.');
        stemp=stemp.substr(0,pos+3);
        line=line+stemp+"KB";
        line+="  ";
        //Display File name
        string p(path);
        line+=p;
        //printf("a");
        return line;
}

////////////////////////////////////// Display line //////////////////////////////////////////////////

void display_line(string line)
{
    int n=line.length();
    for(int i=0;i<n && i<cols ;i++)
        cout<<line[i];
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
        exit(1);
    }

    return meta;
};
/////////////////////////////////////// Get window size ////////////////////////////////////////

void getWindowSize() //For getting dimension of terminal screen
{
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    rows=window.ws_row-3;
    cols= window.ws_col;
}

/////////////////////////// Teriminal Window Cursor Handling////////////////////////////////////
string goback() // this is udes to go back 1 level up in heirarchy of directory
{
    string cwd(current_directory);
    reverse(cwd.begin(),cwd.end());
    for(int i=0;i<cwd.length();i++)
    {
        if(cwd[i]=='/')
        {
            cwd=cwd.substr(i+1);
            break;
        }
    }

    reverse(cwd.begin(),cwd.end());
    return cwd;
}

void displayn(int n) //This funnction is used to print normal mode and the file should remain within the row boundary
{
    clear();
    for(int i=start;i<n;i++)
    {
        char temp[files[i].length()+1];
        strcpy(temp,files[i].c_str());
        temp[files[i].length()]='\0';
        //cout<<temp<<endl;
        
        string line=showfile(temp);
        display_line(line);  
        cout<<"\n";
    }

    pos_cursor(rows+1);
    cout<<"----NORMAL MODE";

}

void display_linen(string line,int n) //This is used to diplay a line in normal mode. used while horizontal scrolling
{
    pos_cursor(x);
    int i;
    for(i=starty;i<n;i++)
    {
        cout<<line[i];
    }
}

void pos_cursor(int x) //Used to postion of cursor at starting of line.
{
    cout<<"\x1b["<<x<<";1H";
}

void pos_cursor(int x,int y) //Used to postion cusror anywhere on screeen
{
    cout<<"\x1b["<<x<<";"<<y<<"H";
}

void DisableScreenMode() //Used to set termios in its default maode
{
      tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_setting);
}

void screen() //Normal Mode
{
    mode=0;

    pos_cursor(x);
    tcgetattr(STDIN_FILENO, &original_setting);
    atexit(DisableScreenMode); // Making sure that even if program crashes termios is set back to default value
    new_setting=original_setting;
    new_setting.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO,TCSAFLUSH, &new_setting);
    
    while(1)
    {
        char ch=cin.get();
        if(ch=='h') //Home Key
        {
            string cwd(current_directory);
            previous.push(cwd);
            strcpy(current_directory,root);
            open_directory(current_directory);
            pos_cursor(x);
        }
        else if(ch == ':') //Entering Command Mode
        {
            mode=1;
            commandmode();
            open_directory(current_directory);
            pos_cursor(x);
            mode=0;
        }
        else if(ch=='q') //To quit the program
        {
            break;
        }
        else if(ch =='a')// For scrolling line left side
        {
            char p[FILENAME_MAX];
            strcpy(p,files[start+x-1].c_str());
            p[files[start+x-1].length()]='\0';
            string line=showfile(p);
            int size=line.length();
            if(y+starty>1)
            {
                if(y>1)
                {
                    y--;
                    pos_cursor(x,y);
                }

                else if(y==1)
                {
                    if(starty>0)
                    {
                        starty--;
                    }

                    int n;

                    if(cols >= size)
                    {
                        n=size;
                    }
                    else
                    {
                        n=starty+cols;
                    }

                    display_linen(line,n);
                    pos_cursor(x,y);
                }
            }
        }
        else if(ch=='d')//For scrolling line right side
        {
            char p[FILENAME_MAX];
            strcpy(p,files[start+x-1].c_str());
            p[files[start+x-1].length()]='\0';
            string line=showfile(p);
            int size=line.length();
            if(y + starty < size )
            {
                    if(y<cols)
                    {
                        y++;
                        pos_cursor(x,y);
                    }
                    else if(y==cols)
                    {
                        if(cols < size)
                        {
                            starty++;
                            
                        }
                        int n;
                        if(cols >= size)
                        {
                            n=size;
                        }
                        else
                        {
                            n=starty+cols;
                        }
                        display_linen(line,n);
                        pos_cursor(x,y);
                    }
                }   
        }
        else if(ch=='l')
        {
            y=1;
                starty=0;
                pos_cursor(x);
                char p[FILENAME_MAX];
                strcpy(p,files[start+x-1].c_str());
                p[files[start+x-1].length()]='\0';
                string line=showfile(p);
                display_line(line);
                pos_cursor(x);
                if(x + start < files.size() )
                {
                    
                    
                    if(x<=rows)
                    {
                        if(rows < files.size())
                        {
                            start++;
                            x=1;
                        }
                        int n;
                        if(rows >= files.size())
                        {
                            n=start+files.size();
                        }
                        else
                        {
                            n=start+rows;
                        }
                        displayn(n);
                        pos_cursor(x);
                    }
                }
        }
        else if(ch=='k')
        {
             y=1;
                starty=0;
                pos_cursor(x);
                char p[FILENAME_MAX];
                strcpy(p,files[start+x-1].c_str());
                p[files[start+x-1].length()]='\0';
                string line=showfile(p);
                display_line(line);
                pos_cursor(x);
                if(x+start>1)
                {
                   
                    
                    

                    if(x==1)
                    {
                        if(start>0)
                        {
                            start--;
                            x=1;
                        }

                        int n;

                        if(rows >= files.size())
                        {
                            n=start+files.size();
                        }
                        else
                        {
                            n=start+rows;
                        }

                        displayn(n);
                        pos_cursor(x);
                    }
                }   
        }
        else if(ch==127) //If backspace is pressed
        {
            string current=goback();
            string cwd(current_directory);
            previous.push(cwd);
            strcpy(current_directory,current.c_str());
            current_directory[current.length()]='\0';
            open_directory(current_directory);
            pos_cursor(1);

        }
        else if(ch== esc) //To see if arrow direction
        {
            ch=cin.get();
            char dir=cin.get();
            if(dir=='A') //Up arrow key is pressed
            {
                //cout<<"Hello";
                 y=1;
                starty=0;
                pos_cursor(x);
                char p[FILENAME_MAX];
                strcpy(p,files[start+x-1].c_str());
                p[files[start+x-1].length()]='\0';
                string line=showfile(p);
                display_line(line);
                pos_cursor(x);
                if(x+start>1)
                {
                   
                    
                    if(x>1)
                    {
                        x--;
                        pos_cursor(x);
                    }

                    else if(x==1)
                    {
                        if(start>0)
                        {
                            start--;
                        }

                        int n;

                        if(rows >= files.size())
                        {
                            n=files.size();
                        }
                        else
                        {
                            n=start+rows;
                        }

                        displayn(n);
                        pos_cursor(x);
                    }
                }
                


            }

            if(dir== 'B') //Down arrow key is pressed
            {
                y=1;
                starty=0;
                pos_cursor(x);
                char p[FILENAME_MAX];
                strcpy(p,files[start+x-1].c_str());
                p[files[start+x-1].length()]='\0';
                string line=showfile(p);
                display_line(line);
                pos_cursor(x);
                if(x + start < files.size() )
                {
                    
                    if(x<rows)
                    {
                        x++;
                        pos_cursor(x);
                    }
                    else if(x==rows)
                    {
                        if(rows < files.size())
                        {
                            start++;
                            
                        }
                        int n;
                        if(rows >= files.size())
                        {
                            n=files.size();
                        }
                        else
                        {
                            n=start+rows;
                        }
                        displayn(n);
                        pos_cursor(x);
                    }
                }
            }

            if(dir=='C') //Right arrow key is pressed
            {
                if(! next_f.empty())
                {
                    string cwd(current_directory);
                    previous.push(cwd);
                    string current=next_f.top();
                    next_f.pop();
                    strcpy(current_directory,current.c_str());
                    current_directory[current.length()]='\0';
                    open_directory(current_directory); 
                    pos_cursor(1);  
                }
            }

            if(dir=='D') //Left arrow key is pressed
            {
                if(!previous.empty())
                {
                    string cwd(current_directory);
                    next_f.push(cwd);
                    string current=previous.top();
                    previous.pop();
                    strcpy(current_directory,current.c_str());
                    current_directory[current.length()]='\0';
                    open_directory(current_directory);
                    pos_cursor(1);
                }   
            }
        }
        //Enter is pressed
        else if(ch==10)
        {
            string current=files[start+x-1];
            if(current=="..")
            {
                current=goback();
                string cwd(current_directory);
                previous.push(cwd);
                strcpy(current_directory,current.c_str());
                current_directory[current.length()]='\0';
                open_directory(current_directory);
                pos_cursor(1);
            }
            else if(current!=".")
            {
                string cwd(current_directory);
                current=cwd+"/"+current;
                absolutePath=1;
                char temp[FILENAME_MAX];
                strcpy(temp,current.c_str());
                struct stat meta=get_meta(temp);
                if(S_ISDIR(meta.st_mode))
                {
                    previous.push(cwd);
                    strcpy(current_directory,current.c_str());
                    current_directory[current.length()]='\0';
                    open_directory(current_directory);
                    pos_cursor(1);
                }
                else
                {
                    pid_t pid=fork(); //Opening a new process so that file explorer doesn't stops
                    if(pid==0)
                    {
                        execl(EXECL_PATH,EXECL_NAME,current.c_str(),(char *) 0);
                    }
                }
            }
        }
    }
    

}
//////////////////////////////////// Command Mode //////////////////////////////////////////////////////////

void refresh()
{
    int line=rows+1;
    pos_cursor(line);
    cout<<"\x1b[0J";
    cout<<"---- Command Mode\n";

}

void commandmode() //command Mode
{
    refresh();
    while(1)
    {
        DisableScreenMode();
        string command;
        getline(cin,command);
        refresh();
        if(command=="q") //Quits the main program
        {
            exit(0);
        }
        else if(command== "\x1b")
        {
            tcsetattr(STDIN_FILENO,TCSAFLUSH, &new_setting);
            return;
        }
        else
        {
            
            //cout<<"Enter command :";
            //cout<<ch;
            //string command;
            
            command_processing(command);
            
        }
    }
}


////////////////////////////////// Command Processing /////////////////////////////////////////

void command_processing(string command)//Strings are tokenized and the the commans is called accordingly
{
    int flag=0;
    int count=0;
    int index=0;
    string token;
    vector<string> tokens;

    for(int i=0;i<command.length();i++)
    {
        if(command[i]=='\'' && flag==0)
        {
            flag=1;
            count=0;
            index=i+1;
        }
        else if(command[i]=='\'' && flag==1)
        {
            flag=0;
            count=0;
            token=command.substr(index,count);
            tokens.push_back(token);
        }
        else if(command[i]==' ' && flag==1)
        {
            count++;
        }
        else if(command[i]==' ' && flag==0)
        {
            tokens.push_back(token);
            token="";
        }
        else if(flag==1)
        {
            count++;
        }
        else
        {
            token+=command[i];
        }
    }

    if(command[command.length()-1]!='\'')
    {
        tokens.push_back(token);
    }

    /*
    for(int i=0;i<tokens.size();i++)
    {
        cout<<tokens[i]<<" ";
    }
    */

   for(int i=0;i<tokens.size();i++)
    {
        if(tokens[i][0]=='~')
        {
            tokens[i]=sys_root+tokens[i].substr(1);
        }
        else if(tokens[i]=="..")
            tokens[i]=goback();
    }

   if(tokens[0]=="copy")
   {
       if(tokens.size()>=3)
       {
           string destination=tokens[tokens.size()-1];
           //cout<<destination;
           for(int i=1;i<tokens.size()-1;i++)
            {
                if(tokens[i]==".")
                {
                    absolutePath=1;
                    string cwd(current_directory);
                    tokens[i]=cwd;
                }
                else if(tokens[i][0]!='/')
                {
                    if(tokens[i][0]=='.')
                        tokens[i]=tokens[i].substr(2);
                    absolutePath=0;
                }
                else
                    absolutePath=1;
                
                char path[FILENAME_MAX];
                strcpy(path,tokens[i].c_str());
                path[tokens[i].length()]='\0';
                struct stat meta=get_meta(path);
                if(S_ISDIR(meta.st_mode))
                    copy_directory(tokens[i],destination);
                else
                    copyfile(tokens[i],destination);
            }
       }
       else
       {
           perror("Wrong no.of argument");
           exit(1);
       }
   }
   else if(tokens[0]=="move")
   {
       if(tokens.size()>=3)
       {
           string destination=tokens[tokens.size()-1];
           for(int i=1;i<tokens.size()-1;i++)
            {
                if(tokens[i]==".")
                {
                    absolutePath=1;
                    string cwd(current_directory);
                    tokens[i]=cwd;
                }
                move_file(tokens[i],destination);
            }
       }
       else
       {
           perror("Wrong no.of argument");
           exit(1);
       }
   }
   else if(tokens[0]=="rename")
   {
       if(tokens.size()!=3)
       {
           cout<<"Error";
           return;
       }
       rename_file(tokens[1],tokens[2]);
   }
   else if(tokens[0]=="create_file")
   {
       if(tokens.size()!=3)
       {
           cout<<"Error";
           return;
       }
       create_file(tokens[1],tokens[2]);
   }
   else if(tokens[0]=="create_dir")
   {
       if(tokens.size()!=3)
       {
           cout<<"Error";
           return;
       }
       create_dir(tokens[1],tokens[2]);
   }
   else if(tokens[0]=="goto")
   {
       if(tokens.size()!=2)
       {
           cout<<"Error";
           return;
       }
       goto_path(tokens[1]);
   }
   else if(tokens[0]=="search")
   {
       if(tokens.size()!=2)
       {
           cout<<"Error";
           return;
       }
       string cwd(current_directory);
       bool ans=search(tokens[1],cwd);
       if(ans)
            cout<<"true";
       else
            cout<<"false";
   }
   else if(tokens[0]=="delete_dir")
   {
       if(tokens.size()!=2)
       {
           cout<<"Error";
           return;
       }
       delete_dir(tokens[1]);
   }
   else if(tokens[0]=="delete_file")
   {
       if(tokens.size()!=2)
       {
           cout<<"Error";
           return;
       }
       delete_file(tokens[1]);
   }
   else
   {
       cout<<"Error";
       return;
   }

}

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
        exit(1);
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

    //cout<<source<<endl;
    //cout<<destination;


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

void create_file(string filename,string path)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    path=path+"/"+filename;
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
void create_dir(string filename,string path)
{
    string cwd(current_directory);
    
    
    if(path[0]!='/')
    {
        if(path[0]=='.')
            path=path.substr(2);
        //cwd=cwd+"/";
        path=cwd+"/"+path;  
        
    }
    path=path+"/"+filename;
    char file[FILENAME_MAX];
    strcpy(file,path.c_str());
    file[path.length()]='\0';

    if (mkdir(file,S_IRUSR| S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH ) == -1)
    {
        perror("Error Creating Directory");
        return;
    }
}