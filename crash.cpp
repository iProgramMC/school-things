#include <windows.h>
#include <stdio.h>

//#define ACTUALLY_CRASH

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

typedef LONG NTSTATUS;
//! RtlAdjustPrivilege
typedef NTSTATUS(NTAPI *PRTLADJUSTPRIVILEGE)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
//! NtRaiseHardError
typedef NTSTATUS(NTAPI *PNTRAISEHARDERROR)(NTSTATUS ErrorStatus, ULONG NumberOfParameters,ULONG UnicodeStringParameterMask, PULONG_PTR *Parameters, ULONG ValidResponseOption, PULONG Response);
//! NtTestAlert
typedef NTSTATUS(NTAPI *PNTTESTALERT)();

HMODULE g_ntdll;
PRTLADJUSTPRIVILEGE g_RtlAdjustPrivilege;
PNTRAISEHARDERROR   g_NtRaiseHardError;
PNTTESTALERT        g_NtTestAlert;

#define SE_SHUTDOWN_PRIVILEGE 19

#ifdef ACTUALLY_CRASH
#define ERROR_CODE 0xDEADDEAD // MANUALLY_INITIATED_CRASH1
#else
#define ERROR_CODE STATUS_SEGMENT_NOTIFICATION
#endif // ACTUALLY_CRASH

void SetupPrivilege() {
    NTSTATUS state; BOOLEAN garbage;
    state = g_RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE, true, false, &garbage);
}
void RaiseHardError(NTSTATUS errState) {
    NTSTATUS state; ULONG r;
    state = g_NtRaiseHardError(errState, 0, 0, NULL, 6, &r);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int cmdShow)
{
    g_ntdll = GetModuleHandle("ntdll.dll");

    if (g_ntdll) {
        g_RtlAdjustPrivilege = (PRTLADJUSTPRIVILEGE)GetProcAddress(g_ntdll, "RtlAdjustPrivilege");
        g_NtRaiseHardError   = (PNTRAISEHARDERROR)  GetProcAddress(g_ntdll, "NtRaiseHardError");
        g_NtTestAlert        = (PNTTESTALERT)       GetProcAddress(g_ntdll, "NtTestAlert");
    }
    else
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(NULL, "Curse you, can't do jack to your computer.", "Yes", MB_ICONERROR);
    }

    int h = MessageBoxA(NULL, "Would you Like to crash your system? :)", "ah yes", MB_YESNO | MB_ICONQUESTION);

    if (h == IDNO) {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(NULL, "Yeahhh there you go", "h", MB_ICONINFORMATION);
        return 0;
    }
    // answered yes

    SetupPrivilege();
    RaiseHardError(ERROR_CODE);

    //MessageBoxA(NULL, ":wholesome:", ":wholesome:", MB_ICONINFORMATION);

	return 0;
}
