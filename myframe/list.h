/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once

/**
 *  双向循环链表
 **/
namespace myframe {

class ListNode {
 public:
  ListNode() : prev(nullptr), next(nullptr) {}
  virtual ~ListNode() {}

  ListNode* prev;

  ListNode* next;
};

class List final {
 public:
  List() { __Init(); }

  ~List() { Clear(false); }

  void AddHead(ListNode* node);

  void AddTail(ListNode* node);

  void Del(ListNode* node, bool b = false);

  void DelHead(bool b = false);

  void DelTail(bool b = false);

  void DelWithIndex(int index, bool b = false);  // not useful

  void MoveHead(ListNode* node);

  void MoveTail(ListNode* node);

  void Append(List* from);

  ListNode* Begin() { return root_.next; }

  ListNode* End() { return &root_; }

 public:
  ListNode* GetData(int index);  // not useful

  int Count() { return count_; }

  bool IsEmpty() { return &root_ == root_.next; }

  void Clear(bool b = false);

 private:
  void __Init();

  void __Add(ListNode* prev, ListNode* next, ListNode* node);

  void __Del(ListNode* prev, ListNode* next, bool b);

  ListNode root_;

  int count_;
};

}  // namespace myframe
