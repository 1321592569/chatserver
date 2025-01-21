# ifndef GROUPUSER_H
# define GROUPUSER_H

#include "user.hpp"

//群组用户，只是多了一个role属性，所以该直接从User类继承即可
class GroupUser : public User
{
public:
    void setRole(int role) { this->role = role; }
    string getRole() { return this->role; }
private:
    string role;
};
# endif //GROUPUSER_H