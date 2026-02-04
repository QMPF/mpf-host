import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    
    property string errorTitle: qsTr("Error")
    property string errorMessage: ""
    
    title: errorTitle
    modal: true
    standardButtons: Dialog.Ok
    
    anchors.centerIn: parent
    width: Math.min(400, parent.width - 40)
    
    background: Rectangle {
        color: Theme ? Theme.surfaceColor : "#F5F5F5"
        radius: Theme ? Theme.radiusMedium : 8
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: Theme ? Theme.spacingMedium : 16
        
        RowLayout {
            spacing: Theme ? Theme.spacingSmall : 8
            
            Label {
                text: "⚠️"
                font.pixelSize: 24
            }
            
            Label {
                text: root.errorTitle
                font.pixelSize: 18
                font.bold: true
                color: Theme ? Theme.errorColor : "#F44336"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
        
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 200)
            
            TextArea {
                text: root.errorMessage
                readOnly: true
                wrapMode: Text.Wrap
                color: Theme ? Theme.textColor : "#212121"
                background: Rectangle {
                    color: Theme ? Theme.backgroundColor : "#FFFFFF"
                    radius: Theme ? Theme.radiusSmall : 4
                }
            }
        }
    }
    
    onAccepted: close()
}
