
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <curl/curl.h>
#include <regex>

using namespace std;

using namespace adsk::core;
using namespace adsk::fusion;

Ptr<Application> app;
Ptr<UserInterface> ui;

// Fetch the value of a given property
// inside a json text
string GetValue(string text, string find)
{
  regex IsValid("\\s*\"" + find + "\"\\s*:\\s*(.+?)[\\s,]");
  smatch m;
  
  if (regex_search(text, m, IsValid))
  {
    if (m.size() > 1)
      return m[1];
  }
  
  return "";
}

size_t callback_func(void *ptr, size_t size, size_t count, void *stream)
{
  // ptr - your string variable.
  // stream - data chunck you received
  
  string reply((char*)ptr);
  string ret = GetValue(reply, "IsValid");
  
  // If the app is not valid
  if (ret != "true")
    ui->messageBox("IsValid = false");
  else
    ui->messageBox("IsValid = true");
  
  return 0;
}

// Using Entitlement API to check if the
// app usage is valid
void CheckValidity()
{
  // e.g. the URL for Voronoi Sketch generator is:
  // https://apps.autodesk.com/FUSION/en/Detail/Index?id=appstore.exchange.autodesk.com%3avoronoisketchgenerator_macos%3aen
  // This cotains the "id":
  // "appstore.exchange.autodesk.com%3avoronoisketchgenerator_macos%3aen"
  // so we can use that
  string userId = app->userId();
  string userName = app->userName();
  string appId =
  "appstore.exchange.autodesk.com%3avoronoisketchgenerator_macos%3aen";
  string url =
  string("https://apps.exchange.autodesk.com/webservices/checkentitlement") +
  string("?userid=") + userId +
  string("&appid=") + appId;
  
  CURL * curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_func);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
}

extern "C" XI_EXPORT bool run(const char* context)
{
  app = Application::get();
  if (!app)
    return false;
  
  ui = app->userInterface();
  if (!ui)
    return false;
  
  ui->messageBox("in run");
  
  CheckValidity();
  
  return true;
}

extern "C" XI_EXPORT bool stop(const char* context)
{
  if (ui)
  {
    ui->messageBox("in stop");
    ui = nullptr;
  }
  
  return true;
}

#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

#endif // XI_WIN
