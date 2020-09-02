import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
    property int catID: -1
    onCatIDChanged: {
        product.update(catID)
    }

    id: searchPage
    width: parent.width
    height: parent.height

    background: Rectangle{
        color: "#eccba7"
        anchors.fill: parent
    }

    header: Rectangle{
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
                id:headSrchTI
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
            }
        }

        Button {
            id: gearButton
            height: parent.height
            width: inputBox.x + inputBox.width / 2
            background: Rectangle {
                id: gearButtonBG
                color: head.color
                anchors.fill: parent
            }
            Image {
                id: gear
                y: parent.height * 0.20
                x: 8
                opacity: 0.65
                height: parent.height*0.6
                fillMode: Image.PreserveAspectFit
                source: "qrc:/pics/settings.svg"
                antialiasing: true
            }

            onPressed: {
                gearButtonBG.color = "#bbd5db"
            }

            onReleased: {
                gearButtonBG.color = head.color
            }

            onClicked: {
                searchPane.opacity ? hidePane.start() : showPane.start()
            }
        }

        Button {
            id: loupeButton
            height: parent.height
            padding: 0
            x: inputBox.x + inputBox.width - inputBox.width / 2
            width: head.width - (inputBox.x + inputBox.width) + inputBox.width / 2
            background: Rectangle {
                id: loupeButtonBG
                color: head.color
                anchors.fill: parent
            }
            Image {
                id: loupe
                y: parent.height * 0.15
                x: parent.width * 0.77
                height: parent.height*0.7
                fillMode: Image.PreserveAspectFit
                source: "qrc:/pics/loupe.svg"
                antialiasing: true
            }

            onPressed: {
                loupeButtonBG.color = "#bbd5db"
            }

            onReleased: {
                loupeButtonBG.color = head.color
            }

            onClicked: {
                var accum = []
                if(headSrchTI.text != "")
                    accum.push("text=" + headSrchTI.text)
                if(comboBox.currentText != "")
                    accum.push("text=" + comboBox.currentText)
                for(var child in params.contentItem.children) {
                    var key = params.contentItem.children[child].key
                    if(key === undefined || key === "")
                        continue
                    var val = params.contentItem.children[child].value
                    accum.push(key + "=" + val)
                }
                product.sendQuery(accum.join("&"), catID)
            }
        }
    }

    ParallelAnimation {
        id: hidePane
        running: false
        alwaysRunToEnd: true
        NumberAnimation { target: searchPane; property: "height"; to: 0; duration: 200 }
        NumberAnimation { target: searchPane; property: "opacity"; to: 0; duration: 200 }
    }

    ParallelAnimation {
        id: showPane
        running: false
        alwaysRunToEnd: true
        NumberAnimation { target: searchPane; property: "height"; to: searchPane.paneHeight; duration: 200 }
        NumberAnimation { target: searchPane; property: "opacity"; to: 1; duration: 200 }
    }

    Pane {
        property double paneHeight: contentHeight + 3 * padding + comboBox.height

        id: searchPane
        background: Rectangle {
            color: "#1f4752"
        }

        height: 0
        opacity: 0
        width: parent.width
        Column {
            anchors.fill: parent
            ComboBox {
                id: comboBox
                width: parent.width
                height: 50

                model: product.getLabels(catID)
                currentIndex: -1

                displayText: outFormat(currentText)

                delegate: RowLayout {
                    width: parent.width
                    ItemDelegate {
                        id: item
                        Layout.fillWidth: true
                        implicitHeight: combText.implicitHeight
                        height: combText.implicitHeight
                        Layout.minimumHeight: 50

                        onReleased: {
                            comboBox.currentIndex = index
                            comboBox.popup.close()
                        }

                        background: Rectangle {
                            anchors.fill: parent
                            Rectangle{
                                color: "black"
                                width: parent.width
                                anchors.bottom: parent.bottom
                                height: 0.7
                            }

                            color: item.down ? "#dddedf" : "#eeeeee"

                        }

                        Text {
                            id: combText
                            anchors.centerIn: parent
                            font.pixelSize: 20
                            width: parent.width
                            text: comboBox.outFormat(modelData)
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            clip: true
                        }
                    }
                }

                function outFormat(kek) {
                    return kek.replace("ru:", "").split("-").join(" ")
                }
            }

            ListView {
                property Button foot: null
                id: params
                interactive: false
                y: comboBox.y + comboBox.height + searchPane.padding
                height: model.count * (comboBox.height + 2*searchPane.padding) + comboBox.height

                function push(el) {
                    if(model.count < 3) {
                        model.append(el)

                        if(model.count === 3) {
                            foot.visible = false
                            foot.height = 0
                        } else {
                            params.height = params.height + comboBox.height + searchPane.padding
                            searchPane.contentHeight = searchPane.contentHeight + comboBox.height
                            searchPane.height = searchPane.paneHeight
                        }
                    }
                }

                function removeElement(id) {
                    if(model.count === 3 || model.count === 0) {
                        foot.visible = true
                        foot.height = comboBox.height
                    } else {
                        params.height = params.height - comboBox.height + searchPane.padding
                        searchPane.contentHeight = searchPane.contentHeight - comboBox.height
                        searchPane.height = searchPane.paneHeight
                    }
                    model.remove(id)
                }

                model : ListModel {
                    id: paramsModel
                }

                delegate: Row {
                    id:rowD
                    property string value: ""
                    property string key: ""
                    property bool contains: true
                    width: comboBox.width
                    height: comboBox.height
                    spacing: width * 0.025
                    ListModel {
                        id:comboSearchModel
                        ListElement {
                            orig: "OFF.additives_tags"
                            name: "Добавки"
                        }
                        ListElement {
                            orig: "OFF.allergens"
                            name: "Аллергены"
                        }
                        ListElement {
                            orig: "OFF.ingredients_tags"
                            name: "Ингредиенты"
                        }
                        ListElement {
                            orig: "OFF.countries_tags"
                            name: "Страны"
                        }
                    }

                    ComboBox {
                        id: vars
                        width: parent.width * 0.4
                        height: parent.height

                        model:comboSearchModel

                        textRole: "name"

                        onCurrentTextChanged: {
                            rowD.key = comboSearchModel.get(currentIndex).orig
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.45
                        height: parent.height* 0.75

                        color: "white"
                        radius: 3
                        y:parent.height * 0.125
                        TextInput {

                            clip: true
                            font.pixelSize: Qt.application.font.pixelSize * 1.7
                            x: parent.width * 0.025
                            width: parent.width * 0.95
                            height: parent.height
                            onTextChanged: {
                                rowD.value = text
                            }
                        }
                    }
                    Button {
                        id:rem
                        width: parent.width * 0.1

                        text: "X"
                        onClicked: params.removeElement(index)
                    }
                }

                Component {
                    id:lalka
                    Button {
                           id:tratata
                           height: comboBox.height
                           width: comboBox.width
                           text: "+"
                           onClicked: {
                               params.push({"kek": "lol"})
                           }

                           Component.onCompleted: params.foot = tratata
                       }
                }

                footer: lalka
            }

        }
    }

    ListView {
       id: productItems
       y: searchPane.y + searchPane.height
       width: parent.width
       height: parent.height - y
       clip: true
       model: product

       footer: Button {
           id:footButton
           width: parent.width
           height: width / 6

           background: Rectangle {
               anchors.fill: parent
               color: footButton.down ? "#d6d6d6" : "#bbd5db"
               opacity: 0.7
           }

           Text {
               id: endButton
               text: "ShowMore"
               verticalAlignment: Text.AlignVCenter
               horizontalAlignment: Text.AlignHCenter
               anchors.fill:parent
               color: "#1f4752"
               font.pointSize: 15
           }

           onClicked: {
               product.fetchMore()
               if(!product.canFetch()) {
                   footButton.visible = false
                   footButton.height = 0
               }
           }
       }

       delegate: ButtonDelegate {
           id : listButton
           nova: nova_group
           name: Name
           brand: BrandName
           nugrade: grade

           Component {
               id: productComp
               ProductInfo {
                   height: parent.height
                   attributes: FACTS
                   researches: RK
                   code: barcode
                   comments: product.comments
               }
           }

           onClicked: {
               stack.push(productComp)
           }
       }
    }
}
