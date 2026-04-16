
#include "maia_engine.h"
#include "movegen.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <cstdio>
#include <stdio.h>

using namespace std;


#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>
    #include <poll.h>
    #include <fcntl.h>
#endif


#ifdef _WIN32
    static HANDLE hChildStdinWr = NULL;
    static HANDLE hChildStdoutRd = NULL;
    static PROCESS_INFORMATION piProcInfo;
#endif


MaiaEngine::MaiaEngine()
    : toEngineFd(-1), fromEngineFd(-1),
#ifdef _WIN32
      childPid(0),
#else
      childPid(-1),
#endif
      running(false) {
}

MaiaEngine::~MaiaEngine() {
    stop();
}



#ifdef _WIN32

bool MaiaEngine::start(const string& lc0Path, const string& weightsPath) {
    if (running) return true;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStdinRd = NULL;
    HANDLE hChildStdoutWr = NULL;


    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
        cerr << "  [Maia] Failed to create stdout pipe." << endl;
        return false;
    }
    SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);


    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) {
        cerr << "  [Maia] Failed to create stdin pipe." << endl;
        CloseHandle(hChildStdoutRd);
        CloseHandle(hChildStdoutWr);
        return false;
    }
    SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);


    STARTUPINFOA siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStdoutWr;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.hStdInput = hChildStdinRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    siStartInfo.wShowWindow = SW_HIDE;


    string cmdLine = lc0Path + " --weights=" + weightsPath;
    char cmdLineBuf[1024];
    strncpy(cmdLineBuf, cmdLine.c_str(), sizeof(cmdLineBuf) - 1);
    cmdLineBuf[sizeof(cmdLineBuf) - 1] = '\0';


    BOOL success = CreateProcessA(
        NULL,           
        cmdLineBuf,     
        NULL,           
        NULL,           
        TRUE,           
        CREATE_NO_WINDOW, 
        NULL,           
        NULL,           
        &siStartInfo,   
        &piProcInfo     
    );


    CloseHandle(hChildStdoutWr);
    CloseHandle(hChildStdinRd);

    if (!success) {
        cerr << "  [Maia] Failed to create process. Error: " << GetLastError() << endl;
        CloseHandle(hChildStdoutRd);
        CloseHandle(hChildStdinWr);
        return false;
    }

    running = true;


    sendCommand("uci");
    string response = waitForResponse("uciok", 15000);
    if (response.empty()) {
        cerr << "  [Maia] lc0 did not respond with 'uciok'. Engine may not be working." << endl;
        stop();
        return false;
    }

    sendCommand("isready");
    response = waitForResponse("readyok", 15000);
    if (response.empty()) {
        cerr << "  [Maia] lc0 did not respond with 'readyok'." << endl;
        stop();
        return false;
    }

    return true;
}

#else

bool MaiaEngine::start(const string& lc0Path, const string& weightsPath) {
    if (running) return true;

    int parentToChild[2];  
    int childToParent[2];

    if (pipe(parentToChild) != 0 || pipe(childToParent) != 0) {
        cerr << "  [Maia] Failed to create pipes." << endl;
        return false;
    }

    childPid = fork();

    if (childPid < 0) {
        cerr << "  [Maia] Failed to fork process." << endl;
        return false;
    }

    if (childPid == 0) {

        dup2(parentToChild[0], STDIN_FILENO);
        dup2(childToParent[1], STDOUT_FILENO);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        close(parentToChild[1]);
        close(childToParent[0]);

        string weightsArg = "--weights=" + weightsPath;
        execlp(lc0Path.c_str(), lc0Path.c_str(), weightsArg.c_str(), (char*)nullptr);
        _exit(1);
    }


    close(parentToChild[0]);
    close(childToParent[1]);

    toEngineFd = parentToChild[1];
    fromEngineFd = childToParent[0];

    int flags = fcntl(fromEngineFd, F_GETFL, 0);
    fcntl(fromEngineFd, F_SETFL, flags | O_NONBLOCK);

    running = true;


    sendCommand("uci");
    string response = waitForResponse("uciok", 15000);
    if (response.empty()) {
        cerr << "  [Maia] lc0 did not respond with 'uciok'. Engine may not be working." << endl;
        stop();
        return false;
    }

    sendCommand("isready");
    response = waitForResponse("readyok", 15000);
    if (response.empty()) {
        cerr << "  [Maia] lc0 did not respond with 'readyok'." << endl;
        stop();
        return false;
    }

    return true;
}
#endif


void MaiaEngine::stop() {
    if (!running) return;

    sendCommand("quit");

#ifdef _WIN32

    if (WaitForSingleObject(piProcInfo.hProcess, 500) != WAIT_OBJECT_0) {
        TerminateProcess(piProcInfo.hProcess, 0);
    }
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    if (hChildStdinWr) { CloseHandle(hChildStdinWr); hChildStdinWr = NULL; }
    if (hChildStdoutRd) { CloseHandle(hChildStdoutRd); hChildStdoutRd = NULL; }
#else
    if (toEngineFd >= 0) { close(toEngineFd); toEngineFd = -1; }
    if (fromEngineFd >= 0) { close(fromEngineFd); fromEngineFd = -1; }

    if (childPid > 0) {
        int status;
        usleep(100000);  // 100ms grace
        if (waitpid(childPid, &status, WNOHANG) == 0) {
            kill(childPid, SIGKILL);
            waitpid(childPid, &status, 0);
        }
        childPid = -1;
    }
#endif

    running = false;
}


bool MaiaEngine::isRunning() const {
    return running;
}


