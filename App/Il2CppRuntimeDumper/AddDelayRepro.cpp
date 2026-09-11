// AddDelayRepro.dll - 临时、自动恢复的 AddDelay 执行流验证器
// 只在当前进程线程上下文设置执行断点；不修改代码/字段，不调用游戏方法。

#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")

namespace {
FILE *g_log = nullptr;
std::string g_path;
std::string g_stopPath;
uintptr_t g_target = 0;
uintptr_t g_sync = 0;
uintptr_t g_gameStatics = 0;
volatile LONG g_hits = 0;
PVOID g_veh = nullptr;
const DWORD kArmMs = 8000;

void Log(const char *fmt, ...) {
  if (!g_log) return;
  va_list ap; va_start(ap, fmt); vfprintf(g_log, fmt, ap); va_end(ap);
  fputc('\n', g_log); fflush(g_log);
}
bool ReadBytes(const void *p, void *out, size_t n) {
  __try { memcpy(out, p, n); return true; } __except (EXCEPTION_EXECUTE_HANDLER) { memset(out, 0, n); return false; }
}
template<class T> bool ReadV(uintptr_t p, T &v) { return ReadBytes((const void *)p, &v, sizeof(v)); }
bool Ptr(uintptr_t p) { return p >= 0x10000ull && p < 0x800000000000ull && !(p & 7); }
bool CStr(uintptr_t p, char *out, size_t cap) {
  if (!out || !cap || p < 0x10000ull || p >= 0x800000000000ull) return false;
  for (size_t i=0; i+1<cap; ++i) { char c=0; if(!ReadV(p+i,c)) return false; out[i]=c; if(!c)return true; if((unsigned char)c<0x20 && c!='\t')return false; }
  out[cap-1]=0; return false;
}
struct Mod { uintptr_t base=0; DWORD size=0; char name[MAX_PATH]{}; DWORD ts=0; };
bool FindMod(Mod &m) {
  const char *ns[]={"GameAssembly.dll","GameAssembly_Super.dll","GameAssembly_Super_IBT.dll"};
  for(const char *n:ns){ HMODULE h=GetModuleHandleA(n); if(!h)continue; MODULEINFO i{}; if(!GetModuleInformation(GetCurrentProcess(),h,&i,sizeof(i)))continue; m.base=(uintptr_t)h;m.size=i.SizeOfImage;strncpy_s(m.name,n,_TRUNCATE); unsigned char b[0x400]{}; if(ReadBytes(h,b,sizeof(b))&&b[0]=='M'&&b[1]=='Z'){DWORD e=0;memcpy(&e,b+0x3c,4);ReadV(m.base+e+8,m.ts);}return true; }
  return false;
}
void OpenLog(HMODULE self) {
  char p[MAX_PATH]{}; GetModuleFileNameA(self,p,MAX_PATH); std::string s(p); size_t slash=s.find_last_of("\\/"); std::string dir=slash==std::string::npos?".":s.substr(0,slash); std::string cfg=s.substr(0,s.rfind('.'))+".cfg"; FILE*f=nullptr; if(fopen_s(&f,cfg.c_str(),"rb")==0&&f){char line[MAX_PATH]{};if(fgets(line,sizeof(line),f)){size_t a=strspn(line," \t\r\n"),b=strlen(line);while(b>a&&(line[b-1]=='\r'||line[b-1]=='\n'||line[b-1]==' '||line[b-1]=='\t'))--b;if(b>a)dir.assign(line+a,b-a);}fclose(f);}CreateDirectoryA(dir.c_str(),nullptr);g_path=dir+"\\adddelay_repro.log";g_stopPath=dir+"\\adddelay_repro.stop";fopen_s(&g_log,g_path.c_str(),"ab");
}
bool ResolveObject(const Mod &m, DWORD rva, const char *want, uintptr_t &obj, uintptr_t &statics, uintptr_t &instance) {
  uintptr_t k=0,np=0; if(!ReadV(m.base+rva,k)||!Ptr(k)||!ReadV(k+0x10,np))return false; char n[128]{};CStr(np,n,sizeof(n)); if(strcmp(n,want)!=0)return false; if(!ReadV(k+0xB8,statics)||!Ptr(statics))return false; instance=0;ReadV(statics+0x8,instance);obj=k;Log("[OBJ] %s rva=0x%X klass=0x%llX statics=0x%llX instance=0x%llX",want,rva,(unsigned long long)k,(unsigned long long)statics,(unsigned long long)instance);return true;
}
LONG CALLBACK Veh(PEXCEPTION_POINTERS ep) {
  if (ep && ep->ExceptionRecord && ep->ContextRecord && ep->ExceptionRecord->ExceptionCode==EXCEPTION_SINGLE_STEP && ep->ContextRecord->Rip==g_target) {
    ep->ContextRecord->Rip += 5; InterlockedIncrement(&g_hits); return EXCEPTION_CONTINUE_EXECUTION;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}
bool SetBp(DWORD tid, bool enable) {
  if(tid==GetCurrentThreadId())return false;
  HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,tid); if(!h)return false;
  bool ok=false; if(SuspendThread(h)!=DWORD(-1)){CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;if(GetThreadContext(h,&c)){if(enable){c.Dr0=g_target;c.Dr7|=1ull;}else{c.Dr0=0;c.Dr7&=~1ull;}ok=SetThreadContext(h,&c)!=0;}ResumeThread(h);}CloseHandle(h);return ok;
}
void ApplyAll(bool enable) {
  HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
  if(s==INVALID_HANDLE_VALUE)return; THREADENTRY32 e{};e.dwSize=sizeof(e);DWORD pid=GetCurrentProcessId();if(Thread32First(s,&e)){do{if(e.th32OwnerProcessID==pid)SetBp(e.th32ThreadID,enable);}while(Thread32Next(s,&e));}CloseHandle(s);
}
void Sample(unsigned seq) {
  double d=0,g=0;float r=0;bool okd=ReadV(g_sync+0x10,d),okr=ReadV(g_sync+0x68,r),okg=Ptr(g_gameStatics)&&ReadV(g_gameStatics+0x28,g);Log("[SAMPLE] #%u D=%.9f R=%.6f G=%.9f G+D=%.9f read=%s hits=%ld",seq,d,(double)r,g,g+d,(okd&&okr&&okg)?"OK":"PARTIAL",g_hits);
}
DWORD WINAPI Worker(LPVOID arg) {
  OpenLog((HMODULE)arg);Log("=== AddDelayRepro 持续复现开始 ===");Mod m;if(!FindMod(m)){Log("[FAIL] GameAssembly 未找到");if(g_log)fclose(g_log);FreeLibraryAndExitThread((HMODULE)arg,0);return 0;}Log("[MODULE] name=%s base=0x%llX size=0x%X timestamp=0x%08X",m.name,(unsigned long long)m.base,m.size,m.ts);if(strcmp(m.name,"GameAssembly.dll")!=0){Log("[STOP] 当前不是普通版，拒绝使用普通版 RVA");if(g_log)fclose(g_log);FreeLibraryAndExitThread((HMODULE)arg,0);return 0;}g_target=m.base+0x156C395;unsigned char code[5]{};ReadBytes((const void*)g_target,code,5);Log("[TARGET] RVA=0x156C395 VA=0x%llX bytes=%02X %02X %02X %02X %02X",(unsigned long long)g_target,code[0],code[1],code[2],code[3],code[4]);uintptr_t st=0,inst=0,syncKlass=0;if(!ResolveObject(m,0xE606490,"CharactorSync",syncKlass,st,inst)||!inst){Log("[FAIL] CharactorSync 未初始化");if(g_log)fclose(g_log);FreeLibraryAndExitThread((HMODULE)arg,0);return 0;}g_sync=inst;uintptr_t bk=0;ResolveObject(m,0xE644808,"GameBaseObject",bk,g_gameStatics,inst);if(GetFileAttributesA(g_stopPath.c_str())!=INVALID_FILE_ATTRIBUTES){Log("[STOP] 发现停止标志，未设置断点：%s",g_stopPath.c_str());if(g_log)fclose(g_log);FreeLibraryAndExitThread((HMODULE)arg,0);return 0;}g_veh=AddVectoredExceptionHandler(1,Veh);Log("[BASELINE] 先采集 10 个样本");for(unsigned i=0;i<10;i++){Sample(i);Sleep(100);}Log("[ARM] 已进入持续复现；每 100ms 补齐线程断点；创建停止标志即可恢复");unsigned seq=10;while(GetFileAttributesA(g_stopPath.c_str())==INVALID_FILE_ATTRIBUTES){ApplyAll(true);Sample(seq++);Sleep(100);}Log("[STOP] 收到停止标志，开始清理");ApplyAll(false);Log("[RESTORE] 已清除执行断点，hits=%ld；继续观察恢复",g_hits);for(unsigned i=0;i<20;i++){Sample(seq++);Sleep(100);}if(g_veh){RemoveVectoredExceptionHandler(g_veh);g_veh=nullptr;}Log("=== AddDelayRepro 持续复现结束，代码页未写入，输出=%s ===",g_path.c_str());if(g_log)fclose(g_log);FreeLibraryAndExitThread((HMODULE)arg,0);return 0;
}
}
BOOL APIENTRY DllMain(HMODULE h,DWORD r,LPVOID){if(r==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);HANDLE t=CreateThread(nullptr,0,Worker,h,0,nullptr);if(t)CloseHandle(t);}return TRUE;}
