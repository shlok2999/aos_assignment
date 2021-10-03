#include<bits/stdc++.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
using namespace std;


///////////////////////////// Defining Tree ///////////////////////////////////////// 

class node
{
    public:
    string name;
    vector<node *> children;
    node* parent;
    node(string n);
    void message();
};
/////////////////////////////////////////////////////////////////////////////////


node *root;
