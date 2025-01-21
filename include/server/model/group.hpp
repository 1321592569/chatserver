#ifndef GROUP_H
#define GROUP_H

# include "groupuser.hpp"
# include <string>
# include <vector>
using namespace std;

// AllGroup表的ORM（对象关系映射）类
/*
在面向对象编程中，ORM（对象关系映射）是一种技术，
它允许开发者将数据库中的表映射为对象，从而简化数据库操作。
ORM技术可以将数据库中的表映射为类，将表中的字段映射为类的属性
从而使得开发者可以使用面向对象的方式来操作数据库。
*/
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    :id(id), name(name), desc(desc) {}
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    vector<GroupUser>& getUsers() { return this->users; }

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};
#endif // GROUP_H