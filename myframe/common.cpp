/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/common.h"

#include <string.h>
#include <utility>

#include "myframe/platform.h"
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
#include <sched.h>
#include <unistd.h>
#elif defined(MYFRAME_OS_WINDOWS)
#include <Windows.h>
#elif defined(MYFRAME_OS_MACOSX)
#include <mach-o/dyld.h>
#include <mach/mach.h>
#else
#error "Platform not supported"
#endif

#include <fstream>
#include <sstream>

#define MYFRAME_MAX_PATH 256

namespace myframe {

std::vector<stdfs::path> Common::GetDirFiles(const std::string& conf_path) {
  std::vector<stdfs::path> res;
  stdfs::path path(conf_path);
  for (auto const& dir_entry : stdfs::directory_iterator{path}) {
    if (dir_entry.is_regular_file()) {
      res.emplace_back(dir_entry.path());
    }
  }
  return res;
}

Json::Value Common::LoadJsonFromFile(const std::string& json_file) {
  std::ifstream ifs(json_file);
  if (!ifs.is_open()) {
    return Json::Value::nullSingleton();
  }
  Json::Value root;
  Json::Reader reader(Json::Features::strictMode());
  if (!reader.parse(ifs, root)) {
    ifs.close();
    return Json::Value::nullSingleton();
  }
  ifs.close();
  return root;
}

stdfs::path Common::GetWorkRoot() {
  char path_buf[MYFRAME_MAX_PATH];
  memset(path_buf, 0, MYFRAME_MAX_PATH);
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  int ret = readlink("/proc/self/exe", path_buf, MYFRAME_MAX_PATH);
  if (ret == -1) {
    return "";
  }
#elif defined(MYFRAME_OS_WINDOWS)
  auto ret = GetModuleFileName(NULL, path_buf, MYFRAME_MAX_PATH);
  if (ret == 0) {
    return "";
  }
#elif defined(MYFRAME_OS_MACOSX)
  uint32_t ret_path_buf_size = MYFRAME_MAX_PATH;
  auto ret = ::_NSGetExecutablePath(path_buf, &ret_path_buf_size);
  if (ret == -1) {
    return "";
  }
#else
  #error "Platform not supported"
#endif
  if (static_cast<std::size_t>(ret) >= MYFRAME_MAX_PATH) {
    path_buf[MYFRAME_MAX_PATH - 1] = '\0';
  }
  stdfs::path p(path_buf);
  if (p.has_parent_path()) {
    p = p.parent_path();
    if (p.has_parent_path()) {
      p = p.parent_path();
    }
  }
  return p;
}

stdfs::path Common::GetAbsolutePath(const std::string& flag_path) {
  stdfs::path p(flag_path);
  if (p.is_absolute()) {
    return flag_path;
  }
  auto root = GetWorkRoot();
  if (root.empty()) {
    return flag_path;
  }
  root /= p;
  return root;
}

bool Common::IsAbsolutePath(const std::string& path) {
  stdfs::path p(path);
  if (p.is_absolute()) {
    return true;
  }
  return false;
}

std::vector<std::string_view> Common::SplitMsgName(const std::string& name) {
  std::vector<std::string_view> tokens;
  tokens.reserve(3);
  size_t name_sz = name.size();
  size_t start_pos = 0;
  for (size_t i = 0; i < name_sz; ++i) {
    if (name[i] == '.') {
      tokens.emplace_back(&name[start_pos], i - start_pos);
      start_pos = i + 1;
    }
    if (i == name_sz - 1) {
      tokens.emplace_back(&name[start_pos], i - start_pos + 1);
    }
  }
  return tokens;
}

// 可以通过std::thread::hardware_concurrency()获得核心数
int Common::SetThreadAffinity(std::thread* t, int cpu_core) {
#if defined(MYFRAME_OS_WINDOWS)
  auto hThread = t->native_handle();
  DWORD_PTR mask = 1 << cpu_core;
  if (SetThreadAffinityMask(hThread, mask) == 0) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  thread_affinity_policy policy;
  policy.affinity_tag = cpu_core;
  auto handle = pthread_mach_thread_np(t->native_handle());
  kern_return_t kr = thread_policy_set(
    handle,
    THREAD_AFFINITY_POLICY,
    reinterpret_cast<thread_policy_t>(&policy),
    THREAD_AFFINITY_POLICY_COUNT);
  mach_port_deallocate(mach_task_self(), handle);
  if (kr != KERN_SUCCESS) {
    return -1;
  }
  return 0;
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_core, &cpuset);
  auto handle = t->native_handle();
  int result = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
  return result;
#endif
}

