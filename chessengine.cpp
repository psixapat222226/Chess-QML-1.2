#include "ChessEngine.h"
#include <QDebug>
#include <QTimer>

ChessEngine::ChessEngine(QObject *parent)
    : QObject(parent),
    selectedPiece(QPoint(-1, -1)),
    currentStatus(STATUS::WHITE_TO_MOVE),
    gameMode(GameMode::TwoPlayers),
    m_difficulty(2), // По умолчанию средняя сложность
    hasLastMoveInfo(false)
{
    // Загружаем сохраненные партии при запуске
    loadSavedGames();

    // Стартуем новую партию
    startNewGame();
}

void ChessEngine::startNewGame()
{
    // Инициализация позиции на шахматной доске
    position = Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
                        Position::NONE, true, true, true, true, 1);
    selectedPiece = QPoint(-1, -1);
    currentStatus = STATUS::WHITE_TO_MOVE;

    // Очищаем историю ходов и последний ход
    moveHistory.clear();
    hasLastMoveInfo = false;

    emit piecesChanged();
    emit statusChanged();
    emit canUndoChanged();
    emit vsComputerEnabled();
    emit lastMoveChanged();
}

void ChessEngine::setGameMode(const QString& mode)
{
    if (mode == "twoPlayers") {
        gameMode = GameMode::TwoPlayers;
    } else if (mode == "vsComputer") {
        gameMode = GameMode::VsComputer;
    }
}

QVariantList ChessEngine::getPieces() const
{
    QVariantList piecesList;
    Pieces pieces = position.getPieces();

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int index = y * 8 + x;
            QString pieceName;

            if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::PAWN), index))
                pieceName = "whitePawn";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::KNIGHT), index))
                pieceName = "whiteKnight";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::BISHOP), index))
                pieceName = "whiteBishop";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::ROOK), index))
                pieceName = "whiteRook";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::QUEEN), index))
                pieceName = "whiteQueen";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::KING), index))
                pieceName = "whiteKing";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::PAWN), index))
                pieceName = "blackPawn";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::KNIGHT), index))
                pieceName = "blackKnight";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::BISHOP), index))
                pieceName = "blackBishop";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::ROOK), index))
                pieceName = "blackRook";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::QUEEN), index))
                pieceName = "blackQueen";
            else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::KING), index))
                pieceName = "blackKing";
            else
                continue;

            QVariantMap piece;
            piece["x"] = x;
            piece["y"] = y;
            piece["type"] = pieceName;
            piecesList.append(piece);
        }
    }

    return piecesList;
}

QString ChessEngine::getTextureName(int x, int y) const
{
    int index = y * 8 + x;
    Pieces pieces = position.getPieces();

    if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::PAWN), index))
        return "whitePawn";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::KNIGHT), index))
        return "whiteKnight";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::BISHOP), index))
        return "whiteBishop";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::ROOK), index))
        return "whiteRook";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::QUEEN), index))
        return "whiteQueen";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::WHITE, PIECE::KING), index))
        return "whiteKing";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::PAWN), index))
        return "blackPawn";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::KNIGHT), index))
        return "blackKnight";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::BISHOP), index))
        return "blackBishop";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::ROOK), index))
        return "blackRook";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::QUEEN), index))
        return "blackQueen";
    else if (BOp::getBit(pieces.getPieceBitboard(SIDE::BLACK, PIECE::KING), index))
        return "blackKing";

    return "";
}

QVariantList ChessEngine::getLegalMovesForPiece(int x, int y) const
{
    QVariantList moves;
    int from = y * 8 + x;

    // Получаем ходы только для фигур текущего игрока
    Pieces pieces = position.getPieces();
    if ((currentStatus == STATUS::WHITE_TO_MOVE &&
         !BOp::getBit(pieces.getSideBitboard(SIDE::WHITE), from)) ||
        (currentStatus == STATUS::BLACK_TO_MOVE &&
         !BOp::getBit(pieces.getSideBitboard(SIDE::BLACK), from))) {
        return moves;
    }

    uint8_t side = (currentStatus == STATUS::WHITE_TO_MOVE) ? SIDE::WHITE : SIDE::BLACK;
    MoveList legalMoves = LegalMoveGen::generate(position, side);

    for (uint8_t i = 0; i < legalMoves.getSize(); i++) {
        Move move = legalMoves[i];
        if (move.getFrom() == from) {
            int toX = move.getTo() % 8;
            int toY = move.getTo() / 8;
            QVariantMap moveMap;
            moveMap["x"] = toX;
            moveMap["y"] = toY;
            moves.append(moveMap);
        }
    }

    return moves;
}

