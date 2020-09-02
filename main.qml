import QtQuick 2.12
import QtQuick.Controls 2.12
import QtAndroidTools 1.0

ApplicationWindow {
    id: app
    visible: true
    width: 640
    height: 480

    Connections {
        target: QtAndroidGoogleAccount

        onSignedInAccountChanged: {
             product.setUserEmail(QtAndroidGoogleAccount.signedInAccount.email)
        }

        onSignedIn: {
            if(signInSuccessfully === true)
            {
                login.close()
            } else
            {
                login.open()
            }
        }

    }

    onVisibleChanged: {
        QtAndroidGoogleAccount.signIn("https://www.googleapis.com/auth/userinfo.email")
    }

    Popup {
        id: login
        background: Rectangle {
            color: "#eccba7"
            anchors.fill: parent
        }

        width: parent.width
        height: parent.height + tabBar.height
        modal: true
        closePolicy: Popup.NoAutoClose
        Button {
            id: loginButton
            width: parent.width * 0.65
            height: width * 0.3
            anchors.centerIn: parent
            text: "Войти через Google"
            onClicked: QtAndroidGoogleAccount.signInSelectAccount("https://www.googleapis.com/auth/userinfo.email")
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: {
            if(currentIndex == 0) {
                scanForm.changeCamera(true)
                scanForm.forceActiveFocus()
            }
            else if(currentIndex == 1) {
                scanForm.changeCamera(false)
                historyForm.forceActiveFocus()
            }
            else if(currentIndex == 2) {
                scanForm.changeCamera(false)
                productsForm.forceActiveFocus()
            }

        }

        ScanForm {
            id: scanForm
        }

        HistoryForm {
            id: historyForm
        }

        ProductsForm{
            id: productsForm
        }

    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        font.pointSize: 13

        TabButton {
            text: qsTr("Сканировать")
        }

        TabButton {
            text: qsTr("История")
        }

        TabButton {
            text: qsTr("Продукты")
        }
    }


}
