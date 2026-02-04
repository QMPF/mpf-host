import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Single menu item in the side menu
 */
Rectangle {
    id: root
    
    property string itemId: ""
    property string title: ""
    property string icon: ""
    property string route: ""
    property string badge: ""
    property bool enabled: true
    property bool selected: false
    property bool expanded: true
    
    signal clicked()
    
    implicitHeight: 48
    radius: Theme ? Theme.radiusSmall : 4
    
    color: {
        if (!enabled) return "transparent"
        if (selected) return Qt.alpha(Theme ? Theme.primaryColor : "#2196F3", 0.15)
        if (mouseArea.containsMouse) return Qt.alpha(Theme ? Theme.textColor : "#212121", 0.08)
        return "transparent"
    }
    
    Behavior on color {
        ColorAnimation { duration: 150 }
    }
    
    // Selection indicator
    Rectangle {
        visible: root.selected
        width: 4
        height: parent.height - 16
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        radius: 2
        color: Theme ? Theme.primaryColor : "#2196F3"
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
        
        onClicked: {
            if (root.enabled) {
                root.clicked()
            }
        }
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 12
        spacing: 12
        
        // Icon
        Text {
            text: root.icon || "📦"
            font.pixelSize: 20
            opacity: root.enabled ? 1.0 : 0.5
            
            Layout.preferredWidth: 24
            horizontalAlignment: Text.AlignHCenter
        }
        
        // Title
        Label {
            text: root.title
            font.pixelSize: 14
            font.weight: root.selected ? Font.Medium : Font.Normal
            color: {
                if (!root.enabled) return Theme ? Theme.textSecondaryColor : "#757575"
                if (root.selected) return Theme ? Theme.primaryColor : "#2196F3"
                return Theme ? Theme.textColor : "#212121"
            }
            elide: Text.ElideRight
            visible: root.expanded
            
            Layout.fillWidth: true
        }
        
        // Badge
        Rectangle {
            visible: root.badge.length > 0 && root.expanded
            implicitWidth: Math.max(20, badgeLabel.implicitWidth + 8)
            implicitHeight: 20
            radius: 10
            color: Theme ? Theme.accentColor : "#FF4081"
            
            Label {
                id: badgeLabel
                anchors.centerIn: parent
                text: root.badge
                font.pixelSize: 11
                font.weight: Font.Medium
                color: "white"
            }
        }
    }
    
    // Tooltip when collapsed
    ToolTip {
        visible: !root.expanded && mouseArea.containsMouse
        text: root.title + (root.badge ? " (" + root.badge + ")" : "")
        delay: 500
    }
}