void ChessEngine::recordMove(int fromX, int fromY, int toX, int toY)
{
    MoveHistoryItem item;
    item.position = position;  // Сохраняем положение до хода
    item.moveFrom = QPoint(fromX, fromY);
    item.moveTo = QPoint(toX, toY);
    moveHistory.push(item);

    // Обновляем информацию о последнем ходе
    setLastMove(fromX, fromY, toX, toY);

    emit canUndoChanged();
    emit vsComputerEnabled();
}

void ChessEngine::setLastMove(int fromX, int fromY, int toX, int toY)
{
    lastMoveFrom = QPoint(fromX, fromY);
    lastMoveTo = QPoint(toX, toY);
    hasLastMoveInfo = true;

    qDebug() << "Last move set: " << fromX << "," << fromY << " to " << toX << "," << toY;

    emit lastMoveChanged();
}

bool ChessEngine::processMove(int fromX, int fromY, int toX, int toY)
{
    if (currentStatus != STATUS::WHITE_TO_MOVE &&
        currentStatus != STATUS::BLACK_TO_MOVE) {
        return false;
    }

    // В режиме игры с компьютером, игрок может ходить только белыми
    if (gameMode == GameMode::VsComputer && currentStatus == STATUS::BLACK_TO_MOVE) {
        return false;
    }

    uint8_t side = (currentStatus == STATUS::WHITE_TO_MOVE) ? SIDE::WHITE : SIDE::BLACK;
    uint8_t from = fromY * 8 + fromX;
    uint8_t to = toY * 8 + toX;

    MoveList moves = LegalMoveGen::generate(position, side);
    int moveIndex = -1;
    bool isPawnPromotion = false;


    bool isWhitePawnPromotion = (side == SIDE::WHITE && toY == 7 &&
                                 BOp::getBit(position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::PAWN), from));
    bool isBlackPawnPromotion = (side == SIDE::BLACK && toY == 0 &&
                                 BOp::getBit(position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::PAWN), from));

    if (isWhitePawnPromotion || isBlackPawnPromotion) {
        // Просто проверяем, что такой ход возможен (без учета типа превращения)
        for (uint8_t i = 0; i < moves.getSize(); i++) {
            Move move = moves[i];
            if (move.getFrom() == from && move.getTo() == to &&
                (move.getFlag() == Move::FLAG::PROMOTE_TO_QUEEN ||
                 move.getFlag() == Move::FLAG::PROMOTE_TO_ROOK ||
                 move.getFlag() == Move::FLAG::PROMOTE_TO_BISHOP ||
                 move.getFlag() == Move::FLAG::PROMOTE_TO_KNIGHT)) {
                isPawnPromotion = true;
                // Сохраняем позицию до превращения
                recordMove(fromX, fromY, toX, toY);
                // Мы нашли допустимый ход превращения пешки, но не выполняем его сейчас
                // Вместо этого запрашиваем у пользователя тип фигуры
                emit pawnPromotion(fromX, fromY, toX, toY);
                return true; // Возвращаем true, чтобы показать, что ход будет обработан позже
            }
        }
    }

    // Если это не превращение пешки, продолжаем как обычно
    if (!isPawnPromotion) {
        for (uint8_t i = 0; i < moves.getSize(); i++) {
            Move move = moves[i];
            if (move.getFrom() == from && move.getTo() == to) {
                moveIndex = i;
                break;
            }
        }
    }

    if (moveIndex != -1) {
        Move selectedMove = moves[moveIndex];

        // Записываем ход в историю перед его выполнением
        recordMove(fromX, fromY, toX, toY);

        position.move(selectedMove);
        updateStatus();
        emit piecesChanged();
        emit statusChanged();

        if (currentStatus == STATUS::WHITE_WON ||
            currentStatus == STATUS::BLACK_WON ||
            currentStatus == STATUS::DRAW) {
            QString result;
            if (currentStatus == STATUS::WHITE_WON) {
                result = "Белые победили!";
            } else if (currentStatus == STATUS::BLACK_WON) {
                result = "Чёрные победили!";
            } else {
                result = "Ничья!";
            }
            emit gameEnded(result);
        }
        // Если режим игры с компьютером и сейчас ход черных, компьютер делает ход
        else if (gameMode == GameMode::VsComputer && currentStatus == STATUS::BLACK_TO_MOVE) {
            QTimer::singleShot(500, this, &ChessEngine::makeAIMove);
        }

        return true;
    }

    return false;
}

