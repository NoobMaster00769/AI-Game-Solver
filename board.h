
#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include <vector>
#include <string>

using namespace std;


struct UndoInfo {
    Piece capturedPiece;        
    int   capturedSquare;       
    bool  castleWhiteKing;      
    bool  castleWhiteQueen;
    bool  castleBlackKing;
    bool  castleBlackQueen;
    int   enPassantSquare;      
    int   halfMoveClock;        
};


class Board {
public:

    Piece board[64];


    Color sideToMove;


    bool castleWhiteKing;     
    bool castleWhiteQueen;    
    bool castleBlackKing;     
    bool castleBlackQueen;    


    int enPassantSquare;


    int halfMoveClock;   
    int fullMoveNumber;  


    int whiteKingSquare;
    int blackKingSquare;

    vector<UndoInfo> history;


    vector<Move> moveHistory;


    Board();


    void initFromFEN(const string& fen);


    void makeMove(const Move& move);


    void unmakeMove(const Move& move);


    bool isSquareAttacked(int square, Color byColor) const;


    bool isInCheck(Color color) const;
    bool isCheckmate(const vector<Move>& legalMoves) const;
    bool isStalemate(const vector<Move>& legalMoves) const;
    bool isDraw() const;  


    void printBoard() const;


    int getKingSquare(Color color) const;


    string toFEN() const;

private:

    void updateKingPosition(Piece piece, int square);
};

#endif 
