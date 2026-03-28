#include <sarsa/crash_handler.h>
#include <sarsa/log.h>

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <DbgHelp.h>
    #pragma comment(lib, "Dbghelp.lib")
#else
    #include <csignal>
#endif

namespace sarsa {

static bool write_crash_log() {
    auto messages = Log::recent_messages();
    if (messages.empty()) {
        return false;
    }

    FILE* f = nullptr;
#ifdef _WIN32
    fopen_s(&f, "sarsa_crash.log", "w");
#else
    f = fopen("sarsa_crash.log", "w");
#endif
    if (!f) {
        return false;
    }

    fprintf(f, "=== Sarsa Crash Log ===\n\n");
    fprintf(f, "Last %zu log messages before crash:\n\n", messages.size());
    for (const auto& msg : messages) {
        fprintf(f, "%s\n", msg.c_str());
    }

    fclose(f);
    return true;
}

#ifdef _WIN32

static LONG WINAPI unhandled_exception_handler(EXCEPTION_POINTERS* exception_info) {
    // Write minidump
    HANDLE file = CreateFileA(
        "sarsa_crash.dmp",
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    bool dump_ok = false;
    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dump_info{};
        dump_info.ThreadId = GetCurrentThreadId();
        dump_info.ExceptionPointers = exception_info;
        dump_info.ClientPointers = FALSE;

        dump_ok = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            file,
            MiniDumpNormal,
            &dump_info,
            nullptr,
            nullptr) != 0;

        CloseHandle(file);
    }

    // Write recent log messages to crash log
    bool log_ok = write_crash_log();

    // Print to stderr in case console is still visible
    fprintf(stderr, "\n*** CRASH ***\n");
    if (dump_ok) {
        fprintf(stderr, "  Minidump written to sarsa_crash.dmp\n");
    } else {
        fprintf(stderr, "  Failed to write minidump\n");
    }
    if (log_ok) {
        fprintf(stderr, "  Log written to sarsa_crash.log\n");
    } else {
        fprintf(stderr, "  Failed to write crash log\n");
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void install_crash_handler() {
    SetUnhandledExceptionFilter(unhandled_exception_handler);
}

#else

static void signal_handler(int signal) {
    bool log_ok = write_crash_log();

    fprintf(stderr, "\n*** CRASH: Signal %d received\n", signal);
    if (log_ok) {
        fprintf(stderr, "  Log written to sarsa_crash.log\n");
    } else {
        fprintf(stderr, "  Failed to write crash log\n");
    }

    // Re-raise to get default behavior (core dump)
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void install_crash_handler() {
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGFPE, signal_handler);
    std::signal(SIGILL, signal_handler);
}

#endif

} // namespace sarsa