bool ChessEngine::promotePawn(int fromX, int fromY, int toX, int toY, const QString& pieceType)
{
    uint8_t side = (currentStatus == STATUS::WHITE_TO_MOVE) ? SIDE::WHITE : SIDE::BLACK;
    uint8_t from = fromY * 8 + fromX;
    uint8_t to = toY * 8 + toX;

    MoveList moves = LegalMoveGen::generate(position, side);
    int moveIndex = -1;

    // Определяем флаг превращения в зависимости от выбранного типа фигуры
    Move::FLAG promotionFlag;
    if (pieceType == "queen") {
        promotionFlag = Move::FLAG::PROMOTE_TO_QUEEN;
    } else if (pieceType == "rook") {
        promotionFlag = Move::FLAG::PROMOTE_TO_ROOK;
    } else if (pieceType == "bishop") {
        promotionFlag = Move::FLAG::PROMOTE_TO_BISHOP;
    } else if (pieceType == "knight") {
        promotionFlag = Move::FLAG::PROMOTE_TO_KNIGHT;
    } else {
        // По умолчанию превращаем в ферзя
        promotionFlag = Move::FLAG::PROMOTE_TO_QUEEN;
    }

    // Находим ход с соответствующим флагом
    for (uint8_t i = 0; i < moves.getSize(); i++) {
        Move move = moves[i];
        if (move.getFrom() == from && move.getTo() == to && move.getFlag() == promotionFlag) {
            moveIndex = i;
            break;
        }
    }

    if (moveIndex != -1) {
        Move selectedMove = moves[moveIndex];

        position.move(selectedMove);
        updateStatus();
        emit piecesChanged();
        emit statusChanged();

        if (currentStatus == STATUS::WHITE_WON ||
            currentStatus == STATUS::BLACK_WON ||
            currentStatus == STATUS::DRAW) {
            QString result;
            if (currentStatus == STATUS::WHITE_WON) {
                result = "Белые победили!";
            } else if (currentStatus == STATUS::BLACK_WON) {
                result = "Чёрные победили!";
            } else {
                result = "Ничья!";
            }
            emit gameEnded(result);
        }
        // Если режим игры с компьютером и сейчас ход черных, компьютер делает ход
        else if (gameMode == GameMode::VsComputer && currentStatus == STATUS::BLACK_TO_MOVE) {
            QTimer::singleShot(500, this, &ChessEngine::makeAIMove);
        }

        return true;
    }

    return false;
}

void ChessEngine::updateStatus()
{
    currentStatus = static_cast<STATUS>(getStatus());
}

QString ChessEngine::status() const
{
    return statusToString();
}

QString ChessEngine::statusToString() const
{
    switch (currentStatus) {
    case STATUS::WHITE_TO_MOVE: return "Ход белых";
    case STATUS::BLACK_TO_MOVE: return "Ход чёрных";
    case STATUS::WHITE_WON: return "Белые выиграли";
    case STATUS::BLACK_WON: return "Чёрные выиграли";
    case STATUS::DRAW: return "Ничья";
    }
    return "Неизвестно";
}

uint8_t ChessEngine::getStatus() const
{
    if (position.fiftyMovesRuleDraw() || position.threefoldRepetitionDraw()) {
        return STATUS::DRAW;
    }

    if (!position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::PAWN) &&
        !position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::PAWN) &&
        !position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::ROOK) &&
        !position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::ROOK) &&
        !position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::QUEEN) &&
        !position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::QUEEN) &&
        BOp::count1(position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::KNIGHT) |
                    position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::BISHOP)) < 2 &&
        BOp::count1(position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::KNIGHT) |
                    position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::BISHOP)) < 2) {
        return STATUS::DRAW;
    }

    if (position.whiteToMove()) {
        MoveList whiteMoves = LegalMoveGen::generate(position, SIDE::WHITE);
        if (whiteMoves.getSize() == 0) {
            bool whiteInCheck = PsLegalMoveMaskGen::inDanger(
                position.getPieces(),
                BOp::bsf(position.getPieces().getPieceBitboard(SIDE::WHITE, PIECE::KING)),
                SIDE::WHITE
                );
            return whiteInCheck ? STATUS::BLACK_WON : STATUS::DRAW;
        }
        return STATUS::WHITE_TO_MOVE;
    } else {
        MoveList blackMoves = LegalMoveGen::generate(position, SIDE::BLACK);
        if (blackMoves.getSize() == 0) {
            bool blackInCheck = PsLegalMoveMaskGen::inDanger(
                position.getPieces(),
                BOp::bsf(position.getPieces().getPieceBitboard(SIDE::BLACK, PIECE::KING)),
                SIDE::BLACK
                );
            return blackInCheck ? STATUS::WHITE_WON : STATUS::DRAW;
        }
        return STATUS::BLACK_TO_MOVE;
    }
}

