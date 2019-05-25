#include <gtest/gtest.h>
#include "MyObj.h"

TEST(MyObj, Inherits)
{
    MyObj obj;
    obj.SetInherits("MyObj1");
    obj.SetInherits("MyObj2");
    obj.SetInherits("MyObj3");
    ASSERT_EQ(false, obj.Inherits("MyObj"));
    ASSERT_EQ(true, obj.Inherits("MyObj1"));
    ASSERT_EQ(true, obj.Inherits("MyObj2"));
    ASSERT_EQ(true, obj.Inherits("MyObj3"));
}



