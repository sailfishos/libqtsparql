import Qt 4.7

Rectangle {
    width: 360
    height: 360

    ListView {
        id: contactsView
        anchors.fill: parent
        model: contactModel

        delegate: Text { text: "ID = " + urn + "\nSubject = " + subject + "\n" }
    }
}
