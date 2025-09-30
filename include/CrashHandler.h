// CrashHandler.h
#pragma once

#include <cstdio>     // FILE, fopen, fprintf, fflush, fclose
#include <csignal>    // signal, SIG*
#include <exception>  // std::exception, std::current_exception, std::terminate_handler
#include <new>        // std::set_new_handler, std::set_terminate
#include <ctime>      // std::time, std::tm, std::localtime
#include <utility>    // std::forward
#include <cstdlib>    // std::getenv
#include <cstring>    // std::strrchr, std::strlen, std::strncmp, std::snprintf

namespace CrashHandler {

    // ---------- Minimal RAII for FILE* -------------------------------------------
    struct FileGuard {
        FILE* f{ nullptr };
        FileGuard() = default;
        explicit FileGuard(FILE* fp) : f(fp) {}
        FileGuard(const FileGuard&) = delete;
        FileGuard& operator=(const FileGuard&) = delete;
        FileGuard(FileGuard&& o) noexcept : f(o.f) { o.f = nullptr; }
        FileGuard& operator=(FileGuard&& o) noexcept {
            if (this != &o) { if (f) std::fclose(f); f = o.f; o.f = nullptr; }
            return *this;
        }
        ~FileGuard() { if (f) std::fclose(f); }
        explicit operator bool() const noexcept { return f != nullptr; }
    };

    // ---------- Internal state ----------------------------------------------------
    inline std::terminate_handler g_prev_terminate = nullptr;
    inline void (*g_prev_sigabrt)(int) = SIG_DFL;
    inline void (*g_prev_sigsegv)(int) = SIG_DFL;
    inline void (*g_prev_sigill)(int) = SIG_DFL;
    inline void (*g_prev_sigfpe)(int) = SIG_DFL;
    inline void (*g_prev_sigterm)(int) = SIG_DFL;

    // Storage for user path (copied) and effective path actually used.
    inline char  g_path_storage[1024] = ".log";
    inline const char* g_log_path = g_path_storage;
    inline char  g_effective_path[1024] = ".log";

    // ---------- Small helpers -----------------------------------------------------
    inline const char* SigName(int sig) noexcept {
        switch (sig) {
        case SIGABRT: return "SIGABRT";
        case SIGSEGV: return "SIGSEGV";
        case SIGILL:  return "SIGILL";
        case SIGFPE:  return "SIGFPE";
        case SIGTERM: return "SIGTERM";
        default:      return "SIGNAL";
        }
    }

    inline void Timestamp(char* buf, std::size_t n) noexcept {
        if (!buf || n == 0) return;
        std::time_t t = std::time(nullptr);
        std::tm* lt = std::localtime(&t);
        if (lt) std::snprintf(buf, n, "%04d-%02d-%02d %02d:%02d:%02d",
            lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
            lt->tm_hour, lt->tm_min, lt->tm_sec);
        else    std::snprintf(buf, n, "unknown-time");
    }

    inline bool is_absolute(const char* p) noexcept {
        if (!p || !*p) return false;
#if defined(_WIN32)
        // "C:\..." or "\\server\share" or leading slash
        if ((std::strlen(p) >= 3 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':' &&
            (p[2] == '\\' || p[2] == '/')) ||
            (p[0] == '\\' && p[1] == '\\') || p[0] == '/' || p[0] == '\\')
            return true;
        return false;
#else
        return p[0] == '/';
#endif
    }

    inline const char* basename_of(const char* path) noexcept {
        if (!path || !*path) return ".log";
        const char* b1 = std::strrchr(path, '/');
        const char* b2 = std::strrchr(path, '\\');
        const char* b = (b1 && b2) ? (b1 > b2 ? b1 : b2) : (b1 ? b1 : b2);
        return b ? (b + 1) : path;
    }

    // Join dir + filename into out (size out_n). dir may or may not end with slash.
    inline void join_path(char* out, std::size_t out_n, const char* dir, const char* fname) noexcept {
        if (!out || out_n == 0) return;
#if defined(_WIN32)
        const char sep = '\\';
#else
        const char sep = '/';
#endif
        if (!dir || !*dir) {
            std::snprintf(out, out_n, "%s", fname && *fname ? fname : ".log");
            return;
        }
        const std::size_t len = std::strlen(dir);
        const bool has_sep = len && (dir[len - 1] == '/' || dir[len - 1] == '\\');
        if (has_sep)
            std::snprintf(out, out_n, "%s%s", dir, (fname && *fname) ? fname : ".log");
        else
            std::snprintf(out, out_n, "%s%c%s", dir, sep, (fname && *fname) ? fname : ".log");
    }