bool ChessEngine::undoLastMove()
{
    if (moveHistory.isEmpty()) {
        return false;
    }

    MoveHistoryItem lastMove = moveHistory.pop();
    position = lastMove.position;

    // Обновляем последний ход для визуализации
    if (!moveHistory.isEmpty()) {
        MoveHistoryItem prevMove = moveHistory.top();
        lastMoveFrom = prevMove.moveFrom;
        lastMoveTo = prevMove.moveTo;
        hasLastMoveInfo = true;
    } else {
        hasLastMoveInfo = false;
    }

    updateStatus();
    emit piecesChanged();
    emit canUndoChanged();
    emit vsComputerEnabled();
    emit lastMoveChanged();
    emit statusChanged();

    return true;
}

bool ChessEngine::canUndo() const
{
    return !moveHistory.isEmpty();
}

bool ChessEngine::vsComputer() const
{
    return gameMode == GameMode::VsComputer;
}

QPoint ChessEngine::getLastMoveFrom() const
{
    return lastMoveFrom;
}

QPoint ChessEngine::getLastMoveTo() const
{
    return lastMoveTo;
}

bool ChessEngine::hasLastMove() const
{
    return hasLastMoveInfo;
}

int ChessEngine::difficulty() const
{
    return m_difficulty;
}

void ChessEngine::setDifficulty(int newDifficulty)
{
    if (newDifficulty < 1)
        newDifficulty = 1;
    else if (newDifficulty > 3)
        newDifficulty = 3;

    if (m_difficulty != newDifficulty) {
        m_difficulty = newDifficulty;
        emit difficultyChanged();
    }
}

QString ChessEngine::getDifficultyName() const
{
    switch (m_difficulty) {
    case 1: return "Легкий";
    case 2: return "Средний";
    case 3: return "Сложный";
    default: return "Средний";
    }
}

void ChessEngine::makeAIMove()
{
    if (currentStatus != STATUS::BLACK_TO_MOVE) {
        return;
    }


    int thinkingTime;
    switch (m_difficulty) {
    case 1: // Легкий
        thinkingTime = 500; // 0.5 секунд
        break;
    case 2: // Средний
        thinkingTime = 1000; // 1 секунда
        break;
    case 3: // Сложный
        thinkingTime = 2000; // 2 секунды
        break;
    default:
        thinkingTime = 1000;
    }

    // AI делает ход с учетом сложности
    Move aiMove = AI::getBestMove(position, SIDE::BLACK, thinkingTime);

    int fromX = aiMove.getFrom() % 8;
    int fromY = aiMove.getFrom() / 8;
    int toX = aiMove.getTo() % 8;
    int toY = aiMove.getTo() / 8;


    recordMove(fromX, fromY, toX, toY);


    position.move(aiMove);
    updateStatus();


    emit piecesChanged();
    emit statusChanged();

    // Проверка на конец игры
    if (currentStatus == STATUS::WHITE_WON ||
        currentStatus == STATUS::BLACK_WON ||
        currentStatus == STATUS::DRAW) {
        QString result;
        if (currentStatus == STATUS::WHITE_WON) {
            result = "Белые победили!";
        } else if (currentStatus == STATUS::BLACK_WON) {
            result = "Чёрные победили!";
        } else {
            result = "Ничья!";
        }
        emit gameEnded(result);
    }
}


bool ChessEngine::saveGame(const QString& name)
{

    if (savedGames.size() >= 3) {
        return false;
    }

    SavedGame game;
    game.name = name;
    game.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    game.gameMode = (gameMode == GameMode::TwoPlayers) ? "twoPlayers" : "vsComputer";
    game.difficulty = m_difficulty;
    game.status = statusToString();
    game.fen = serializePosition(position);


    for (const MoveHistoryItem& item : moveHistory) {
        game.moveFromHistory.append(item.moveFrom);
        game.moveToHistory.append(item.moveTo);
    }


    savedGames.append(game);
    saveSavedGames();
    emit savedGamesChanged();

    return true;
}


