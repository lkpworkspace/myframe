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
    Py_XDECREF(pyactor_);
    pyactor_ = nullptr;
  }

  int Init() {
    // std::cout << GetActorName() << " init\n";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    // 封装python Msg
    // pymyframe::Msg* m = new pymyframe::Msg("", "");
    // PyObject* py_msg = SWIG_NewPointerObj(
    //   (void*)m,
    //   SWIGTYPE_p_pymyframe__Msg,
    //   0);
    // 调用python proc
    PyObject* py_res = PyObject_CallMethod(
      pyactor_,
      "proc",
      "y",  // 表示传入byte类型
      msg->GetData().c_str());
    if (!py_res) {
      PyErr_Print();
      PyGILState_Release(gstate);
      return;
    }
    Py_DECREF(py_res);
    PyGILState_Release(gstate);
  }

  void SetPyObj(PyObject* obj) {
    if (pyactor_) {
      Py_DECREF(pyactor_);
    }
    pyactor_ = obj;
  }

 private:
  PyObject* pyactor_{nullptr};
};

}  // namespace pymyframe