    // Attempt to open the configured path; on failure, fall back to temp dirs.
    inline FileGuard open_log_for_append() noexcept {
        // 1) Try exact requested path.
        if (FILE* f = std::fopen(g_log_path, "a")) {
            std::snprintf(g_effective_path, sizeof(g_effective_path), "%s", g_log_path);
            return FileGuard{ f };
        }

        // 2) Fall back to environment temp directories (Windows/POSIX).
        const char* cand_envs[] = {
    #if defined(_WIN32)
            "LOCALAPPDATA", "TEMP", "TMP", "USERPROFILE"
    #else
            "TMPDIR", "TEMP", "TMP", "HOME"
    #endif
        };
        const char* base_name = basename_of(g_log_path);

        for (const char* key : cand_envs) {
            const char* dir = std::getenv(key);
            if (dir && *dir) {
                char candidate[1024];
                join_path(candidate, sizeof(candidate), dir, base_name);
                if (FILE* f = std::fopen(candidate, "a")) {
                    std::snprintf(g_effective_path, sizeof(g_effective_path), "%s", candidate);
                    return FileGuard{ f };
                }
            }
        }

        // 3) Last resort POSIX: /tmp  (safe on macOS/Linux; on Windows this is ignored)
#if !defined(_WIN32)
        {
            char candidate[1024];
            join_path(candidate, sizeof(candidate), "/tmp", base_name);
            if (FILE* f = std::fopen(candidate, "a")) {
                std::snprintf(g_effective_path, sizeof(g_effective_path), "%s", candidate);
                return FileGuard{ f };
            }
        }
#endif

        // 4) Give up.
        return FileGuard{};
    }

    // Centralized logging (best-effort; non-throwing).
    inline void LogLine(const char* line) noexcept {
        FileGuard fp = open_log_for_append();
        if (!fp) return;
        char ts[32]; Timestamp(ts, sizeof ts);
        std::fprintf(fp.f, "[%s] %s\n", ts, line ? line : "(null)");
        std::fflush(fp.f);
    }

    template <typename... Args>
    inline void Logf(const char* fmt, Args... args) noexcept {
        FileGuard fp = open_log_for_append();
        if (!fp) return;
        char ts[32]; Timestamp(ts, sizeof ts);
        std::fprintf(fp.f, "[%s] ", ts);
        std::fprintf(fp.f, fmt ? fmt : "(null)", args...);
        std::fputc('\n', fp.f);
        std::fflush(fp.f);
    }

    // Make sure a log exists right away and record both requested/effective paths.
    inline void EnsureLogFile() noexcept {
        {
            FileGuard fp = open_log_for_append();
            if (!fp) return; // as best-effort; nothing else we can do
        }
        LogLine("CrashHandler initialized log");
        Logf("Log path requested: %s", g_log_path ? g_log_path : "(null)");
        Logf("Log path effective: %s", g_effective_path);
    }

    // ---------- Terminate & Signal handlers --------------------------------------
    inline void TerminateHandler() noexcept {
        try {
            if (auto ex = std::current_exception()) {
                try {
                    std::rethrow_exception(ex);
                }
                catch (const std::exception& e) {
                    Logf("std::terminate due to unhandled std::exception: %s", e.what());
                }
                catch (...) {
                    LogLine("std::terminate due to unknown (non-std::exception) exception");
                }
            }
            else {
                LogLine("std::terminate with no active exception");
            }
        }
        catch (...) {
            // swallow
        }

        if (g_prev_terminate) g_prev_terminate(); // may abort()
        std::abort();
    }

    // NOTE: This uses stdio in a signal handler (not async-signal-safe).
    // It's intentionally simple/minimal for portability and tiny header goals.
    inline void SignalHandler(int sig) {
        Logf("Caught %s (%d). Aborting.", SigName(sig), sig);

        void (*prev)(int) = SIG_DFL;
        switch (sig) {
        case SIGABRT: prev = g_prev_sigabrt; break;
        case SIGSEGV: prev = g_prev_sigsegv; break;
        case SIGILL:  prev = g_prev_sigill;  break;
        case SIGFPE:  prev = g_prev_sigfpe;  break;
        case SIGTERM: prev = g_prev_sigterm; break;
        default: break;
        }
        if (prev && prev != SIG_DFL && prev != SIG_IGN) prev(sig);

        std::abort();
    }

    // ---------- Public API --------------------------------------------------------
    inline void SetLogPath(const char* path) noexcept {
        // Copy into internal storage to avoid dangling pointers.
        const char* use = (path && *path) ? path : ".log";
        std::snprintf(g_path_storage, sizeof(g_path_storage), "%s", use);
        g_log_path = g_path_storage;
        // Reset effective path to requested until next open
        std::snprintf(g_effective_path, sizeof(g_effective_path), "%s", g_log_path);
    }

