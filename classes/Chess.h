#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "GameState.h"

constexpr int pieceSize = 80;
constexpr int negInfinite = -1000000;
constexpr int posInfinite = 1000000;
//columns
constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = FILE_A << 1;
constexpr uint64_t FILE_C = FILE_A << 2;
constexpr uint64_t FILE_D = FILE_A << 3;
constexpr uint64_t FILE_E = FILE_A << 4;
constexpr uint64_t FILE_F = FILE_A << 5;
constexpr uint64_t FILE_G = FILE_A << 6;
constexpr uint64_t FILE_H = FILE_A << 7;
//rows
constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

constexpr uint64_t NOT_FILE_A = ~FILE_A;
constexpr uint64_t NOT_FILE_B = ~FILE_B;
constexpr uint64_t NOT_FILE_G = ~FILE_G;
constexpr uint64_t NOT_FILE_H = ~FILE_H;
constexpr uint64_t NOT_FILE_AB = ~(FILE_A | FILE_B);
constexpr uint64_t NOT_FILE_GH = ~(FILE_G | FILE_H);

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst);
    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }
    void updateAI();
    bool gameHasAI() override { return true; }
private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    int _currentPlayer = WHITE;
    int _countMoves = 0;
    GameState _gameState;
    std::vector<BitMove> _moves;
    BitBoard _knightBitBoards[64];
    BitBoard _kingBitBoards[64];

    void clearBoardHighlights();
    int negamax(GameState& state, int depth, int alpha, int beta);
    int evaluateBoard(const GameState&);
};