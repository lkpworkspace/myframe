/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <pthread.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "myframe/macros.h"

namespace myframe {

class Actor;
class Worker;
class ModLib final {
 public:
  ModLib();
  virtual ~ModLib();

  /**
   * @brief 是否加载动态库
   *
   * @param dlname lib name
   * @return true
   * @return false
   */
  bool IsLoad(const std::string& dlname);

  /**
   * @brief 加载模块动态库
   *
   * @param dlname full lib path
   * @return true
   * @return false
   */
  bool LoadMod(const std::string& dlpath);

  /**
   * @brief 创建actor实例
   *
   * @param mod_name eg: libtest.so
   * @param actor_name eg: /my/test
   * @return std::shared_ptr<Actor>
   */
  std::shared_ptr<Actor> CreateActorInst(const std::string& mod_name,
                                           const std::string& actor_name);

  /**
   * @brief 创建Worker实例
   *
   * @param mod_name eg: libtest.so
   * @param worker_name eg: /my/test
   * @return Worker*
   */
  std::shared_ptr<Worker> CreateWorkerInst(const std::string& mod_name,
                                             const std::string& worker_name);

 private:
  bool UnloadMod(const std::string& dlname);
  std::string GetModName(const std::string& full_path);

  std::unordered_map<std::string, void*> mods_;
  pthread_rwlock_t rw_;

  DISALLOW_COPY_AND_ASSIGN(ModLib)
};

}  // namespace myframe
