import QtQuick 2.12
import QtQuick.Controls 2.12

Page {
    width: 600
    height: 400

    StackView {
        id: stack
        Keys.onBackPressed: {
            if(depth > 1) pop()
            else Qt.quit()
        }
        width: parent.width
        height: parent.height
        anchors.fill: parent
        initialItem: CategoryList {}
    }


}
