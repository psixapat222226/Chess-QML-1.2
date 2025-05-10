import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 800
    height: 650
    visible: true
    title: "Шахматы"

    property int boardMargin: 20
    property int boardSize: Math.min(width, height - 50) - (2 * boardMargin)
    property int cellSize: boardSize / 8
    property var selectedPiece: null
    property bool inMenu: true
    property bool inSettings: false

    // Функция для преобразования логических координат в визуальные
    function logicalToVisualPos(x, y) {
        return {
            x: x * cellSize,
            y: (7 - y) * cellSize
        }
    }

    // Функция для преобразования визуальных координат в логические
    function visualToLogicalPos(x, y) {
        return {
            x: Math.floor(x / cellSize),
            y: 7 - Math.floor(y / cellSize)
        }
    }

    // Очистка всех индикаторов и выбранных фигур
    function clearAllSelections() {
        moveIndicators.visible = false
        if (selectedPiece !== null) {
            selectedPiece.highlighted = false
        }
        selectedPiece = null
    }
    component StyledButton: Item {
        id: buttonContainer
        width: 300
        height: 50

        property string buttonText: "Button"
        property bool isSmall: false
        property int fontSize: isSmall ? 14 : 16
        property bool enabled: true
        property color shadowColor: "#333333"
        property int shadowSize: 4

        signal clicked()

        // Тень (нижняя полоса)
        Rectangle {
            id: shadow
            width: parent.width
            height: shadowSize
            color: shadowColor
            opacity: 0.5
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
        }

        // Основная кнопка
        Rectangle {
            id: styleButton
            width: parent.width
            height: parent.height - shadowSize
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
            anchors.top: parent.top

            Rectangle {
                width: parent.width - 4
                height: parent.height - 4
                x: 2
                y: 2
                color: mouseArea.pressed && buttonContainer.enabled ? "#5A5A5A" : "#6D6D6D"
                opacity: buttonContainer.enabled ? 1.0 : 0.5

                Text {
                    anchors.centerIn: parent
                    text: buttonContainer.buttonText
                    color: "white"
                    font.pixelSize: buttonContainer.fontSize
                    font.family: "Courier"
                    font.bold: true
                }
            }
        }

        // Эффект при нажатии - кнопка опускается к тени
        states: State {
            name: "pressed"
            when: mouseArea.pressed && buttonContainer.enabled
            PropertyChanges {
                target: styleButton
                y: shadowSize / 2
            }
            PropertyChanges {
                target: shadow
                height: shadowSize / 2
            }
        }

        // Плавная анимация перехода
        transitions: Transition {
            PropertyAnimation {
                properties: "y, height"
                duration: 50
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: {
                if (buttonContainer.enabled) {
                    buttonContainer.clicked()
                }
            }
        }
    }


    // Начальный экран с выбором режима игры
    Rectangle {
        id: menuScreen
        anchors.fill: parent
        visible: inMenu && !inSettings

        // Фон
        Image {
            anchors.fill: parent
            source: "qrc:/resources/images/fon.png"
            fillMode: Image.PreserveAspectCrop
        }

        // Позиционируем кнопки в нижней части экрана, чтобы не перекрывать надпись
        ColumnLayout {
            anchors {
                bottom: parent.bottom
                bottomMargin: 100
                horizontalCenter: parent.horizontalCenter
            }
            spacing: 20
            width: 300

            // Кнопки меню
            StyledButton {
                Layout.fillWidth: true
                buttonText: "Одиночная игра"
                onClicked: {
                    chessEngine.setGameMode("vsComputer")
                    chessEngine.startNewGame()
                    inMenu = false
                }
            }

            StyledButton {
                Layout.fillWidth: true
                buttonText: "Многопользовательский режим"
                onClicked: {
                    chessEngine.setGameMode("twoPlayers")
                    chessEngine.startNewGame()
                    inMenu = false
                }
            }

            StyledButton {
                Layout.fillWidth: true
                buttonText: "Настройки"
                onClicked: {
                    inSettings = true
                }
            }

            // Нижняя кнопка
            StyledButton {
                buttonText: "Выход из игры"
                Layout.fillWidth: true
                Layout.topMargin: 20
                onClicked: {
                    Qt.quit()
                }
            }
        }
    }

    // Экран настроек
    Rectangle {
        id: settingsScreen
        anchors.fill: parent
        visible: inSettings

        // Фон
        Image {
            anchors.fill: parent
            source: "qrc:/resources/images/fon2.png"
            fillMode: Image.PreserveAspectCrop
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20
            width: 500

            Text {
                text: "НАСТРОЙКИ"
                font.pixelSize: 32
                font.family: "Courier"
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 20
            }

            // Настройка сложности
            Rectangle {
                Layout.fillWidth: true
                height: 200
                color: Qt.rgba(0, 0, 0, 0.5)
                border.color: "#5A5A5A"
                border.width: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    Text {
                        text: "Сложность"
                        font.pixelSize: 20
                        font.family: "Courier"
                        font.bold: true
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        // Легкий
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: chessEngine.difficulty === 1 ? "#6DC76D" : "#828282"
                            border.color: "#5A5A5A"
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: "Легкий"
                                font.pixelSize: 16
                                font.family: "Courier"
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: chessEngine.difficulty = 1
                            }
                        }

                        // Средний
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: chessEngine.difficulty === 2 ? "#6D90D7" : "#828282"
                            border.color: "#5A5A5A"
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: "Средний"
                                font.pixelSize: 16
                                font.family: "Courier"
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: chessEngine.difficulty = 2
                            }
                        }

                        // Сложный
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: chessEngine.difficulty === 3 ? "#D76D6D" : "#828282"
                            border.color: "#5A5A5A"
                            border.width: 2

                            Text {
                                anchors.centerIn: parent
                                text: "Сложный"
                                font.pixelSize: 16
                                font.family: "Courier"
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: chessEngine.difficulty = 3
                            }
                        }
                    }
                }
            }

            // Управление сохраненными партиями
            Rectangle {
                Layout.fillWidth: true
                height: 150
                color: Qt.rgba(0, 0, 0, 0.5)
                border.color: "#5A5A5A"
                border.width: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    Text {
                        text: "Сохранения"
                        font.pixelSize: 20
                        font.family: "Courier"
                        font.bold: true
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        StyledButton {
                            buttonText: "Загрузить"
                            Layout.fillWidth: true
                            enabled: chessEngine.getSavedGames().length > 0
                            onClicked: {
                                loadGameDialog.open()
                            }
                        }

                        StyledButton {
                            buttonText: "Сохранить"
                            Layout.fillWidth: true
                            enabled: chessEngine.getSavedGames().length < 3
                            onClicked: {
                                gameNameInput.text = ""
                                saveGameDialog.open()
                            }
                        }
                    }
                }
            }

            // Кнопки в нижней части
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Layout.topMargin: 10

                StyledButton {
                    buttonText: "Готово"
                    Layout.fillWidth: true
                    onClicked: {
                        inSettings = false
                    }
                }

                StyledButton {
                    buttonText: "Отмена"
                    Layout.fillWidth: true
                    onClicked: {
                        inSettings = false
                    }
                }
            }
        }
    }

    // Игровой экран
    Item {
        anchors.fill: parent
        visible: !inMenu && !inSettings

        Image {
            anchors.fill: parent
            source: "qrc:/resources/images/fon2.png"
            fillMode: Image.PreserveAspectCrop
        }

        // Статус игры (перемещен в верхнюю часть)
        Text {
            id: statusText
            text: chessEngine.status
            font.pixelSize: 20
            font.family: "Courier"
            font.bold: true
            color: "white"
            anchors {
                top: parent.top
                topMargin: 10
                horizontalCenter: parent.horizontalCenter
            }
        }

        // Компонент шахматной фигуры
        component ChessPiece: Image {
            id: piece

            property int pieceX: 0  // Логическая X координата (0-7)
            property int pieceY: 0  // Логическая Y координата (0-7)
            property bool highlighted: false

            // Небольшой эффект подсветки для выбранной фигуры
            Rectangle {
                anchors.fill: parent
                color: "yellow"
                opacity: piece.highlighted ? 0.3 : 0
                z: -1
            }

            // Начальное позиционирование
            Component.onCompleted: {
                let pos = logicalToVisualPos(pieceX, pieceY)
                x = pos.x
                y = pos.y
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // Уже выбрана эта фигура - отменяем выбор
                    if (selectedPiece === piece) {
                        clearAllSelections()
                        return
                    }

                    // Сбрасываем предыдущие выборы
                    clearAllSelections()

                    // Проверяем, может ли эта фигура ходить
                    let legalMoves = chessEngine.getLegalMovesForPiece(pieceX, pieceY)

                    if (legalMoves.length > 0) {
                        // Показываем возможные ходы
                        moveIndicators.fromX = pieceX
                        moveIndicators.fromY = pieceY
                        moveIndicators.legalMoves = legalMoves
                        moveIndicators.visible = true

                        // Выделяем выбранную фигуру
                        piece.highlighted = true
                        selectedPiece = piece
                    }
                }
            }
        }

        // Шахматная доска (центр)
        Rectangle {
            id: board
            width: boardSize
            height: boardSize
            color: "#FFFFFF"
            anchors {
                centerIn: parent
                verticalCenterOffset: -10 // Немного выше центра для пространства кнопкам
            }

            // MouseArea для фона доски чтобы снимать выделения по клику на пустую область
            MouseArea {
                anchors.fill: parent
                z: -1  // Под всеми фигурами
                onClicked: {
                    clearAllSelections()
                }
            }

            // Рисуем шахматную доску
            Grid {
                anchors.fill: parent
                rows: 8
                columns: 8

                Repeater {
                    model: 64

                    Rectangle {
                        width: cellSize
                        height: cellSize
                        color: {
                            let row = Math.floor(index / 8)
                            let col = index % 8
                            return (row + col) % 2 === 0 ? "#F1D9B5" : "#B98863"
                        }
                    }
                }
            }

            // Компонент для выделения последнего хода
            Item {
                id: lastMoveHighlight
                anchors.fill: parent
                visible: chessEngine.hasLastMove()
                z: 1  // Над доской

                Rectangle {
                    width: cellSize
                    height: cellSize
                    x: chessEngine.hasLastMove() ? chessEngine.getLastMoveFrom().x * cellSize : 0
                    y: chessEngine.hasLastMove() ? (7 - chessEngine.getLastMoveFrom().y) * cellSize : 0
                    color: "#FFFF0055" // Полупрозрачный желтый
                    visible: chessEngine.hasLastMove()
                }

                Rectangle {
                    width: cellSize
                    height: cellSize
                    x: chessEngine.hasLastMove() ? chessEngine.getLastMoveTo().x * cellSize : 0
                    y: chessEngine.hasLastMove() ? (7 - chessEngine.getLastMoveTo().y) * cellSize : 0
                    color: "#FFFF0055" // Полупрозрачный желтый
                    visible: chessEngine.hasLastMove()
                }
            }

            // Шахматные фигуры
            Repeater {
                id: piecesRepeater
                model: chessEngine.getPieces()

                ChessPiece {
                    width: cellSize
                    height: cellSize
                    source: resourceManager.getTexturePath(modelData.type)
                    pieceX: modelData.x
                    pieceY: modelData.y
                }
            }

            // Индикаторы возможных ходов
            Item {
                id: moveIndicators
                anchors.fill: parent
                visible: false

                property var legalMoves: []
                property int fromX: -1
                property int fromY: -1

                Repeater {
                    id: movesRepeater
                    model: moveIndicators.legalMoves

                    Rectangle {
                        property var visualPos: logicalToVisualPos(modelData.x, modelData.y)

                        x: visualPos.x
                        y: visualPos.y
                        width: cellSize
                        height: cellSize
                        color: "transparent"
                        border.width: 3
                        border.color: "#32CD32"
                        radius: cellSize / 2
                        opacity: 0.7

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (chessEngine.processMove(
                                    moveIndicators.fromX, moveIndicators.fromY,
                                    modelData.x, modelData.y)) {
                                    clearAllSelections()
                                }
                            }
                        }
                    }
                }
            }
        }

        // Кнопки в нижней части
        Row {
            spacing: 30
            anchors {
                bottom: parent.bottom
                bottomMargin: 4
                horizontalCenter: parent.horizontalCenter
            }

            StyledButton {
                id: newGameButton
                width: 150
                height: 45
                buttonText: "Новая игра"
                isSmall: true
                onClicked: {
                    chessEngine.startNewGame()
                    clearAllSelections()
                }
            }

            StyledButton {
                id: undoButton
                width: 150
                height: 45
                buttonText: "Отменить ход"
                isSmall: true
                enabled: chessEngine.canUndo
                onClicked: {
                    chessEngine.undoLastMove()
                    if (chessEngine.vsComputer) {
                        chessEngine.undoLastMove()
                    }
                }
            }
            StyledButton {
                buttonText: "Меню"
                isSmall: true
                width: 150
                height: 45
                onClicked: {
                    inMenu = true
                }
            }
        }
    }

    // Обработка сигналов от движка
    Connections {
        target: chessEngine

        function onGameEnded(result) {
            resultDialog.text = result
            resultDialog.open()
        }

        function onPiecesChanged() {
            // Сбрасываем все выбранные фигуры и индикаторы при обновлении доски
            clearAllSelections()
            piecesRepeater.model = chessEngine.getPieces()
        }

        function onStatusChanged() {
            // Дополнительно убеждаемся, что индикаторы очищаются при смене хода
            clearAllSelections()
        }

        function onPawnPromotion(fromX, fromY, toX, toY) {
            promotionDialog.fromX = fromX
            promotionDialog.fromY = fromY
            promotionDialog.toX = toX
            promotionDialog.toY = toY
            promotionDialog.open()
        }

        function onLastMoveChanged() {
            lastMoveHighlight.visible = chessEngine.hasLastMove()
        }

        function onDifficultyChanged() {
            // Этот метод будет вызываться при изменении сложности
        }

        function onSavedGamesChanged() {
            // Обновляем состояние кнопок
        }
    }

    // Диалоги
    Dialog {
        id: resultDialog
        title: "Игра окончена"
        modal: true

        background: Rectangle {
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
        }

        property string text: ""

        Label {
            text: resultDialog.text
            font.pixelSize: 16
            font.family: "Courier"
            font.bold: true
            color: "white"
        }

        footer: DialogButtonBox {
            Button {
                text: "ОК"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                background: Rectangle {
                    color: parent.hovered ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        anchors.centerIn: Overlay.overlay

        onAccepted: {
            close()
        }
    }

    // Диалог выбора фигуры для превращения пешки
    Dialog {
        id: promotionDialog
        title: "Превращение пешки"
        modal: true
        closePolicy: Dialog.NoAutoClose
        width: 280  // Увеличена ширина
        height: 280 // Увеличена высота

        background: Rectangle {
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
        }

        header: Rectangle {
            color: "#6D6D6D"
            height: 40

            Text {
                anchors.centerIn: parent
                text: promotionDialog.title
                font.pixelSize: 18
                font.family: "Courier"
                font.bold: true
                color: "white"
            }
        }

        property int fromX: -1
        property int fromY: -1
        property int toX: -1
        property int toY: -1

        anchors.centerIn: Overlay.overlay

        // Сетка 2x2 для фигур
        Grid {
            anchors.centerIn: parent
            rows: 2
            columns: 2
            spacing: 20

            // Ферзь
            Rectangle {
                width: 80
                height: 80
                color: "#6D6D6D"
                border.color: "#5A5A5A"
                border.width: 1

                Image {
                    anchors.fill: parent
                    anchors.margins: 5
                    source: {
                        let side = promotionDialog.toY === 7 ? "white" : "black"
                        return resourceManager.getTexturePath(side + "Queen")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        chessEngine.promotePawn(
                            promotionDialog.fromX,
                            promotionDialog.fromY,
                            promotionDialog.toX,
                            promotionDialog.toY,
                            "queen"
                        )
                        promotionDialog.close()
                    }
                }
            }

            // Ладья
            Rectangle {
                width: 80
                height: 80
                color: "#6D6D6D"
                border.color: "#5A5A5A"
                border.width: 1

                Image {
                    anchors.fill: parent
                    anchors.margins: 5
                    source: {
                        let side = promotionDialog.toY === 7 ? "white" : "black"
                        return resourceManager.getTexturePath(side + "Rook")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        chessEngine.promotePawn(
                            promotionDialog.fromX,
                            promotionDialog.fromY,
                            promotionDialog.toX,
                            promotionDialog.toY,
                            "rook"
                        )
                        promotionDialog.close()
                    }
                }
            }

            // Слон
            Rectangle {
                width: 80
                height: 80
                color: "#6D6D6D"
                border.color: "#5A5A5A"
                border.width: 1

                Image {
                    anchors.fill: parent
                    anchors.margins: 5
                    source: {
                        let side = promotionDialog.toY === 7 ? "white" : "black"
                        return resourceManager.getTexturePath(side + "Bishop")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        chessEngine.promotePawn(
                            promotionDialog.fromX,
                            promotionDialog.fromY,
                            promotionDialog.toX,
                            promotionDialog.toY,
                            "bishop"
                        )
                        promotionDialog.close()
                    }
                }
            }

            // Конь
            Rectangle {
                width: 80
                height: 80
                color: "#6D6D6D"
                border.color: "#5A5A5A"
                border.width: 1

                Image {
                    anchors.fill: parent
                    anchors.margins: 5
                    source: {
                        let side = promotionDialog.toY === 7 ? "white" : "black"
                        return resourceManager.getTexturePath(side + "Knight")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        chessEngine.promotePawn(
                            promotionDialog.fromX,
                            promotionDialog.fromY,
                            promotionDialog.toX,
                            promotionDialog.toY,
                            "knight"
                        )
                        promotionDialog.close()
                    }
                }
            }
        }
    }

    // Диалог сохранения партии
    Dialog {
        id: saveGameDialog
        title: "Сохранить игру"
        modal: true

        background: Rectangle {
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
        }

        header: Rectangle {
            color: "#6D6D6D"
            height: 40

            Text {
                anchors.centerIn: parent
                text: saveGameDialog.title
                font.pixelSize: 18
                font.family: "Courier"
                font.bold: true
                color: "white"
            }
        }

        anchors.centerIn: Overlay.overlay

        property bool canSave: gameNameInput.text.trim().length > 0

        footer: DialogButtonBox {
            Button {
                text: "Сохранить"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: saveGameDialog.canSave
                background: Rectangle {
                    color: parent.hovered && parent.enabled ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                    opacity: parent.enabled ? 1.0 : 0.5
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Отмена"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                background: Rectangle {
                    color: parent.hovered ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        onAccepted: {
            if (canSave) {
                chessEngine.saveGame(gameNameInput.text.trim())
            }
        }

        ColumnLayout {
            width: 300
            spacing: 10

            Text {
                text: "Введите имя сохранения:"
                font.pixelSize: 14
                font.family: "Courier"
                color: "white"
            }

            TextField {
                id: gameNameInput
                Layout.fillWidth: true
                placeholderText: "Название партии"
                selectByMouse: true
                color: "white"
                background: Rectangle {
                    color: "#5A5A5A"
                    border.color: "#828282"
                    border.width: 1
                }
                onTextChanged: {
                    saveGameDialog.canSave = text.trim().length > 0
                }
            }

            Text {
                text: "Максимум 3 сохранения разрешено."
                font.pixelSize: 12
                font.family: "Courier"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#CCCCCC"
            }
        }
    }

    // Диалог загрузки партии
    Dialog {
        id: loadGameDialog
        title: "Загрузить игру"
        modal: true

        background: Rectangle {
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
        }

        header: Rectangle {
            color: "#6D6D6D"
            height: 40

            Text {
                anchors.centerIn: parent
                text: loadGameDialog.title
                font.pixelSize: 18
                font.family: "Courier"
                font.bold: true
                color: "white"
            }
        }

        anchors.centerIn: Overlay.overlay
        width: 450

        footer: DialogButtonBox {
            Button {
                text: "Закрыть"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                background: Rectangle {
                    color: parent.hovered ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        ColumnLayout {
            width: 400
            spacing: 15

            Text {
                text: "Выберите сохранение:"
                font.pixelSize: 14
                font.family: "Courier"
                color: "white"
            }

            ListView {
                id: savedGamesList
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                clip: true
                model: chessEngine.getSavedGames()

                delegate: Rectangle {
                    width: savedGamesList.width
                    height: 80
                    color: index % 2 === 0 ? "#6D6D6D" : "#5A5A5A"
                    border.color: "#828282"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        // Информация о сохранении
                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: modelData.name
                                font.bold: true
                                font.pixelSize: 14
                                font.family: "Courier"
                                color: "white"
                            }

                            Text {
                                text: "Дата: " + modelData.date
                                font.pixelSize: 12
                                font.family: "Courier"
                                color: "#CCCCCC"
                            }

                            Text {
                                text: "Режим: " + (modelData.gameMode === "twoPlayers" ? "Игра на двоих" : "Одиночная игра") +
                                      (modelData.gameMode === "vsComputer" ? " (Сложность: " +
                                      (modelData.difficulty === 1 ? "Легкий" :
                                       modelData.difficulty === 2 ? "Средний" : "Сложный") + ")" : "")
                                font.pixelSize: 12
                                font.family: "Courier"
                                color: "#CCCCCC"
                            }

                            Text {
                                text: "Статус: " + modelData.status
                                font.pixelSize: 12
                                font.family: "Courier"
                                color: "#CCCCCC"
                            }
                        }

                        // Кнопки действий
                        Column {
                            spacing: 6

                            StyledButton {
                                buttonText: "Загрузить"
                                isSmall: true
                                width: 100
                                height: 30
                                onClicked: {
                                    chessEngine.loadGame(modelData.slot)
                                    loadGameDialog.close()
                                    inMenu = false
                                    inSettings = false
                                }
                            }

                            StyledButton {
                                buttonText: "Удалить"
                                isSmall: true
                                width: 100
                                height: 30
                                onClicked: {
                                    deleteConfirmDialog.slotToDelete = modelData.slot
                                    deleteConfirmDialog.gameName = modelData.name
                                    deleteConfirmDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            Text {
                text: savedGamesList.model.length === 0 ? "Нет доступных сохранений." : ""
                font.pixelSize: 14
                font.family: "Courier"
                color: "#CCCCCC"
                Layout.alignment: Qt.AlignHCenter
                visible: savedGamesList.model.length === 0
            }
        }

        Connections {
            target: chessEngine
            function onSavedGamesChanged() {
                savedGamesList.model = chessEngine.getSavedGames()
            }
        }
    }

    // Диалог подтверждения удаления
    Dialog {
        id: deleteConfirmDialog
        title: "Удаление сохранения"
        modal: true

        background: Rectangle {
            color: "#828282"
            border.color: "#5A5A5A"
            border.width: 2
        }

        header: Rectangle {
            color: "#6D6D6D"
            height: 40

            Text {
                anchors.centerIn: parent
                text: deleteConfirmDialog.title
                font.pixelSize: 18
                font.family: "Courier"
                font.bold: true
                color: "white"
            }
        }

        anchors.centerIn: Overlay.overlay

        property int slotToDelete: -1
        property string gameName: ""

        footer: DialogButtonBox {
            Button {
                text: "Да"
                DialogButtonBox.buttonRole: DialogButtonBox.YesRole
                background: Rectangle {
                    color: parent.hovered ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Нет"
                DialogButtonBox.buttonRole: DialogButtonBox.NoRole
                background: Rectangle {
                    color: parent.hovered ? "#6D6D6D" : "#828282"
                    border.color: "#5A5A5A"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.family: "Courier"
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        onAccepted: {
            if (slotToDelete >= 0) {
                chessEngine.deleteGame(slotToDelete)
            }
        }

        Text {
            text: "Вы уверены, что хотите удалить\n\"" + deleteConfirmDialog.gameName + "\"?"
            font.pixelSize: 14
            font.family: "Courier"
            color: "white"
            wrapMode: Text.WordWrap
            width: 300
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
