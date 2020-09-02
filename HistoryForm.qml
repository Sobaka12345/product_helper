import QtQuick 2.12
import QtQuick.Controls 2.5

Page {
    id: page
    width: 600
    height: 400
    antialiasing: true
    clip: true
    background: Rectangle{
        color: "#eccba7"
        anchors.fill: parent
    }

    header: Label {
        text: qsTr("История просмотров")
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10
        color: "white"
        height: parent.height * 0.1
        width: parent.width
        background:Rectangle {
            color: "#4b7d8d"
            anchors.fill: parent
        }
    }


    StackView {
        id: stack
        Keys.onBackPressed: {
            if(depth > 1) pop()
            else Qt.quit()
        }
        width: parent.width
        height: parent.height
        anchors.fill: parent
        initialItem: historyList
    }

    Component {
        id:  historyList
        ListView {
            id: hisView
            anchors.fill: parent
            antialiasing: true
            model: history

            delegate: ButtonDelegate {
                nugrade: grade
                name: Name
                brand: BrandName
                nova: nova_group
                Component {
                    id: productCompHis
                    ProductInfo {
                        height: parent.height
                        attributes: FACTS
                        researches: RK
                        code: barcode
                        comments: product.comments
                    }
                }

                onClicked: {
                   stack.push(productCompHis)
                   product.requestComments(code)
                }
            }
        }
    }

}