    inline const char* GetRequestedPath()  noexcept { return g_log_path; }
    inline const char* GetEffectivePath()  noexcept { return g_effective_path; }

    inline void Install() noexcept {
        // Install handlers first
        g_prev_terminate = std::set_terminate(TerminateHandler);
        g_prev_sigabrt = std::signal(SIGABRT, SignalHandler);
        g_prev_sigsegv = std::signal(SIGSEGV, SignalHandler);
        g_prev_sigill = std::signal(SIGILL, SignalHandler);
        g_prev_sigfpe = std::signal(SIGFPE, SignalHandler);
        g_prev_sigterm = std::signal(SIGTERM, SignalHandler);

        // Ensure the log file exists *now* and record paths
        EnsureLogFile();
        std::cout << "CrashHandler installed" << std::endl;
        LogLine("CrashHandler installed");
    }

    inline void Uninstall() noexcept {
        if (g_prev_terminate) std::set_terminate(g_prev_terminate);
        std::signal(SIGABRT, g_prev_sigabrt);
        std::signal(SIGSEGV, g_prev_sigsegv);
        std::signal(SIGILL, g_prev_sigill);
        std::signal(SIGFPE, g_prev_sigfpe);
        std::signal(SIGTERM, g_prev_sigterm);
        LogLine("CrashHandler uninstalled");
    }

    // Manual log helper
    inline void Log(const char* msg) noexcept { LogLine(msg); }

    // Run callable; logs exceptions; returns 0 on success, 1 on failure.
    template <typename T>
    inline int RunProtected(T&& fn) noexcept {
        try {
            std::forward<T>(fn)();
            return 0;
        }
        catch (const std::exception& e) {
            Logf("Unhandled exception: %s", e.what());
        }
        catch (...) {
            LogLine("Unhandled non-std::exception");
        }
        return 1;
    }

    // ---------- Helpers for game states ------------------------------------------
    template <typename T>
    inline void Safe(const char* file, const char* state, T&& fn) {
        if (RunProtected(std::forward<T>(fn)) != 0) {
            Logf("%s::%s failed (caught exception)", file, state);
            GSM_SetNextState(GameState::GS_QUIT); // safe shutdown
        }
    }

    //inline void DrainAllGLErrors() {
    //    GLenum e;
    //    while ((e = glGetError()) != GL_NO_ERROR) { (void)e; }
    //}

    //// At scope entry: clear prior errors. At exit: log only new ones.
    //struct GLErrorScope {
    //    const char* where;
    //    explicit GLErrorScope(const char* w) : where(w) { DrainAllGLErrors(); }
    //    ~GLErrorScope() {
    //        GLenum e; bool any = false;
    //        while ((e = glGetError()) != GL_NO_ERROR) {
    //            any = true;
    //            CrashHandler::Logf("OpenGL error 0x%X in %s", (unsigned)e, where);
    //        }
    //        (void)any;
    //    }
    //};

    //inline void LogIfGLError(const char* state, bool drain_previous = false, bool log_all = true) {
    //    if (drain_previous) {
    //        DrainAllGLErrors();
    //        return;
    //    }

    //    GLenum e;
    //    bool any = false;

    //    while ((e = glGetError()) != GL_NO_ERROR) {
    //        any = true;
    //        Logf("OpenGL error 0x%X in %s", static_cast<unsigned>(e), state ? state : "(unknown)");
    //        if (!log_all) break;
    //    }
    //    (void)any;
    //}

    inline void LogIfZeroTexture(GLuint tex, const char* path) {
        if (!tex) Logf("Failed to load texture: %s", path);
    }

} // namespace CrashHandler

/* -------------------------- Usage Example ------------------------------------

#include "CrashHandler.h"

int main() {
    CrashHandler::SetLogPath("rockandghoul-crash.log"); // optional
    CrashHandler::Install();                             // creates the log immediately
    CrashHandler::Log("App starting up");               // sanity line you can search for

    // ... your program ...
    // e.g., return CrashHandler::RunProtected([&]{ /* body * / });

    CrashHandler::Uninstall();
    return 0;
}

Notes:
- On failure to open the requested path, the logger falls back to a writable
  temp location (LOCALAPPDATA/TEMP on Windows; TMPDIR/HOME or /tmp on POSIX).
- The *effective* path used is recorded via GetEffectivePath() and logged on init.
- This is intentionally simplified; for production signal safety, switch
  to async-signal-safe writes (e.g., ::write) and add platform backtraces.
------------------------------------------------------------------------------- */
