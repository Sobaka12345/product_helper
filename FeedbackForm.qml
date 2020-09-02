import QtQuick 2.0
import QtQuick.Controls 2.12

Item {
    property string comments: ""
    Rectangle {
        id: commentSection
        color: "#4b7d8d"
        width: parent.width
        height: parent.height * 0.25

        Rectangle {
            color: "white"
            width: parent.width * 0.9
            x: parent.width * 0.05
            height: parent.height * 0.90
            y: parent.height * 0.05
            radius: 10
            TextInput {
                id:comment
                maximumLength: 255
                width: parent.width * 0.72
                x: parent.width * 0.04
                height: parent.height * 0.94
                y: parent.height * 0.03
                clip: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            Button {
                id: deleteComment

                background: Rectangle {
                    radius: 10
                    anchors.fill: parent
                    color: deleteComment.down ? "#d6d6d6" : "#bbd5db"
                }
                Rectangle {
                    x:parent.width/2
                    width: parent.width/2
                    height: parent.height
                    color: deleteComment.down ? "#d6d6d6" : "#bbd5db"
                }
                z:100
                width: parent.width * 0.10
                x: 0
                height: parent.height
                y:0
                text: "❌"
                onClicked: {
                    confirmDeletion.open()
                }
            }

            Button {
                id: sendComment

                background: Rectangle {
                    radius: 10
                    anchors.fill: parent
                    color: sendComment.down ? "#d6d6d6" : "#bbd5db"
                }
                Rectangle {
                    x:0
                    width: parent.width/2
                    height: parent.height
                    color: sendComment.down ? "#d6d6d6" : "#bbd5db"
                }
                z:100
                width: parent.width * 0.10
                x: parent.width * 0.90
                height: parent.height
                y:0
                text: ">>"
                onClicked: {
                    var i = 4
                    for(; i >= 0; i--) {
                        if (repStars.itemAt(i).full === true)
                            break;
                    }
                    var trimmed = comment.text.trim()
                    if(trimmed.length === 0)
                        product.sendFeedback(i + 1, listRect.code)
                    else
                        product.sendFeedback(i + 1, listRect.code, trimmed)
                    product.requestComments(listRect.code)
                }
            }

            Rectangle {
                id: stars
                width: parent.width * 0.10
                x: parent.width * 0.8
                height: parent.height

                color: "#bbd5db"
                Column {
                    anchors.fill: parent

                    Repeater {
                        id: repStars
                        model: 5
                        Button {
                            property bool full: false
                            height: stars.height / 5;
                            width: stars.width
                            background: null
                            onFullChanged: {
                                emptyStar.visible = !full
                                fullStar.visible = full
                            }

                            Image {
                                fillMode: Image.PreserveAspectFit
                                id: emptyStar
                                height: parent.height
                                source: "qrc:/pics/star_empty.png"
                                x: parent.width / 2 - width / 2
                            }
                            Image {
                                fillMode: Image.PreserveAspectFit
                                id: fullStar
                                height: parent.height
                                visible: false
                                source: "qrc:/pics/star_full.png"
                                x: parent.width / 2 - width / 2
                            }
                            onClicked: {
                                if(fullStar.visible) {
                                    var idx = 4
                                    for(; idx >= index; idx--)
                                        if(repStars.itemAt(idx).full === true)
                                            break;
                                    if(idx === index)
                                        for(var i = 0; i < 5; i++) {
                                            repStars.itemAt(i).full = false
                                        }
                                    else {
                                        for(var i = index + 1; i <= idx; i++) {
                                            repStars.itemAt(i).full = false
                                        }
                                    }
                                } else {
                                    for(var i = 0; i <= index; i++)
                                    {
                                        repStars.itemAt(i).full = true
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    //c:

    }

    Rectangle {
        id: feedbacks
        y: parent.y + commentSection.height
        width: parent.width
        height: parent.height - commentSection.height
        ProductParameters {
            height: parent.height
            width: parent.width
            txt: comments
            fontSize: 16
            paddingView: 0
        }
    }

    Popup {
        id:confirmDeletion
        x: parent.width * 0.1
        width: parent.width * 0.8
        height: parent.height *0.3
        y: parent.height / 2 - height / 2
        modal: true
        background: Rectangle {
            color: "#eccba7"
            anchors.fill: parent
        }
        Text {
            id: name
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
            y: parent.height * 0.1
            text: qsTr("Удалить комментарий?")
        }
        Button {
            id: yes
            x: parent.width * 0.05
            width: parent.width * 0.4
            height: parent.height * 0.6
            y: parent.height * 0.35
            text: "Да"
            onClicked: {
                product.sendFeedback(-1, listRect.code)
                confirmDeletion.close()
            }
        }
        Button {
            id: no
            x: parent.width * 0.55
            width: parent.width * 0.4
            height: parent.height * 0.6
            y: parent.height * 0.35
            text: "Нет"
            onClicked: confirmDeletion.close()
        }
    }
}
