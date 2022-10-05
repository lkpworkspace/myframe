/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "MyList.h"

namespace myframe {

// init
void MyList::__Init()
{
    m_root.prev = m_root.next = &m_root;
    m_count = 0;
}

// add
void MyList::__Add(MyNode* prev, MyNode* next, MyNode* node)
{
    if(!node) return;
    ++m_count;
    prev->next = node;
    next->prev = node;
    node->prev = prev;
    node->next = next;
}

void MyList::AddHead(MyNode* node)
{
    __Add(&m_root,m_root.next,node);
}

void MyList::AddTail(MyNode *node)
{
    __Add(m_root.prev,&m_root,node);
}

// del
void MyList::__Del(MyNode *prev, MyNode *next,bool b)
{
    --m_count;
    if(b)
        delete prev->next;
    prev->next = next;
    next->prev = prev;
}

void MyList::Del(MyNode *node,bool b)
{
    __Del(node->prev,node->next,b);
    node->prev = node->next = node;
}

void MyList::DelHead(bool b)
{
    if(IsEmpty()) return;
    Del(m_root.next,b);
}

void MyList::DelTail(bool b)
{
    if(IsEmpty()) return;
    Del(m_root.prev,b);
}

void MyList::DelWithIndex(int index,bool b)
{
    if(IsEmpty()) return;
    MyNode* temp;

    temp = GetData(index);
    if(temp)
        __Del(temp->prev,temp->next,b);
}

// move append函数
void MyList::MoveHead(MyNode* node)
{
    __Del(node->prev,node->next,false);
    AddHead(node);
}

void MyList::MoveTail(MyNode* node)
{
    __Del(node->prev,node->next,false);
    AddTail(node);
}

void MyList::Append(MyList *from)
{
    if(from->IsEmpty()) return;
    MyNode* f_head = from->m_root.next;
    MyNode* f_tail = from->m_root.prev;

    m_root.prev->next = f_head;
    f_head->prev = m_root.prev;

    f_tail->next = &m_root;
    m_root.prev = f_tail;

    m_count += from->m_count;
    // must invoke this method
    from->__Init();
}

MyNode* MyList::GetData(int index)
{
    if(IsEmpty()) return &m_root;
    if(index < 0 || index >= m_count) return nullptr;

    MyNode* temp = &m_root;
    int temp_index = -1;

    if(index >= m_count / 2)
    {// 使用倒序遍历
        temp_index = m_count;
        do{
            if(index != temp_index)
            {
                --temp_index;
                temp = temp->prev;
            }else
                return temp;
        }while(true);
    }else
    {// 使用正序遍历
        temp_index = -1;
        do{
            if(index != temp_index)
            {
                ++temp_index;
                temp = temp->next;
            }else
                return temp;
        }while(true);
    }
}

void MyList::Clear(bool b)
{
    while(!IsEmpty()){DelHead(b);}
    m_count = 0;
}

} // namespace myframe