
#include "types.h"
#include "board.h"
#include "movegen.h"
#include "evaluator.h"
#include "ai_engine.h"
#include "ids_engine.h"
#include "maia_engine.h"
#include "protocol.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;




void printTitle() {
    cout << endl;
    cout << "  ======================================" << endl;
    cout << "  |                                    |" << endl;
    cout << "  |      CHESS AI ENGINE  (C++)        |" << endl;
    cout << "  |  Minimax + Alpha-Beta Pruning      |" << endl;
    cout << "  |                                    |" << endl;
    cout << "  |  DSA Project: Game Tree Search     |" << endl;
    cout << "  |                                    |" << endl;
    cout << "  ======================================" << endl;
    cout << endl;
}


void printHelp() {
    cout << endl;
    cout << "  --- HOW TO PLAY ---" << endl;
    cout << "  Enter moves in 'from-to' format:" << endl;
    cout << "    e2e4  = move piece from e2 to e4" << endl;
    cout << "    g1f3  = move knight from g1 to f3" << endl;
    cout << "    e1g1  = castle kingside (move king e1 to g1)" << endl;
    cout << endl;
    cout << "  Pawns auto-promote to Queen." << endl;
    cout << endl;
    cout << "  Commands:" << endl;
    cout << "    help   = show this help" << endl;
    cout << "    moves  = show all legal moves" << endl;
    cout << "    eval   = show position evaluation" << endl;
    cout << "    quit   = exit the game" << endl;
    cout << endl;
}


Move parseUserMove(const string& input, const vector<Move>& legalMoves) {
    if (input.length() < 4) return Move();  // Too short


    string fromStr = input.substr(0, 2);
    string toStr   = input.substr(2, 2);

    int fromSq = stringToSquare(fromStr);
    int toSq   = stringToSquare(toStr);

    if (fromSq < 0 || toSq < 0) return Move();  


    for (const Move& move : legalMoves) {
        if (move.from == fromSq && move.to == toSq) {
            return move;  
        }
    }

    return Move();  
}


void showCapturedPieces(const Board& board) {

    int startPieces[12] = {8, 2, 2, 2, 1, 1, 8, 2, 2, 2, 1, 1};  
    int currentPieces[12] = {0};

    for (int sq = 0; sq < 64; sq++) {
        Piece p = board.board[sq];
        if (p != NO_PIECE) {
            currentPieces[p]++;
        }
    }

    cout << "  Captured: ";


    cout << "White lost: ";
    bool any = false;
    for (int p = 0; p < 6; p++) {
        int lost = startPieces[p] - currentPieces[p];
        for (int i = 0; i < lost; i++) {
            cout << PIECE_LETTERS[p] << " ";
            any = true;
        }
    }
    if (!any) cout << "none";

    cout << " | Black lost: ";
    any = false;
    for (int p = 6; p < 12; p++) {
        int lost = startPieces[p] - currentPieces[p];
        for (int i = 0; i < lost; i++) {
            cout << PIECE_LETTERS[p] << " ";
            any = true;
        }
    }
    if (!any) cout << "none";
    cout << endl;
}


void showMoveHistory(const Board& board) {
    if (board.moveHistory.empty()) return;

    cout << "  Move history: ";
    for (int i = 0; i < (int)board.moveHistory.size(); i++) {
        if (i % 2 == 0) {
            cout << (i / 2 + 1) << ".";
        }
        cout << board.moveHistory[i].toString() << " ";
    }
    cout << endl;
}


int showMenu(bool maiaAvailable) {
    cout << "  --- MAIN MENU ---" << endl;
    cout << "  1. Play as White (you go first)" << endl;
    cout << "  2. Play as Black (AI goes first)" << endl;
    cout << "  3. Play Local 1v1 (Human vs Human)" << endl;
    cout << "  4. Set AI difficulty (search depth)" << endl;
    cout << "  5. Quit" << endl;
    if (maiaAvailable) {
        cout << "  [Maia 1200 ELO available]" << endl;
    }
    cout << endl;
    cout << "  Enter choice (1-5): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();  
    return choice;
}


