
#ifndef MAIA_ENGINE_H
#define MAIA_ENGINE_H

#include "types.h"
#include "board.h"
#include <string>
#include <vector>

using namespace std;

class MaiaEngine {
public:
    MaiaEngine();
    ~MaiaEngine();


    bool start(const string& lc0Path, const string& weightsPath);


    void stop();


    bool isRunning() const;


    string getBestMove(const string& fen);


    Move getBestMoveForBoard(Board& board);

private:
    int toEngineFd;     
    int fromEngineFd;   
    int childPid;       
    bool running;       


    void sendCommand(const string& cmd);


    string readLine();


    string waitForResponse(const string& prefix, int timeoutMs = 10000);
};


string findLc0Path();

#endif 
