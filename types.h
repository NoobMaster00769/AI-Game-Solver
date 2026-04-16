
#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <cstdint>

using namespace std;


enum Color {
    WHITE = 0,
    BLACK = 1
};


inline Color oppositeColor(Color c) {
    return (c == WHITE) ? BLACK : WHITE;
}


enum PieceType {
    PAWN   = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK   = 3,
    QUEEN  = 4,
    KING   = 5,
    NO_PIECE_TYPE = 6
};


enum Piece {
    W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,   
    B_PAWN = 6, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,   
    NO_PIECE = 12
};


inline Color pieceColor(Piece p) {
    return (p < 6) ? WHITE : BLACK;
}


inline PieceType pieceType(Piece p) {
    if (p == NO_PIECE) return NO_PIECE_TYPE;
    return (PieceType)(p % 6);
}


inline Piece makePiece(Color c, PieceType pt) {
    return (Piece)(pt + c * 6);
}



inline int squareRank(int sq) { return sq / 8; }   
inline int squareFile(int sq) { return sq % 8; }    
inline int makeSquare(int rank, int file) { return rank * 8 + file; }


inline bool isValidRankFile(int rank, int file) {
    return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}


inline string squareToString(int sq) {
    char file = 'a' + squareFile(sq);  
    char rank = '1' + squareRank(sq);  
    return string(1, file) + string(1, rank);
}


inline int stringToSquare(const string& s) {
    if (s.size() < 2) return -1;
    int file = s[0] - 'a';   
    int rank = s[1] - '1';   
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return makeSquare(rank, file);
}




enum MoveFlag {
    FLAG_NONE       = 0,
    FLAG_CASTLE     = 1,   
    FLAG_EN_PASSANT = 2,   
    FLAG_PROMOTION  = 3    
};

struct Move {
    int from;        
    int to;          
    int flag;        


    Move() : from(-1), to(-1), flag(FLAG_NONE) {}


    Move(int f, int t, int fl = FLAG_NONE) : from(f), to(t), flag(fl) {}

  
    bool isValid() const { return from >= 0 && to >= 0; }


    bool operator==(const Move& other) const {
        return from == other.from && to == other.to && flag == other.flag;
    }


    string toString() const {
        if (!isValid()) return "none";
        return squareToString(from) + squareToString(to);
    }
};




const string PIECE_SYMBOLS[] = {
    "\xe2\x99\x99", 
    "\xe2\x99\x98", 
    "\xe2\x99\x97", 
    "\xe2\x99\x96", 
    "\xe2\x99\x95", 
    "\xe2\x99\x94", 
    "\xe2\x99\x9f", 
    "\xe2\x99\x9e", 
    "\xe2\x99\x9d", 
    "\xe2\x99\x9c", 
    "\xe2\x99\x9b", 
    "\xe2\x99\x9a", 
    " "              
};


const char PIECE_LETTERS[] = {
    'P', 'N', 'B', 'R', 'Q', 'K',   
    'p', 'n', 'b', 'r', 'q', 'k',   
    '.'                               
};




const string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


const int INF_SCORE = 1000000;


const int MATE_SCORE = 999999;

#endif 