void playGame(Color humanColor, int aiDepth, MaiaEngine* maia = nullptr, bool useMaia = false) {
    Board board;  
    Color aiColor = oppositeColor(humanColor);

    cout << endl;
    cout << "  Game started! You are playing as "
              << ((humanColor == WHITE) ? "White" : "Black") << "." << endl;
    if (useMaia && maia && maia->isRunning()) {
        cout << "  AI mode: Maia 1200 ELO (human-like)" << endl;
    } else {
        cout << "  AI mode: Minimax+IDS (depth " << aiDepth << ")" << endl;
    }
    cout << "  Type 'help' for commands." << endl;


    while (true) {

        vector<Move> legalMoves = generateLegalMoves(board);


        board.printBoard();
        showCapturedPieces(board);


        if (legalMoves.empty()) {
            if (board.isInCheck(board.sideToMove)) {
                cout << endl;
                cout << "  ==============================" << endl;
                cout << "  |       CHECKMATE!           |" << endl;
                cout << "  |  "
                          << ((board.sideToMove == WHITE) ? "Black" : "White")
                          << " wins!                |" << endl;
                cout << "  ==============================" << endl;
            } else {
                cout << endl;
                cout << "  ==============================" << endl;
                cout << "  |       STALEMATE!           |" << endl;
                cout << "  |    Game is a draw.         |" << endl;
                cout << "  ==============================" << endl;
            }
            break;
        }

        if (board.isDraw()) {
            cout << endl;
            cout << "  ==============================" << endl;
            cout << "  |     DRAW!                  |" << endl;
            cout << "  |  (50-move rule or          |" << endl;
            cout << "  |   insufficient material)   |" << endl;
            cout << "  ==============================" << endl;
            break;
        }


        if (board.sideToMove == aiColor) {
            cout << endl;


            if (useMaia && maia && maia->isRunning()) {
                cout << "  Maia (1200 ELO) is thinking..." << endl;
                Move maiaMove = maia->getBestMoveForBoard(board);

                if (maiaMove.isValid()) {
                    cout << endl;
                    cout << "  +-----------------------------------+" << endl;
                    cout << "  | Maia plays: " << left << setw(22)
                              << maiaMove.toString() << "|" << endl;
                    cout << "  | Engine:     Maia 1200 ELO        |" << endl;
                    cout << "  +-----------------------------------+" << endl;

                    board.makeMove(maiaMove);
                } else {
                    cout << "  Maia failed, falling back to IDS..." << endl;
                    goto ids_fallback;
                }
            } else {
                ids_fallback:
                cout << "  AI is thinking (Iterative Deepening)..." << endl;


                IDSResult result = findBestMoveIDS(board, aiDepth);

                if (result.bestMove.isValid()) {
                    cout << endl;
                    cout << "  +-----------------------------------+" << endl;
                    cout << "  | AI plays: " << left << setw(24)
                              << result.bestMove.toString() << "|" << endl;
                    cout << "  | Score:    " << left << setw(24)
                              << (result.score / 100.0) << "|" << endl;
                    cout << "  | Nodes:    " << left << setw(24)
                              << result.totalNodes << "|" << endl;
                    cout << "  | Time:     " << left << setw(21)
                              << result.totalTimeMs << " ms |" << endl;
                    cout << "  +-----------------------------------+" << endl;

                    board.makeMove(result.bestMove);
                } else {
                    cout << "  AI has no valid moves!" << endl;
                    break;
                }
            }
            continue;  
        }


        cout << endl;
        cout << "  Calculating hints..." << flush;
        vector<EvaluatedMove> hints = findTopMoves(board, aiDepth, 3);
        cout << "\r                           \r"; 


        cout << "  --- Minimax Top " << hints.size() << " Suggested Moves ---" << endl;
        for (int i = 0; i < (int)hints.size(); ++i) {
            cout << "    " << (i + 1) << ". " << hints[i].move.toString() 
                      << " (Score: " << (hints[i].score / 100.0) << ")" << endl;
        }


        if (maia && maia->isRunning()) {
            Move maiaHint = maia->getBestMoveForBoard(board);
            if (maiaHint.isValid()) {
                cout << "  --- Maia 1200 Suggestion ---" << endl;
                cout << "    -> " << maiaHint.toString()
                          << " (human-like move)" << endl;
            }
        }

        cout << "  Your move (" << ((humanColor == WHITE) ? "White" : "Black")
                  << "): ";

        string input;
        getline(cin, input);


        input.erase(remove(input.begin(), input.end(), ' '), input.end());
        input.erase(remove(input.begin(), input.end(), '\r'), input.end());
        input.erase(remove(input.begin(), input.end(), '\n'), input.end());


        transform(input.begin(), input.end(), input.begin(), ::tolower);


        if (input == "quit" || input == "exit" || input == "q") {
            cout << "  Thanks for playing! Goodbye." << endl;
            break;
        }

        if (input == "help" || input == "h") {
            printHelp();
            continue;
        }

        if (input == "moves" || input == "m") {
            cout << "  Legal moves (" << legalMoves.size() << "): ";
            for (const Move& m : legalMoves) {
                cout << m.toString() << " ";
            }
            cout << endl;
            continue;
        }

        if (input == "eval" || input == "e") {
            int score = evaluate(board);
            cout << "  Position evaluation: " << (score / 100.0)
                      << " (positive = White advantage)" << endl;
            continue;
        }

        if (input == "history") {
            showMoveHistory(board);
            continue;
        }


        Move move = parseUserMove(input, legalMoves);

        if (!move.isValid()) {
            cout << "  Invalid move! Try again." << endl;
            cout << "  Format: e2e4 (from square, to square)" << endl;
            cout << "  Type 'moves' to see all legal moves." << endl;
            continue;
        }


        board.makeMove(move);
    }


    cout << endl;
    showMoveHistory(board);
    cout << endl;
}


