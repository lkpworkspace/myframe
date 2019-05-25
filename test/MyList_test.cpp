#include <gtest/gtest.h>
#include "MyList.h"

TEST(MyNode, Inherits)
{
    MyNode node;
    EXPECT_EQ(true, node.Inherits("MyObj"));
}

TEST(MyList, Inherits)
{
    MyList list;
    EXPECT_EQ(true, list.Inherits("MyObj"));
}

//  1. 添加节点，添加头/尾/中间
//  2. 删除节点，删除头/尾/中间
//  3. 移动节点，移动头/尾
//  4. 遍历节点
//  5. 遍历过程中删除节点
//  6. 把一个链表添加另一个链表上

class MyNodeTest : public MyNode
{
public:
    MyNodeTest()
    {
        SetInherits("MyNode");
    }

    int v;
};

TEST(MyList, SimpleTest)
{
    MyList list;
    MyNodeTest node[10];
    MyNodeTest* b = nullptr;
    MyNodeTest* m = nullptr;
    MyNodeTest* e = nullptr;
    for(int i = 0; i < 10; ++i){
        node[i].v = i;
        list.AddTail(&node[i]);
    }
    EXPECT_EQ(true, node[0].Inherits("MyNode"));
    EXPECT_EQ(10, list.Count());
    EXPECT_EQ(false, list.IsEmpty());
    b = static_cast<MyNodeTest*>(list.GetData(0));
    m = static_cast<MyNodeTest*>(list.GetData(5));
    e = static_cast<MyNodeTest*>(list.GetData(9));
    EXPECT_EQ(0, b->v);
    EXPECT_EQ(5, m->v);
    EXPECT_EQ(9, e->v);
    list.Clear();
}

TEST(MyList, Add_Move_Del_Test)
{
    MyList list;
    MyNodeTest node1;
    node1.v = 11;
    MyNodeTest node2;
    node2.v = 22;
    MyNodeTest node3;
    node3.v = 33;
    MyNodeTest node4;
    node4.v = 44;
    /// ADD
    list.AddHead(&node1);
    list.AddHead(&node2);
    list.AddTail(&node3);
    list.AddTail(&node4);
    // {22, 11, 33, 44}
    MyNodeTest* n1 = static_cast<MyNodeTest*>(list.GetData(0));
    MyNodeTest* n2 = static_cast<MyNodeTest*>(list.GetData(1));
    MyNodeTest* n3 = static_cast<MyNodeTest*>(list.GetData(2));
    MyNodeTest* n4 = static_cast<MyNodeTest*>(list.GetData(3));
    EXPECT_EQ(22, n1->v);
    EXPECT_EQ(11, n2->v);
    EXPECT_EQ(33, n3->v);
    EXPECT_EQ(44, n4->v);
    /// MOVE
    // {33, 22, 44, 11}
    list.MoveHead(n2);
    list.MoveTail(n3);
    n1 = static_cast<MyNodeTest*>(list.GetData(0));
    n2 = static_cast<MyNodeTest*>(list.GetData(1));
    n3 = static_cast<MyNodeTest*>(list.GetData(2));
    n4 = static_cast<MyNodeTest*>(list.GetData(3));
    EXPECT_EQ(11, n1->v);
    EXPECT_EQ(22, n2->v);
    EXPECT_EQ(44, n3->v);
    EXPECT_EQ(33, n4->v);
    /// DEL
    list.Del(&node2);
    EXPECT_EQ(3, list.Count());
    n1 = static_cast<MyNodeTest*>(list.GetData(0));
    n2 = static_cast<MyNodeTest*>(list.GetData(1));
    n3 = static_cast<MyNodeTest*>(list.GetData(2));
    EXPECT_EQ(11, n1->v);
    EXPECT_EQ(44, n2->v);
    EXPECT_EQ(33, n3->v);
    /// for del
    MyNode *b, *e, *temp;
    b = list.Begin();
    e = list.End();
    for(;b != e;)
    {
        temp = b->next;

        // dosomething ...

        list.Del(b);
        b = temp;
    }
    EXPECT_EQ(0, list.Count());
    list.Clear();
}


TEST(MyList, AppendTest)
{
    MyList list1;
    MyList list2;
    MyNodeTest nodes1[10];
    MyNodeTest nodes2[10];
    for(int i = 0; i < 10; ++i){
        nodes1[i].v = i;
        list1.AddTail(&nodes1[i]);
        nodes2[i].v = i + 10;
        list2.AddTail(&nodes2[i]);
    }
    MyNodeTest* n = nullptr;
    /// APPEND
    list1.Append(&list2);
    EXPECT_EQ(true, list2.IsEmpty());
    EXPECT_EQ(20, list1.Count());
    for(int i = 0; i < list1.Count(); ++i){
        n = static_cast<MyNodeTest*>(list1.GetData(i));
        if(i < 10)
            EXPECT_EQ(i, n->v);
        else
            EXPECT_EQ(i, n->v);
    }
}
