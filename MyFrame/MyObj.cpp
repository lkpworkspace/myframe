#include "MyObj.h"

MyObj::MyObj()
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
}
