import QtQuick 2.12
import QtQuick.Controls 2.12

Page {
    background: Rectangle{
        color: "#eccba7"
        anchors.fill: parent
    }

    header: Rectangle {
        id: head
        height: parent.height * 0.1
        width: parent.width

        color: "#4b7d8d"
        Rectangle {
            z: 1000
            id: inputBox
            height: parent.height
            width: parent.width - parent.width * 0.26
            color: "#166F8C"
            x: parent.width * 0.13
            radius: 27
            TextInput {
                color: "white"
                x: inputBox.width * 0.05
                height: parent.height
                width: inputBox.width * 0.9
                verticalAlignment: TextInput.AlignVCenter
                font.pixelSize: Qt.application.font.pixelSize * 1.8
                opacity: 0.65
                topPadding: 10
                bottomPadding: 10
                clip: true
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                onTextEdited: {
                    category.setFilterFixedString(text)
                }
            }
        }
    }

    ListView {
        id: catView
        anchors.fill: parent
        antialiasing: true
        model: category

        delegate: Button {
            id:button
            width: parent.width
            height: width / 6

            background: Rectangle {
                anchors.fill: parent
                color: button.down ? "#d6d6d6" : "#bbd5db"
                opacity: 0.7
            }

            Text {
                id: code
                text: getCat(name)
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill:parent
                color: "#1f4752"
                font.pointSize: 15
            }

            Component {
                id: prodSearchForm
                ProductSearch {
                    catID: id
                }
            }

            onClicked: {
                stack.push(prodSearchForm)
            }

            function getCat(str) {
                var parts = str.split('/');
                return parts[parts.length - 1]
            }
        }
    }
}
