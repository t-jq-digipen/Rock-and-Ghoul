/* ========================================================
 * @file	Crash Handler
 * @author	Manfred Sim
 * @date	29 SEP 2025
 *
 * @brief	Crash Handler for writing crash logs to a text file
 *
 * This header file contains the declaration of functions
 * used by the crash handler. The crash handler uses safe
 * wrappers like "try-catch" with "exception safety" to write
 * a log to the text file should the program crash.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

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

    // RAII Wrapper for FILE*
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
        // Guarantees fclose() on scope exit
        ~FileGuard() { if (f) std::fclose(f); }
        explicit operator bool() const noexcept { return f != nullptr; }
    };

    // ---------- Internal state ----------------------------------------------------
    // Terminate handlers
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
    // Returns a short string for a signal number (e.g. SIGABRT)
    inline const char* SigName(int sig) noexcept {
        switch (sig) {
        case SIGABRT: return "SIGABRT";
        case SIGSEGV: return "SIGSEGV";
        case SIGILL:  return "SIGILL";
        case SIGFPE:  return "SIGFPE";
        case SIGTERM: return "SIGTERM";
        default:      return "SIGNAL";  // For unknown signals
        }
    }

    // Writes timestamp into buffer in format "YYYY-MM-DD HH:MM:SS"
    inline void Timestamp(char* buf, std::size_t n) noexcept {
        if (!buf || n == 0) return;
        std::time_t t = std::time(nullptr);
        std::tm* lt = std::localtime(&t);
        if (lt) std::snprintf(buf, n, "%04d-%02d-%02d %02d:%02d:%02d",
            lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
            lt->tm_hour, lt->tm_min, lt->tm_sec);
        else    std::snprintf(buf, n, "unknown-time");
    }

    // Returns the filename from a path
    inline const char* BasenameOf(const char* path) noexcept {
        if (!path || !*path) return ".log";
        const char* b1 = std::strrchr(path, '/');
        const char* b2 = std::strrchr(path, '\\');
        const char* b = (b1 && b2) ? (b1 > b2 ? b1 : b2) : (b1 ? b1 : b2);
        return b ? (b + 1) : path;
    }

    // Join directory and filename into out (size out_n).
    inline void JoinPath(char* out, std::size_t out_n, const char* dir, const char* fname) noexcept {
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
    inline FileGuard OpenLogForAppend() noexcept {
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
        const char* base_name = BasenameOf(g_log_path);

        for (const char* key : cand_envs) {
            const char* dir = std::getenv(key);
            if (dir && *dir) {
                char candidate[1024];
                JoinPath(candidate, sizeof(candidate), dir, base_name);
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

    // Appends a single line (with timestamp) to the log file
    inline void LogLine(const char* line) noexcept {
        FileGuard fp = OpenLogForAppend();
        if (!fp) return;
        char ts[32]; Timestamp(ts, sizeof ts);
        std::fprintf(fp.f, "[%s] %s\n", ts, line ? line : "(null)");
        std::fflush(fp.f);
    }

    // Appends a single formatted line (with timestamp) to the log file
    template <typename... Args>
    inline void Logf(const char* fmt, Args... args) noexcept {
        FileGuard fp = OpenLogForAppend();
        if (!fp) return;
        char ts[32]; Timestamp(ts, sizeof ts);
        std::fprintf(fp.f, "[%s] ", ts);
        std::fprintf(fp.f, fmt ? fmt : "(null)", args...);
        std::fputc('\n', fp.f);
        std::fflush(fp.f);
    }

    // Ensures the log file exists by opening and closing it once, then writes
    // startup breadcrumbs including both requested and effective paths
    inline void EnsureLogFile() noexcept {
        {
            FileGuard fp = OpenLogForAppend();
            if (!fp) return;
        }
        LogLine("CrashHandler initialized log");
        Logf("Log path requested: %s", g_log_path ? g_log_path : "(null)");
        Logf("Log path effective: %s", g_effective_path);
    }

    // ---------- Terminate & Signal handlers --------------------------------------
    // Custom std::terminate handler that logs details about any exception thrown and
    // then aborts the program
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

    // Logs the signal name then aborts the program
    inline void SignalHandler(int sig) {
        Logf("Caught %s SIGNAL (%d). Aborting.", SigName(sig), sig);

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
    // Sets the desired log file path
    inline void SetLogPath(const char* path) noexcept {
        // Copy into internal storage to avoid dangling pointers.
        const char* use = (path && *path) ? path : ".log";
        std::snprintf(g_path_storage, sizeof(g_path_storage), "%s", use);
        g_log_path = g_path_storage;
        // Reset effective path to requested until next open
        std::snprintf(g_effective_path, sizeof(g_effective_path), "%s", g_log_path);
    }

    // Returns the last requested log path string
    inline const char* GetRequestedPath()  noexcept { return g_log_path; }

    // Returns the actual path used on the last successful opening of the file
    inline const char* GetEffectivePath()  noexcept { return g_effective_path; }

    // Install the crash handler (upon program startup)
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

    // Uninstall the crash handler (upon program shutdown)
    inline void Uninstall() noexcept {
        if (g_prev_terminate) std::set_terminate(g_prev_terminate);
        std::signal(SIGABRT, g_prev_sigabrt);
        std::signal(SIGSEGV, g_prev_sigsegv);
        std::signal(SIGILL, g_prev_sigill);
        std::signal(SIGFPE, g_prev_sigfpe);
        std::signal(SIGTERM, g_prev_sigterm);
        LogLine("CrashHandler uninstalled");
    }

    // Convenience helper to write a single line to the log file
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
    // Wraps a callable function with RunProtected()
    template <typename T>
    inline void Safe(const char* file, const char* state, T&& fn) {
        if (RunProtected(std::forward<T>(fn)) != 0) {
            Logf("%s::%s failed (caught exception)", file, state); 
            GSM_SetNextState(GameState::GS_QUIT); // safe shutdown
        }
    }

    // Logs a message if texture object ID is zero (failed load)
    inline void LogIfZeroTexture(GLuint tex, const char* path) {
        if (!tex) Logf("Failed to load texture: %s", path);
    }

} // namespace CrashHandler