void playLocalGame(int aiDepth, MaiaEngine* maia = nullptr) {
    Board board;  
    bool maiaAvailable = (maia && maia->isRunning());

    cout << endl;
    cout << "  Game started! Local 1v1 Mode." << endl;
    cout << "  Hint depth: " << aiDepth << endl;
    if (maiaAvailable) {
        cout << "  Maia 1200 hints: ENABLED" << endl;
    }
    cout << "  Type 'help' for commands." << endl;


    while (true) {

        vector<Move> legalMoves = generateLegalMoves(board);


        board.printBoard();
        showCapturedPieces(board);


        if (legalMoves.empty()) {
            if (board.isInCheck(board.sideToMove)) {
                cout << endl;
                cout << "  ==============================" << endl;
                cout << "  |       CHECKMATE!           |" << endl;
                cout << "  |  "
                          << ((board.sideToMove == WHITE) ? "Black" : "White")
                          << " wins!                |" << endl;
                cout << "  ==============================" << endl;
            } else {
                cout << endl;
                cout << "  ==============================" << endl;
                cout << "  |       STALEMATE!           |" << endl;
                cout << "  |    Game is a draw.         |" << endl;
                cout << "  ==============================" << endl;
            }
            break;
        }

        if (board.isDraw()) {
            cout << endl;
            cout << "  ==============================" << endl;
            cout << "  |     DRAW!                  |" << endl;
            cout << "  |  (50-move rule or          |" << endl;
            cout << "  |   insufficient material)   |" << endl;
            cout << "  ==============================" << endl;
            break;
        }

        cout << endl;
        cout << "  Calculating hints..." << flush;
        vector<EvaluatedMove> hints = findTopMoves(board, aiDepth, 3);
        cout << "\r                           \r";


        cout << "  --- Minimax Top " << hints.size() << " Suggested Moves ---" << endl;
        for (int i = 0; i < (int)hints.size(); ++i) {
            cout << "    " << (i + 1) << ". " << hints[i].move.toString()
                      << " (Score: " << (hints[i].score / 100.0) << ")" << endl;
        }


        if (maiaAvailable) {
            Move maiaHint = maia->getBestMoveForBoard(board);
            if (maiaHint.isValid()) {
                cout << "  --- Maia 1200 Suggestion ---" << endl;
                cout << "    -> " << maiaHint.toString()
                          << " (human-like move)" << endl;
            }
        }

        cout << "  Your move (" << ((board.sideToMove == WHITE) ? "White" : "Black")
                  << "): ";

        string input;
        getline(cin, input);


        input.erase(remove(input.begin(), input.end(), ' '), input.end());
        input.erase(remove(input.begin(), input.end(), '\r'), input.end());
        input.erase(remove(input.begin(), input.end(), '\n'), input.end());

  
        transform(input.begin(), input.end(), input.begin(), ::tolower);

  
        if (input == "quit" || input == "exit" || input == "q") {
            cout << "  Thanks for playing! Goodbye." << endl;
            break;
        }

        if (input == "help" || input == "h") {
            printHelp();
            continue;
        }

        if (input == "moves" || input == "m") {
            cout << "  Legal moves (" << legalMoves.size() << "): ";
            for (const Move& m : legalMoves) {
                cout << m.toString() << " ";
            }
            cout << endl;
            continue;
        }

        if (input == "eval" || input == "e") {
            int score = evaluate(board);
            cout << "  Position evaluation: " << (score / 100.0)
                      << " (positive = White advantage)" << endl;
            continue;
        }

        if (input == "history") {
            showMoveHistory(board);
            continue;
        }

        Move move = parseUserMove(input, legalMoves);

        if (!move.isValid()) {
            cout << "  Invalid move! Try again." << endl;
            cout << "  Format: e2e4 (from square, to square)" << endl;
            cout << "  Type 'moves' to see all legal moves." << endl;
            continue;
        }


        board.makeMove(move);
    }


    cout << endl;
    showMoveHistory(board);
    cout << endl;
}


