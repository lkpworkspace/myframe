#include "MyObj.h"

MyObj::MyObj() :
    m_obj_name("")
{}

MyObj::~MyObj()
{
}

bool MyObj::Inherits(const char* classname)
{
    return m_inherits.find(classname) != m_inherits.end();
}

void MyObj::SetInherits(const char* classname)
{
    m_inherits.insert(classname);
    m_obj_name = classname;
}
