import QtQuick 2.12
import QtMultimedia 5.12
import QtQuick.Controls 2.5

Page {
    width: 600
    height: 400

    header: null

    background: Rectangle{
        color: "#eccba7"
        anchors.fill: parent
    }

    VideoOutput {
        id: video
        anchors.fill: parent
        filters: [filter]
        source: camera
        autoOrientation: true
        focus: visible
        fillMode: VideoOutput.PreserveAspectCrop
    }

    Camera {
        id: camera
        function autoFocus() {
              focus.focusMode = Camera.FocusContinuous;
              focus.focusPointMode = Camera.FocusPointAuto;
          }

          /*property var autoFocusTimer: Timer {
              interval: 5000
              onTriggered: camera.autoFocus();
          }*/

          focus {
              focusMode: Camera.FocusContinuous
              focusPointMode: Camera.FocusPointAuto
          }
    }



    SwipePane{
        id: sidePane
        menuHeight: app.height - tabBar.height
        z: 2
        itemVisible: false
    }

    function changeCamera(flag) {
        if(flag)
        {
            if(!sidePane.menuVisible)
                camera.start();
        }
        else
            camera.stop();
    }
}
