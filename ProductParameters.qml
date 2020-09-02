import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    property string txt: ""
    property int fontSize: 20
    property int paddingView: 15
ScrollView {
    background: Rectangle {
        color: "#eccba7"
        anchors.fill: parent
    }
    height: parent.height
    width: parent.width
    ScrollBar.horizontal.policy : ScrollBar.AlwaysOff
    clip: true
    contentWidth: -1
    padding: paddingView

    Text {
        id:innerText
        width: parent.width
        textFormat: Text.RichText
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: fontSize
        text: txt
    }
}
}
