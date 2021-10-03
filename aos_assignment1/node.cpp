#include "header.hpp"

node::node(string n)
{
    parent=NULL;
    name=n;
}

void node::message()
{
    cout<<"Hello";
}