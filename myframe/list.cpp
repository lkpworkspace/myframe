/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/list.h"

namespace myframe {

// init
void List::__Init() {
  root_.prev = root_.next = &root_;
  count_ = 0;
}

// add
void List::__Add(ListNode* prev, ListNode* next, ListNode* node) {
  if (!node) return;
  ++count_;
  prev->next = node;
  next->prev = node;
  node->prev = prev;
  node->next = next;
}

void List::AddHead(ListNode* node) { __Add(&root_, root_.next, node); }

void List::AddTail(ListNode* node) { __Add(root_.prev, &root_, node); }

// del
void List::__Del(ListNode* prev, ListNode* next, bool b) {
  --count_;
  if (b) delete prev->next;
  prev->next = next;
  next->prev = prev;
}

void List::Del(ListNode* node, bool b) {
  __Del(node->prev, node->next, b);
  node->prev = node->next = node;
}

void List::DelHead(bool b) {
  if (IsEmpty()) return;
  Del(root_.next, b);
}

void List::DelTail(bool b) {
  if (IsEmpty()) return;
  Del(root_.prev, b);
}

void List::DelWithIndex(int index, bool b) {
  if (IsEmpty()) return;
  ListNode* temp;

  temp = GetData(index);
  if (temp) __Del(temp->prev, temp->next, b);
}

// move append函数
void List::MoveHead(ListNode* node) {
  __Del(node->prev, node->next, false);
  AddHead(node);
}

void List::MoveTail(ListNode* node) {
  __Del(node->prev, node->next, false);
  AddTail(node);
}

void List::Append(List* from) {
  if (from->IsEmpty()) return;
  ListNode* f_head = from->root_.next;
  ListNode* f_tail = from->root_.prev;

  root_.prev->next = f_head;
  f_head->prev = root_.prev;

  f_tail->next = &root_;
  root_.prev = f_tail;

  count_ += from->count_;
  // must invoke this method
  from->__Init();
}

ListNode* List::GetData(int index) {
  if (IsEmpty()) return &root_;
  if (index < 0 || index >= count_) return nullptr;

  ListNode* temp = &root_;
  int temp_index = -1;

  if (index >= count_ / 2) {  // 使用倒序遍历
    temp_index = count_;
    do {
      if (index != temp_index) {
        --temp_index;
        temp = temp->prev;
      } else {
        return temp;
      }
    } while (true);
  } else {  // 使用正序遍历
    temp_index = -1;
    do {
      if (index != temp_index) {
        ++temp_index;
        temp = temp->next;
      } else {
        return temp;
      }
    } while (true);
  }
}

void List::Clear(bool b) {
  while (!IsEmpty()) {
    DelHead(b);
  }
  count_ = 0;
}

}  // namespace myframe
