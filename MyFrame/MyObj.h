#ifndef __MYOBJ_H__
#define __MYOBJ_H__
#include <string>
#include <unordered_set>

#include "MyCommon.h"

class MyObj
{
public:

    explicit MyObj();

    virtual ~MyObj();

    bool Inherits(const char* classname);

protected:

    void SetInherits(const char *classname);

private:

    std::unordered_set<std::string> m_inherits;
};

#endif // MYOBJ_H