bool ChessEngine::loadGame(int slot)
{
    if (slot < 0 || slot >= savedGames.size()) {
        return false;
    }

    const SavedGame& game = savedGames[slot];


    if (game.gameMode == "twoPlayers") {
        gameMode = GameMode::TwoPlayers;
    } else {
        gameMode = GameMode::VsComputer;
    }


    m_difficulty = game.difficulty;


    position = deserializePosition(game.fen);


    moveHistory.clear();

    for (int i = 0; i < game.moveFromHistory.size(); i++) {
        MoveHistoryItem item;
        item.moveFrom = game.moveFromHistory[i];
        item.moveTo = game.moveToHistory[i];

        moveHistory.push(item);
    }


    if (!game.moveFromHistory.isEmpty()) {
        lastMoveFrom = game.moveFromHistory.last();
        lastMoveTo = game.moveToHistory.last();
        hasLastMoveInfo = true;
    } else {
        hasLastMoveInfo = false;
    }


    updateStatus();


    emit piecesChanged();
    emit statusChanged();
    emit difficultyChanged();
    emit canUndoChanged();
    emit vsComputerEnabled();
    emit lastMoveChanged();

    return true;
}


QVariantList ChessEngine::getSavedGames() const
{
    QVariantList result;

    for (int i = 0; i < savedGames.size(); i++) {
        QVariantMap game;
        game["slot"] = i;
        game["name"] = savedGames[i].name;
        game["date"] = savedGames[i].date;
        game["gameMode"] = savedGames[i].gameMode;
        game["difficulty"] = savedGames[i].difficulty;
        game["status"] = savedGames[i].status;
        result.append(game);
    }

    return result;
}

// Удаление сохраненной партии
bool ChessEngine::deleteGame(int slot)
{
    if (slot < 0 || slot >= savedGames.size()) {
        return false;
    }

    savedGames.remove(slot);
    saveSavedGames();
    emit savedGamesChanged();

    return true;
}


void ChessEngine::loadSavedGames()
{
    savedGames.clear();

    QSettings settings;
    int count = settings.beginReadArray("savedGames");

    for (int i = 0; i < count; i++) {
        settings.setArrayIndex(i);

        SavedGame game;
        game.name = settings.value("name").toString();
        game.date = settings.value("date").toString();
        game.gameMode = settings.value("gameMode").toString();
        game.difficulty = settings.value("difficulty").toInt();
        game.status = settings.value("status").toString();
        game.fen = settings.value("fen").toString();

        QJsonDocument doc = QJsonDocument::fromJson(settings.value("moveHistory").toString().toUtf8());
        QJsonArray historyArray = doc.array();

        for (int j = 0; j < historyArray.size(); j += 2) {
            QJsonObject fromObj = historyArray[j].toObject();
            QJsonObject toObj = historyArray[j+1].toObject();

            QPoint from(fromObj["x"].toInt(), fromObj["y"].toInt());
            QPoint to(toObj["x"].toInt(), toObj["y"].toInt());

            game.moveFromHistory.append(from);
            game.moveToHistory.append(to);
        }

        savedGames.append(game);
    }

    settings.endArray();
}


void ChessEngine::saveSavedGames()
{
    QSettings settings;
    settings.beginWriteArray("savedGames");

    for (int i = 0; i < savedGames.size(); i++) {
        settings.setArrayIndex(i);

        settings.setValue("name", savedGames[i].name);
        settings.setValue("date", savedGames[i].date);
        settings.setValue("gameMode", savedGames[i].gameMode);
        settings.setValue("difficulty", savedGames[i].difficulty);
        settings.setValue("status", savedGames[i].status);
        settings.setValue("fen", savedGames[i].fen);


        QJsonArray historyArray;
        for (int j = 0; j < savedGames[i].moveFromHistory.size(); j++) {
            QJsonObject fromObj;
            fromObj["x"] = savedGames[i].moveFromHistory[j].x();
            fromObj["y"] = savedGames[i].moveFromHistory[j].y();

            QJsonObject toObj;
            toObj["x"] = savedGames[i].moveToHistory[j].x();
            toObj["y"] = savedGames[i].moveToHistory[j].y();

            historyArray.append(fromObj);
            historyArray.append(toObj);
        }

        QJsonDocument doc(historyArray);
        settings.setValue("moveHistory", QString(doc.toJson()));
    }

    settings.endArray();
}


