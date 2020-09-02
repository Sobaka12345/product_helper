import QtQuick 2.0
import QtQuick.Controls 2.12


Item {
    property string researches: ""
    property string attributes: ""
    property string code: ""
    property string comments: ""

    id: listRect
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right

    clip: true

    TabBar {
        id: bar
        width: parent.width
        currentIndex: view.currentIndex
        TabButton {
            text: qsTr("Параметры")
        }
        TabButton {
            text: qsTr("Анализ")
        }
        TabButton {
            text: qsTr("Отзывы")
        }
    }

    SwipeView {
        id: view
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height - bar.height
        currentIndex: bar.currentIndex

        ProductParameters {
            id: benis1
            txt: listRect.attributes
        }

        ProductParameters {
            id: benis2
            txt: listRect.researches
        }

        FeedbackForm {
            id: benis3
            comments: listRect.comments
        }
    }

    onCodeChanged: product.requestComments(code)
}