void MaiaEngine::sendCommand(const string& cmd) {
    if (!running) return;
    string line = cmd + "\n";

#ifdef _WIN32
    if (hChildStdinWr) {
        DWORD bytesWritten;
        WriteFile(hChildStdinWr, line.c_str(), (DWORD)line.size(), &bytesWritten, NULL);
    }
#else
    if (toEngineFd >= 0) {
        write(toEngineFd, line.c_str(), line.size());
    }
#endif
}


string MaiaEngine::readLine() {
    string line;
    char c;

#ifdef _WIN32
    if (!hChildStdoutRd) return "";
    DWORD bytesRead;
    while (true) {
        BOOL success = ReadFile(hChildStdoutRd, &c, 1, &bytesRead, NULL);
        if (!success || bytesRead == 0) break;
        if (c == '\n') break;
        if (c != '\r') line += c;
    }
#else
    while (true) {
        ssize_t n = read(fromEngineFd, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c != '\r') line += c;
    }
#endif

    return line;
}


string MaiaEngine::waitForResponse(const string& prefix, int timeoutMs) {
    if (!running) return "";

    int elapsed = 0;
    const int pollInterval = 50;  // 50ms poll interval
    string buffer;

#ifdef _WIN32
    if (!hChildStdoutRd) return "";

    while (elapsed < timeoutMs) {
        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
            char buf[1024];
            DWORD bytesRead;
            DWORD toRead = (bytesAvailable < sizeof(buf) - 1) ? bytesAvailable : sizeof(buf) - 1;
            if (ReadFile(hChildStdoutRd, buf, toRead, &bytesRead, NULL) && bytesRead > 0) {
                buf[bytesRead] = '\0';
                buffer += buf;

                size_t pos;
                while ((pos = buffer.find('\n')) != string::npos) {
                    string line = buffer.substr(0, pos);
                    buffer = buffer.substr(pos + 1);

                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }

                    if (line.find(prefix) != string::npos) {
                        return line;
                    }
                }
            }
        } else {
            Sleep(pollInterval);
            elapsed += pollInterval;
        }
    }
#else
    struct pollfd pfd;
    pfd.fd = fromEngineFd;
    pfd.events = POLLIN;

    while (elapsed < timeoutMs) {
        int ret = poll(&pfd, 1, pollInterval);
        if (ret > 0 && (pfd.revents & POLLIN)) {
            char buf[1024];
            ssize_t n = read(fromEngineFd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                buffer += buf;

                size_t pos;
                while ((pos = buffer.find('\n')) != string::npos) {
                    string line = buffer.substr(0, pos);
                    buffer = buffer.substr(pos + 1);

                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }

                    if (line.find(prefix) == 0 || line.find(prefix) != string::npos) {
                        return line;
                    }
                }
            }
        }
        elapsed += pollInterval;
    }
#endif

    return "";  // Timeout
}


string MaiaEngine::getBestMove(const string& fen) {
    if (!running) return "";

    sendCommand("position fen " + fen);
    sendCommand("go nodes 1");

    string response = waitForResponse("bestmove", 15000);

    if (response.empty()) return "";

    size_t pos = response.find("bestmove ");
    if (pos == string::npos) return "";

    string moveStr = response.substr(pos + 9);

    size_t space = moveStr.find(' ');
    if (space != string::npos) {
        moveStr = moveStr.substr(0, space);
    }

    return moveStr;
}


Move MaiaEngine::getBestMoveForBoard(Board& board) {
    string fen = board.toFEN();
    string moveStr = getBestMove(fen);

    if (moveStr.empty() || moveStr.length() < 4) return Move();

    vector<Move> legalMoves = generateLegalMoves(board);

    int fromSq = stringToSquare(moveStr.substr(0, 2));
    int toSq   = stringToSquare(moveStr.substr(2, 2));

    if (fromSq < 0 || toSq < 0) return Move();

    for (const Move& m : legalMoves) {
        if (m.from == fromSq && m.to == toSq) {
            return m;
        }
    }

    return Move();  // No match found
}


string findLc0Path() {
#ifdef _WIN32
    // Windows: check common locations
    const char* paths[] = {
        "lc0.exe",
        "../lc0.exe",
        "C:\\Program Files\\lc0\\lc0.exe",
        "C:\\lc0\\lc0.exe",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; i++) {
        string path = paths[i];
        // Check if file exists
        DWORD attrib = GetFileAttributesA(path.c_str());
        if (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
            return path;
        }
    }


    {
        char foundPath[MAX_PATH];
        DWORD result = SearchPathA(NULL, "lc0.exe", NULL, MAX_PATH, foundPath, NULL);
        if (result > 0 && result < MAX_PATH) {
            return string(foundPath);
        }
    }
#else

    const char* paths[] = {
        "lc0",
        "/opt/homebrew/bin/lc0",
        "/usr/local/bin/lc0",
        "/usr/bin/lc0",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; i++) {
        string path = paths[i];

        if (path[0] == '/') {
            if (access(path.c_str(), X_OK) == 0) {
                return path;
            }
        } else {
            string cmd = "which " + path + " 2>/dev/null";
            FILE* fp = popen(cmd.c_str(), "r");
            if (fp) {
                char buf[256];
                if (fgets(buf, sizeof(buf), fp)) {
                    string result(buf);
                    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
                        result.pop_back();
                    }
                    pclose(fp);
                    if (!result.empty()) return result;
                } else {
                    pclose(fp);
                }
            }
        }
    }
#endif

    return "";  
}