int Common::SetSelfThreadAffinity(int cpu_core) {
#if defined(MYFRAME_OS_WINDOWS)
  auto hThread = GetCurrentThread();
  DWORD_PTR mask = 1 << cpu_core;
  if (SetThreadAffinityMask(hThread, mask) == 0) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  thread_affinity_policy policy;
  policy.affinity_tag = cpu_core;
  auto handle = mach_thread_self();
  kern_return_t kr = thread_policy_set(
    handle,
    THREAD_AFFINITY_POLICY,
    reinterpret_cast<thread_policy_t>(&policy),
    THREAD_AFFINITY_POLICY_COUNT);
  mach_port_deallocate(mach_task_self(), handle);
  if (kr != KERN_SUCCESS) {
    return -1;
  }
  return 0;
#else
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_core, &cpuset);
  auto handle = pthread_self();
  int result = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
  return result;
#endif
}

int Common::SetThreadName(std::thread* t, const std::string& name) {
#if defined(MYFRAME_OS_WINDOWS)
  int len = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
  if (len <= 0) {
    return -1;
  }
  wchar_t* wide_name = new wchar_t[len];
  MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wide_name, len);
  auto handle = t->native_handle();
  auto res = SetThreadDescription(handle, wide_name);
  delete[] wide_name;
  if (FAILED(res)) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  // unsupport
  return -1;
#else
  auto handle = t->native_handle();
  return pthread_setname_np(handle, name.c_str());
#endif
}

int Common::SetSelfThreadName(const std::string& name) {
#if defined(MYFRAME_OS_WINDOWS)
  int len = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
  if (len <= 0) {
    return -1;
  }
  wchar_t* wide_name = new wchar_t[len];
  MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wide_name, len);
  auto handle = GetCurrentThread();
  auto res = SetThreadDescription(handle, wide_name);
  delete[] wide_name;
  if (FAILED(res)) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  return pthread_setname_np(name.c_str());
#else
  return pthread_setname_np(pthread_self(), name.c_str());
#endif
}

int Common::SetProcessPriority(ProcessPriority pp) {
#if defined(MYFRAME_OS_WINDOWS)
  HANDLE hProcess = ::GetCurrentProcess();
  int process_pri;
  if (pp == ProcessPriority::kLowest) {
    process_pri = IDLE_PRIORITY_CLASS;
  } else if (pp == ProcessPriority::kNormal) {
    process_pri = NORMAL_PRIORITY_CLASS;
  } else {
    process_pri = REALTIME_PRIORITY_CLASS;
  }
  if (!::SetPriorityClass(hProcess, process_pri)) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  (void)pp;
  return 0;
#else
  struct sched_param process_param;
  int sched_policy = SCHED_OTHER;
  if (pp == ProcessPriority::kLowest) {
    process_param.sched_priority = 0;
    sched_policy = SCHED_IDLE;
  } else if (pp == ProcessPriority::kNormal) {
    process_param.sched_priority = 0;
    sched_policy = SCHED_OTHER;
  } else {
    process_param.sched_priority = 99;
    sched_policy = SCHED_RR;
  }
  if (sched_setscheduler(0, sched_policy, &process_param)) {
    return -1;
  }
  return 0;
#endif
}

int Common::SetThreadPriority(std::thread* t, ThreadPriority tp) {
#if defined(MYFRAME_OS_WINDOWS)
  int thread_pri;
  if (tp == ThreadPriority::kLowest) {
    thread_pri = THREAD_PRIORITY_IDLE;
  } else if (tp == ThreadPriority::kNormal) {
    thread_pri = THREAD_PRIORITY_NORMAL;
  } else {
    thread_pri = THREAD_PRIORITY_TIME_CRITICAL;
  }
  HANDLE thread_handle;
  if (t == nullptr) {
    thread_handle = GetCurrentThread();
  } else {
    thread_handle = t->native_handle();
  }
  if (!::SetThreadPriority(thread_handle, thread_pri)) {
    return -1;
  }
  return 0;
#elif defined(MYFRAME_OS_MACOSX)
  (void)t;
  (void)tp;
  return 0;
#else
  pthread_t handle;
  if (t == nullptr) {
    handle = pthread_self();
  } else {
    handle = t->native_handle();
  }
  struct sched_param thread_param;
  int sched_policy = SCHED_OTHER;
  if (tp == ThreadPriority::kLowest) {
    thread_param.sched_priority = 0;
    sched_policy = SCHED_IDLE;
  } else if (tp == ThreadPriority::kNormal) {
    thread_param.sched_priority = 0;
    sched_policy = SCHED_OTHER;
  } else {
    thread_param.sched_priority = 99;
    sched_policy = SCHED_RR;
  }
  if (pthread_setschedparam(handle, sched_policy, &thread_param)) {
    return -1;
  }
  return 0;
#endif
}

}  // namespace myframe
