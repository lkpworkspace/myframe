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
    void SetObjName(const std::string& obj_name) { m_obj_name = obj_name; }
    std::string& GetObjName() { return m_obj_name; }
protected:

    void SetInherits(const char *classname);

private:
    std::string                     m_obj_name;
    std::unordered_set<std::string> m_inherits;
};

#endif // MYOBJ_H


