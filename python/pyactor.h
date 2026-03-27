/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <Python.h>
#include <string>
#include <memory>
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "pymsg.h"

namespace pymyframe {

class PyActor : public myframe::Actor {
 public:
  PyActor() {
    // std::cout << "pyactor construct\n";
  }

  virtual ~PyActor() {
    // std::cout << GetActorName() << " deconstruct\n";
    // Py_XDECREF(pyactor_);
    // pyactor_ = nullptr;
  }

  int Init() {
    // std::cout << GetActorName() << " init\n";
    PyGILState_STATE gstate = PyGILState_Ensure();
    // 调用python init
    PyObject* py_res = PyObject_CallMethod(
      pyactor_,
      "init",
      NULL);
    if (!py_res) {
      PyErr_Print();
      PyGILState_Release(gstate);
      return -1;
    }
    auto res = PyLong_AsLong(py_res);
    if (res == -1 && PyErr_Occurred()) {
      Py_DECREF(py_res);
      PyErr_Print();
      PyGILState_Release(gstate);
      return -1;
    }
    Py_DECREF(py_res);
    PyGILState_Release(gstate);
    return res;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    // 封装python Msg
    pymyframe::Msg* m = new pymyframe::Msg(GetActorName(), msg->GetData());
    PyObject* py_msg = SWIG_NewPointerObj(
      reinterpret_cast<void*>(m),
      SWIGTYPE_p_pymyframe__Msg,
      SWIG_POINTER_OWN);  // 获得所有权
    // 调用python proc
    PyObject* py_res = PyObject_CallMethod(
      pyactor_,
      "proc",
      "N",  // 传递pyobject，并转移所有权
      py_msg);
    if (!py_res) {
      PyErr_Print();
      PyGILState_Release(gstate);
      return;
    }
    Py_DECREF(py_res);
    PyGILState_Release(gstate);
  }

  void SetPyObj(PyObject* obj) {
    // TODO(FIXME):
    //    如果增加引用会导致应用无法正常退出
    //    因为python判断pyapp持有pyactor引用时，
    //    不会销毁pyapp从而导致应用直接退出，没有清理资源的步骤。

    // if (pyactor_) {
    //   Py_DECREF(pyactor_);
    // }
    pyactor_ = obj;
    // Py_INCREF(pyactor_);
  }

 private:
  PyObject* pyactor_{nullptr};
};

}  // namespace pymyframe
