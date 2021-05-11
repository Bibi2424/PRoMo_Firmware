# From https://stackoverflow.com/questions/55070483/connect-to-serial-from-a-pyqt-gui

import time

from collections import deque
from PyQt5 import QtCore, QtWidgets, QtSerialPort



class SerialWidget(QtWidgets.QWidget):
    def __init__(self, parent=None, callback=None):
        super(SerialWidget, self).__init__(parent)
        self.callback = callback

        self.port_input = QtWidgets.QLineEdit()
        self.baudrate_input = QtWidgets.QLineEdit()
        self.message_le = QtWidgets.QLineEdit()
        self.send_btn = QtWidgets.QPushButton(
            text="Send",
            clicked=self.send
        )
        self.output_text = QtWidgets.QTextEdit(readOnly=True)
        self.connect_button = QtWidgets.QPushButton(
            text="Connect", 
            checkable=True,
            toggled=self.on_toggled
        )

        lay = QtWidgets.QVBoxLayout(self)
        hlay_connect_option = QtWidgets.QHBoxLayout()
        hlay_connect_option.addWidget(self.port_input)
        hlay_connect_option.addWidget(self.baudrate_input)
        hlay_send = QtWidgets.QHBoxLayout()
        hlay_send.addWidget(self.message_le)
        hlay_send.addWidget(self.send_btn)

        lay.addLayout(hlay_connect_option)
        lay.addWidget(self.connect_button)
        # lay.addLayout(hlay_send)
        lay.addWidget(self.output_text)

        self.serial = QtSerialPort.QSerialPort(
            'COM10',
            baudRate=921600,
            readyRead=self.receive
        )
        self.port_input.setText('COM10')
        self.baudrate_input.setText('921600')

        self.text_received = deque(maxlen=10)

    @QtCore.pyqtSlot()
    def receive(self):
        while self.serial.canReadLine():
            try:
                text = self.serial.readLine().data().decode()
            except UnicodeDecodeError:
                continue
            text = text.rstrip('\r\n')
            self.text_received.append(text)
            # self.output_text.append(text)
            self.output_text.clear()
            self.output_text.setPlainText('\r\n'.join((self.text_received)))

            if text.startswith('NRF:'):
                r = map(int, text[5:].split(' - '))
                if self.callback: 
                    self.callback(time.time(), list(r))

    @QtCore.pyqtSlot()
    def send(self):
        self.serial.write(self.message_le.text().encode())

    @QtCore.pyqtSlot(bool)
    def on_toggled(self, checked):
        self.connect_button.setText("Disconnect" if checked else "Connect")
        if checked:
            if not self.serial.isOpen():
                self.serial.setPortName(self.port_input.text())
                self.port_input.setReadOnly(True)
                self.serial.setBaudRate(int(self.baudrate_input.text()))
                self.baudrate_input.setReadOnly(True)
                if not self.serial.open(QtCore.QIODevice.ReadWrite):
                    self.connect_button.setChecked(False)
        else:
            self.serial.close()
            self.port_input.setReadOnly(False)
            self.baudrate_input.setReadOnly(False)
            self.output_text.clear()

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    w = SerialWidget()
    w.show()
    sys.exit(app.exec_())