/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <utility>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>

#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/common.h"

namespace myframe {

class Actor;
class Worker;
class SharedLibrary;
class MYFRAME_EXPORT ModManager final {
 public:
  ModManager();
  virtual ~ModManager();

  bool RegActor(
    const std::string& class_name,
    std::function<std::shared_ptr<Actor>(const std::string&)> func);

  bool RegWorker(
    const std::string& class_name,
    std::function<std::shared_ptr<Worker>(const std::string&)> func);

  std::shared_ptr<Actor> CreateActorInst(
    const std::string& mod_or_class_name,
    const std::string& class_name);

  std::shared_ptr<Worker> CreateWorkerInst(
    const std::string& mod_or_class_name,
    const std::string& class_name);

  bool LoadService(
    const stdfs::path& lib_dir,
    const Json::Value& service,
    std::vector<std::pair<Json::Value, std::shared_ptr<Actor>>>* actors,
    std::vector<std::pair<Json::Value, std::shared_ptr<Worker>>>* workers);

 private:
  bool LoadMod(const std::string& dl_path);

  bool LoadActors(
    const std::string& mod_name,
    const std::string& class_name,
    const Json::Value& actor_list,
    std::vector<std::pair<Json::Value, std::shared_ptr<Actor>>>* actors);

  bool LoadWorkers(
    const std::string& mod_name,
    const std::string& class_name,
    const Json::Value& worker_list,
    std::vector<std::pair<Json::Value, std::shared_ptr<Worker>>>* workers);

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
