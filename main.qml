import QtQuick 2.15
import QtQuick.Window 2.15
//import shortcut.example 1.0

Window {
    id: mainWindow
    width: 1024
    height: 768
    visible: true
    title: qsTr("Hello World")


    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            var route, m, pathLine;
            if(mouse.button == Qt.LeftButton) {

                if(mouse.modifiers == Qt.ControlModifier) {
                    var unitPos = objectList.findPosNode(Qt.point(mouse.x, mouse.y))
                    unit.x = unitPos.x
                    unit.y = unitPos.y
                }
                else {
                    objectList.addObject(Qt.point(mouse.x, mouse.y))
                    pathAnimation.stop();
                    path.pathElements = []
                }
            }
            else {
                pathAnimation.stop();
                path.pathElements = []

                route = objectList.findRoute(Qt.point(unit.x, unit.y), Qt.point(mouse.x, mouse.y))

                for(m = 0; m < route.length; m++) {
                    pathLine = Qt.createQmlObject("import QtQuick 2.15; PathLine {x: " + route[m].x + "; y: " + route[m].y + "}", path, "pathLine")
                    path.pathElements.push(pathLine)
                }

                pathAnimation.duration = route.length * 200
                pathAnimation.start()
            }
        }
    }

    Repeater {
        id: repeater
        model: objectList
        delegate: Rectangle {
            id: gamePolygon
            required property rect position
            x: position.x
            y: position.y
            z: 0
            width: position.width
            height: position.height
            border.width: 1
            border.color: "#30000000"
            color: "transparent"

            Connections {
                target: objectList
                function onHighlightGreen(position) {
                    if(gamePolygon.x === position.x && gamePolygon.y === position.y) {
                        gamePolygon.z = 1
                        gamePolygon.border.color = "#2c9b2d"
                        gamePolygon.border.width = 2
                    }
                }

                function onHighlightRed(position) {
                    if(gamePolygon.x === position.x && gamePolygon.y === position.y)  {
                        gamePolygon.z = 3
                        gamePolygon.color = "#80ff0000"
                        gamePolygon.border.color = "#ff0000"
                        gamePolygon.border.width = 2
                    }
                }

                function onHighlightBlue(position) {
                    if(gamePolygon.x === position.x && gamePolygon.y === position.y)  {
                        gamePolygon.z = 2
                        gamePolygon.border.color = "#0000FF"
                        gamePolygon.border.width = 2
                    }
                }

                function onRemoveNode(position) {
                    if(gamePolygon.x === position.x && gamePolygon.y === position.y)
                       gamePolygon.color = "black"
                }
            }
        }
    }

    Rectangle {
        id: unit
        property var routeList: [ ]
        width: 32
        height: 32
        x: 0
        y: 0
        z: 100
        radius: 32
        color: "green"
    }

    PathAnimation {
        id: pathAnimation
        target: unit
        duration: 1000
        path: Path { id: path }
    }

    Component.onCompleted: {
        objectList.setMapSize(Qt.size(12000, 12000))
    }
}