QString ChessEngine::serializePosition(const Position& pos) const
{



    std::ostringstream fenStream;
    fenStream << pos.getPieces();
    QString fenPart = QString::fromStdString(fenStream.str().c_str());


    fenPart = fenPart.trimmed();

    QString extendedFen = fenPart;
    extendedFen += "|" + QString::number(pos.getEnPassant());
    extendedFen += "|" + QString::number(pos.getWLCastling() ? 1 : 0);
    extendedFen += "|" + QString::number(pos.getWSCastling() ? 1 : 0);
    extendedFen += "|" + QString::number(pos.getBLCastling() ? 1 : 0);
    extendedFen += "|" + QString::number(pos.getBSCastling() ? 1 : 0);
    extendedFen += "|" + QString::number(pos.whiteToMove() ? 1 : 0);

    qDebug() << "Serialized FEN:" << extendedFen;

    return extendedFen;
}

Position ChessEngine::deserializePosition(const QString& data)
{
    qDebug() << "Deserializing position from:" << data;

    QStringList parts = data.split("|");
    if (parts.size() < 7) {
        qWarning() << "Invalid saved position data, expected 7+ parts, got" << parts.size();

        return Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
                        Position::NONE, true, true, true, true, 1);
    }


    std::string fenPart = parts[0].trimmed().toStdString();
    qDebug() << "FEN part:" << QString::fromStdString(fenPart);

    try {

        int slashCount = 0;
        for(char c : fenPart) {
            if(c == '/') slashCount++;
        }

        if(slashCount != 7) {
            qWarning() << "Invalid FEN format: wrong number of rows (slashes)";
            return Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
                            Position::NONE, true, true, true, true, 1);
        }

        uint8_t enPassant = parts[1].toUInt();
        bool wlCastling = parts[2].toInt() != 0;
        bool wsCastling = parts[3].toInt() != 0;
        bool blCastling = parts[4].toInt() != 0;
        bool bsCastling = parts[5].toInt() != 0;
        float moveCtr = parts[6].toInt() != 0 ? 1.0f : 1.5f;


        Position position(fenPart, enPassant, wlCastling, wsCastling, blCastling, bsCastling, moveCtr);


        if (BOp::count1(position.getPieces().getAllBitboard()) == 0) {
            qWarning() << "Position contains no pieces!";
            return Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
                            Position::NONE, true, true, true, true, 1);
        }

        return position;
    }
    catch (const std::exception& e) {
        qWarning() << "Exception during position deserialization:" << e.what();
        return Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
                        Position::NONE, true, true, true, true, 1);
    }
}


QString ChessEngine::getMoveHistoryJson() const
{
    QJsonArray historyArray;
    for (const MoveHistoryItem& item : moveHistory) {
        QJsonObject fromObj;
        fromObj["x"] = item.moveFrom.x();
        fromObj["y"] = item.moveFrom.y();

        QJsonObject toObj;
        toObj["x"] = item.moveTo.x();
        toObj["y"] = item.moveTo.y();

        historyArray.append(fromObj);
        historyArray.append(toObj);
    }

    QJsonDocument doc(historyArray);
    return QString(doc.toJson());
}


void ChessEngine::setMoveHistoryFromJson(const QString& historyJson)
{
    moveHistory.clear();

    QJsonDocument doc = QJsonDocument::fromJson(historyJson.toUtf8());
    QJsonArray historyArray = doc.array();

    for (int i = 0; i < historyArray.size(); i += 2) {
        QJsonObject fromObj = historyArray[i].toObject();
        QJsonObject toObj = historyArray[i+1].toObject();

        MoveHistoryItem item;
        item.moveFrom = QPoint(fromObj["x"].toInt(), fromObj["y"].toInt());
        item.moveTo = QPoint(toObj["x"].toInt(), toObj["y"].toInt());

        moveHistory.push(item);
    }

    if (!moveHistory.isEmpty()) {
        MoveHistoryItem lastItem = moveHistory.top();
        lastMoveFrom = lastItem.moveFrom;
        lastMoveTo = lastItem.moveTo;
        hasLastMoveInfo = true;
    } else {
        hasLastMoveInfo = false;
    }

    emit canUndoChanged();
    emit vsComputerEnabled();
    emit lastMoveChanged();
}
