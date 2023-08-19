/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>

#include "myframe/export.h"
#include "myframe/macros.h"

namespace myframe {

class Actor;
class Worker;
class SharedLibrary;
class MYFRAME_EXPORT ModManager final {
 public:
  ModManager();
  virtual ~ModManager();

  bool LoadMod(const std::string& dl_path);

  bool RegActor(
    const std::string& class_name,
    std::function<std::shared_ptr<Actor>(const std::string&)> func);

  bool RegWorker(
    const std::string& class_name,
    std::function<std::shared_ptr<Worker>(const std::string&)> func);

  std::shared_ptr<Actor> CreateActorInst(
    const std::string& mod_or_class_name,
    const std::string& actor_name);

  std::shared_ptr<Worker> CreateWorkerInst(
    const std::string& mod_or_class_name,
    const std::string& worker_name);

  std::string GetLibName(const std::string& path) const;

 private:
  std::unordered_map<
      std::string, std::function<std::shared_ptr<Actor>(const std::string&)>>
      class_actors_;
  std::unordered_map<
      std::string, std::function<std::shared_ptr<Worker>(const std::string&)>>
      class_workers_;
  std::shared_mutex class_actor_rw_;
  std::shared_mutex class_worker_rw_;

  std::unordered_map<std::string, std::shared_ptr<SharedLibrary>> mods_;
  std::shared_mutex mods_rw_;

  DISALLOW_COPY_AND_ASSIGN(ModManager)
};

}  // namespace myframe
