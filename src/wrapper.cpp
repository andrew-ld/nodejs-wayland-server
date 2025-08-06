#include "weston.h"
#include <dlfcn.h>
#include <filesystem>
#include <napi.h>
#include <stdlib.h>
#include <string>

namespace fs = std::filesystem;

std::string GetAddonPath() {
  Dl_info dl_info;
  if (dladdr((void *)GetAddonPath, &dl_info) == 0) {
    return "";
  }
  return std::string(dl_info.dli_fname);
}

Napi::Value Initialize(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::string addonPathStr = GetAddonPath();
  if (addonPathStr.empty()) {
    Napi::Error::New(env, "Failed to get addon path using dladdr()")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  fs::path addonPath = addonPathStr;
  fs::path artifactsPath = addonPath.parent_path();

  std::string moduleMapString;

  for (const auto &entry : fs::directory_iterator(artifactsPath)) {
    std::string filename = entry.path().filename().string();

    if (fs::is_regular_file(entry.status()) &&
        filename.find(".so") != std::string::npos) {
      if (!moduleMapString.empty()) {
        moduleMapString += ";";
      }
      moduleMapString += filename + "=" + entry.path().string();
    }
  }

  if (!moduleMapString.empty()) {
    setenv("WESTON_MODULE_MAP", moduleMapString.c_str(), 1);
  }

  return env.Undefined();
}

Napi::Value WetMain(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsArray()) {
    Napi::TypeError::New(env,
                         "Expected one argument: an array of strings for argv")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Array jsArgv = info[0].As<Napi::Array>();
  int argc = jsArgv.Length();

  std::vector<std::string> argv_storage;
  std::vector<char *> argv_pointers;

  argv_storage.reserve(argc);
  argv_pointers.reserve(argc + 1);

  for (int i = 0; i < argc; i++) {
    Napi::Value val = jsArgv[i];
    if (!val.IsString()) {
      Napi::TypeError::New(env, "Argument array must only contain strings.")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    argv_storage.push_back(val.As<Napi::String>().Utf8Value());
    argv_pointers.push_back(&argv_storage.back()[0]);
  }

  argv_pointers.push_back(nullptr);

  int exitCode = wet_main(argc, argv_pointers.data(), nullptr);

  return Napi::Number::New(env, exitCode);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("initialize", Napi::Function::New(env, Initialize));
  exports.Set("wetMain", Napi::Function::New(env, WetMain));
  return exports;
}

NODE_API_MODULE(nodejs_wayland, Init)
