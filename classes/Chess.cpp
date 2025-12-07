#include "Chess.h"
#include "ValueTable.h"
#include <limits>
#include <cmath>
#include <array>

static int actualPS[128][64];
#define FLIP(x) (x^56)

static const std::array<int, 128> evaluateScores = []() {
    std::array<int, 128> scores {};
    scores['P'] = 100; scores['p'] = -100;
    scores['N'] = 300; scores['n'] = -300;
    scores['B'] = 300; scores['b'] = -300;
    scores['R'] = 500; scores['r'] = -500;
    scores['Q'] = 900; scores['q'] = -900;
    scores['K'] = 2000; scores['k'] = -2000;
    scores['0'] = 0;
    return scores;
} ();

static const std::array<int *, 128> ValueTables = []() {
    std::array<int *, 128> pieceValues{};
    pieceValues['P'] = (int* )&pawnTableW;      pieceValues['p'] = (int* )&pawnTableB;
    pieceValues['N'] = (int* )&knightTableW;    pieceValues['n'] = (int* )&knightTableB;
    pieceValues['B'] = (int* )&bishopTableW;    pieceValues['b'] = (int* )&bishopTableB;
    pieceValues['R'] = (int* )&rookTableW;      pieceValues['r'] = (int* )&rookTableB;
    pieceValues['Q'] = (int* )&queenTableW;     pieceValues['q'] = (int* )&queenTableB;
    pieceValues['K'] = (int* )&kingTableW;      pieceValues['k'] = (int* )&kingTableB;
    pieceValues['0'] = (int* )&emptyTable;
    return pieceValues;
} ();

Chess::Chess()
{
    _grid = new Grid(8, 8);

    
    std::memset(actualPS, 0, sizeof(actualPS));
    const char pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
    for (int p = 0; p < 6; p++) {
        int score = evaluateScores[pieces[p]];
        for (int sq = 0; sq < 64; sq++) {
            int finalW = ValueTables[pieces[p]][sq] + score;
            int finalB = ValueTables[tolower(pieces[p])][sq] - score;
            actualPS[p + WHITE_PAWNS][sq] = finalW;
            actualPS[p + BLACK_PAWNS][sq] = finalB;
            actualPS[pieces[p]][sq] = finalW;
            actualPS[tolower(pieces[p])][sq] = finalB;
        }
    }
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };
    
    Bit* bit = new Bit();
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    _currentPlayer = WHITE;
    _gameState.init( stateString().c_str(), _currentPlayer);
    _moves = _gameState.generateAllMoves();

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }
    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
    printf("setting up board from FEN: %s\n", fen.c_str());
        int row = 7;
        int col = 0;
        for (char c : fen) {
            if (c == '/') {
                row--;
                col = 0;
            } else if (isdigit(c)) {
                col += c - '0';
            } else {
                int playerNumber = isupper(c) ? 0 : 1;
                ChessPiece piece = NoPiece;
                switch (tolower(c)) {
                    case 'p': piece = Pawn; break;
                    case 'n': piece = Knight; break;
                    case 'b': piece = Bishop; break;
                    case 'r': piece = Rook; break;
                    case 'q': piece = Queen; break;
                    case 'k': piece = King; break;
                }
                if (piece != NoPiece) {
                    printf("%d/%d", row, col);
                    ChessSquare *square = _grid->getSquare(col, row);
                    Bit *bit = PieceForPlayer(playerNumber, piece);
                    bit->setPosition(square->getPosition());
                    bit->setParent(square);
                    bit->setGameTag(isupper(c) ? piece : piece + 128);
                    square->setBit(bit);
                }
                col++;
            }
        }

}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor != currentPlayer) return false;

    clearBoardHighlights(); 
    bool high = false;
    ChessSquare* fromSquare = (ChessSquare *)&src;
    if (fromSquare) {
        int fromIndex = fromSquare -> getSquareIndex();
        for (auto move : _moves) {
            if (move.from == fromIndex){
                high = true;
                auto light = _grid -> getSquareByIndex(move.to);
                light -> setHighlighted(true);
            }   
        }
    } 
    return high;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* fromSquare = (ChessSquare *)&src;
    ChessSquare* toSquare = (ChessSquare *)&dst;
    if (toSquare) {
        int toIndex = toSquare -> getSquareIndex();
        int fromIndex = fromSquare -> getSquareIndex();
        for (auto move : _moves) {
            if (move.to == toIndex && move.from == fromIndex){
                return true;
            }   
        }
    }
    return false;
}

void Chess::clearBoardHighlights() {
    _grid -> forEachSquare([](ChessSquare* sq, int x, int y) {
        sq -> setHighlighted(false);
    });
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    _currentPlayer = (_currentPlayer == WHITE ? BLACK : WHITE);
    _gameState.init(stateString().c_str(), _currentPlayer);
    _moves = _gameState.generateAllMoves();
    clearBoardHighlights();
    endTurn();
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, ChessPiece::Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

void Chess::updateAI() {


    int bestVal = negInfinite;
    BitMove bestMove;

    for (const auto& move : _moves) {
        
        _gameState.pushMove(move);

        int moveVal = -negamax(_gameState, 5, negInfinite, posInfinite);

        _gameState.popState();

        if (moveVal > bestVal) {
            bestMove = move;
            bestVal = moveVal;
        }
    }

    if (bestVal != negInfinite) {
        int fromSquare = bestMove.from;
        int toSquare = bestMove.to;
        BitHolder& from = getHolderAt(fromSquare & 7, fromSquare / 8);
        BitHolder& to = getHolderAt(toSquare & 7, toSquare / 8);
        Bit* bit = from.bit();
        to.dropBitAtPoint(bit, ImVec2(0, 0));
        from.setBit(nullptr);
        bitMovedFromTo(*bit, from, to);
    }
}

int Chess::negamax(GameState& gameState, int depth, int alpha, int beta) {

    if (depth == 0) {
        return evaluateBoard(gameState);
    }

    auto newMoves = gameState.generateAllMoves();

    int bestVal = negInfinite; 

    for(const auto& move : newMoves) {

        gameState.pushMove(move);

        bestVal = std::max(bestVal, -negamax(gameState, depth - 1, -beta, -alpha));

        gameState.popState();

        alpha = std::max(alpha, bestVal);
        if (alpha >= beta) {
            break;
        }
    }
    return bestVal;
}

int Chess::evaluateBoard(const GameState& gameState) {
    int score = 0;

    for (int square = 0; square < 64; square++) {
        const unsigned char piece = (gameState.state[square]);
        score += actualPS[piece][square];
    }

    return score * gameState.color;
}   