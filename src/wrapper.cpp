#include "weston.h"
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <napi.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <uv.h>
#include <vector>

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

  setenv("WESTON_DATA_DIR", (artifactsPath / "weston").c_str(), 1);

  return env.Undefined();
}

struct WetMainContext {
  explicit WetMainContext(Napi::Env env, Napi::Promise::Deferred deferred)
      : env(env), deferred(deferred) {}

  Napi::Env env;
  Napi::Promise::Deferred deferred;
  uv_async_t async_complete;
  uv_async_t async_ready;
  Napi::FunctionReference ready_callback;

  int argc;
  char **argv;
  std::vector<std::string> *argv_storage;

  int result;
};

static WetMainContext *g_current_context = nullptr;

void OnWetMainReady(uv_async_t *handle) {
  WetMainContext *ctx = static_cast<WetMainContext *>(handle->data);
  Napi::HandleScope scope(ctx->env);
  if (!ctx->ready_callback.IsEmpty()) {
    ctx->ready_callback.Call({});
  }
  uv_close(reinterpret_cast<uv_handle_t *>(handle), nullptr);
}

void ReadyCallbackTrampoline() {
  if (g_current_context) {
    uv_async_send(&g_current_context->async_ready);
  }
}

void OnWetMainComplete(uv_async_t *handle) {
  WetMainContext *ctx = static_cast<WetMainContext *>(handle->data);
  Napi::HandleScope scope(ctx->env);

  g_current_context = nullptr;

  ctx->deferred.Resolve(Napi::Number::New(ctx->env, ctx->result));

  delete[] ctx->argv;
  delete ctx->argv_storage;
  ctx->ready_callback.Reset();

  uv_close(reinterpret_cast<uv_handle_t *>(&ctx->async_complete),
           [](uv_handle_t *handle) {
             delete static_cast<WetMainContext *>(handle->data);
           });
}

Napi::Value WetMain(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (g_current_context != nullptr) {
    Napi::Error::New(env, "An instance of wetMain is already running.")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info.Length() < 1 || info.Length() > 2 || !info[0].IsArray() ||
      (info.Length() == 2 && !info[1].IsFunction())) {
    Napi::TypeError::New(
        env, "Expected arguments: string[], [readyCallback: () => void]")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Array jsArgv = info[0].As<Napi::Array>();
  int argc = jsArgv.Length();

  auto *ctx = new WetMainContext(env, Napi::Promise::Deferred::New(env));
  ctx->argc = argc;
  ctx->argv_storage = new std::vector<std::string>();
  ctx->argv_storage->reserve(argc);
  ctx->argv = new char *[argc + 1];

  for (int i = 0; i < argc; ++i) {
    Napi::Value val = jsArgv[i];
    if (!val.IsString()) {
      delete ctx;
      delete[] ctx->argv;
      delete ctx->argv_storage;
      Napi::TypeError::New(env, "All arguments in the array must be strings")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
    ctx->argv_storage->emplace_back(val.As<Napi::String>().Utf8Value());
    ctx->argv[i] = &ctx->argv_storage->back()[0];
  }
  ctx->argv[argc] = nullptr;

  ready_callback captured_callback = nullptr;

  if (info.Length() == 2) {
    ctx->ready_callback = Napi::Persistent(info[1].As<Napi::Function>());
    ctx->async_ready.data = ctx;
    uv_async_init(uv_default_loop(), &ctx->async_ready, OnWetMainReady);
    captured_callback = ReadyCallbackTrampoline;
  }

  ctx->async_complete.data = ctx;
  uv_async_init(uv_default_loop(), &ctx->async_complete, OnWetMainComplete);

  g_current_context = ctx;

  std::thread([ctx, captured_callback]() {
    ready_callback thread_local_callback = captured_callback;

    ready_callback *ptr_to_pass =
        (thread_local_callback) ? &thread_local_callback : nullptr;

    ctx->result = wet_main(ctx->argc, ctx->argv, nullptr, ptr_to_pass);
    uv_async_send(&ctx->async_complete);
  }).detach();

  return ctx->deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("initialize", Napi::Function::New(env, Initialize));
  exports.Set("wetMain", Napi::Function::New(env, WetMain));
  return exports;
}

NODE_API_MODULE(nodejs_wayland, Init)