int main(int argc, char* argv[]) {

    if (argc > 1 && string(argv[1]) == "--protocol") {
        runProtocolMode();
        return 0;
    }

    printTitle();

    int aiDepth = 3;  


    MaiaEngine maia;
    bool maiaAvailable = false;
    string lc0Path = findLc0Path();
    if (!lc0Path.empty()) {
        cout << "  Found lc0 at: " << lc0Path << endl;
        cout << "  Starting Maia 1200 ELO engine..." << endl;
        maiaAvailable = maia.start(lc0Path, "maia-1200.pb.gz");
        if (maiaAvailable) {
            cout << "  Maia 1200 ELO engine ready!" << endl;
        } else {
            cout << "  Warning: Could not start Maia. Continuing without it." << endl;
        }
    } else {
        cout << "  Note: lc0 not found. Install with 'brew install lc0' for Maia support." << endl;
    }
    cout << endl;

    while (true) {
        int choice = showMenu(maiaAvailable);

        switch (choice) {
            case 1:
            case 2: {

                bool useMaia = false;
                if (maiaAvailable) {
                    cout << endl;
                    cout << "  Choose your AI opponent:" << endl;
                    cout << "    1. Minimax + IDS (depth " << aiDepth << ")" << endl;
                    cout << "    2. Maia 1200 ELO (human-like)" << endl;
                    cout << "  Choice: ";
                    int aiChoice = 0;
                    cin >> aiChoice;
                    cin.ignore();
                    if (aiChoice == 2) useMaia = true;
                }

                Color humanColor = (choice == 1) ? WHITE : BLACK;
                playGame(humanColor, aiDepth, &maia, useMaia);
                break;
            }

            case 3:

                playLocalGame(aiDepth, &maia);
                break;

            case 4: {

                cout << endl;
                cout << "  Current depth: " << aiDepth << endl;
                cout << "  Enter new depth (1-6):" << endl;
                cout << "    1-2 = Easy   (instant, weak)" << endl;
                cout << "    3   = Medium (fast, decent)" << endl;
                cout << "    4   = Hard   (a few seconds, strong)" << endl;
                cout << "    5-6 = Expert (may take 10-60 seconds)" << endl;
                cout << "  Depth: ";
                int newDepth;
                cin >> newDepth;
                cin.ignore();
                if (newDepth >= 1 && newDepth <= 6) {
                    aiDepth = newDepth;
                    cout << "  AI depth set to " << aiDepth << "." << endl;
                } else {
                    cout << "  Invalid depth. Keeping depth " << aiDepth << "." << endl;
                }
                cout << endl;
                break;
            }

            case 5:

                cout << endl;
                cout << "  Thanks for playing! Goodbye." << endl;
                cout << endl;
                maia.stop();
                return 0;

            default:
                cout << "  Invalid choice. Please enter 1-5." << endl;
                cout << endl;
                break;
        }
    }

    return 0;
}
