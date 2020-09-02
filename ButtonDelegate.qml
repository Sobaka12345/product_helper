import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    property string nugrade: ""
    property string nova: ""
    property string name: ""
    property string brand: ""

    id:listButton
    width: parent.width
    height: width / 5

    background: Rectangle {
        anchors.fill: parent
        color: listButton.down ? "#d6d6d6" : "#bbd5db"
        opacity: 0.7
    }


    Rectangle {
        id:nutriGrade
        visible: nugrade != undefined && nugrade != 0
        width: parent.height * 0.35
        height: parent.height * 0.35
        radius: parent.height * 0.35
        x: height*1.3 - height / 1.5
        y: parent.height - height*1.3
        color: nugrade=="a" ? "#038141" : nugrade=="b"? "#85bb2f" : nugrade=="c" ? "#fecb02": nugrade == "d" ? "#ee8100" : "#e63e11"
        Text{
            id: score
            text: nugrade
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: parent.height * 0.68
            font.weight: Font.ExtraBold
            color: "white"
        }
    }

    Rectangle {
        id:novaGroup
        visible: nova != undefined && nova != 0
        width: parent.height * 0.28
        height: parent.height * 0.35
        x: height*1.3 - height / 1.5 + (nutriGrade.visible ? nutriGrade.x + nutriGrade.width : 0)
        y: parent.height - height*1.3
        color: nova=="1" ? "#00aa00" : nova=="2" ? "#ffcc00" : nova=="3" ? "#ff6600": "#ff0000"
        Text{
            id: group
            text: nova
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: parent.height * 0.68
            font.weight: Font.ExtraBold
            color: "white"
        }
    }

    Text {
        id: prodName
        padding: 10
        clip: true
        text: name != undefined? name : "No name"
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        anchors.fill:parent
        color: "#1f4752"
        font.pointSize: 15
    }
    Text {
        visible: brand != undefined
        id: prodBrand
        padding: 10
        clip: true
        text: brand
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignRight
        anchors.fill:parent
        color: "#1f4752"
        font.pointSize: 15
    }

    Rectangle {
        id: bottomLine
        anchors.bottom: parent.bottom
        width: parent.width
        height: 0.7
        color: "black"
        z:100
    }
}
