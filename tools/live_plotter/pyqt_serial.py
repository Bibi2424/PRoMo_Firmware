# From https://stackoverflow.com/questions/55070483/connect-to-serial-from-a-pyqt-gui

import time

from collections import deque
from PyQt5 import QtCore, QtWidgets, QtSerialPort



class SerialWidget(QtWidgets.QWidget):
    def __init__(self, window=None, callback=None, show_data_draw = True):
        super(SerialWidget, self).__init__(window)
        self.window = window
        self.callback = callback
        self.show_data_draw = show_data_draw

        self.port_input = QtWidgets.QLineEdit()
        self.baudrate_input = QtWidgets.QLineEdit()
        self.message_le = QtWidgets.QLineEdit()
        self.send_btn = QtWidgets.QPushButton(
            text="Send",
            clicked=lambda: self.write(self.message_le.text())
        )
        self.output_text = QtWidgets.QTextEdit(readOnly=True)
        self.connect_button = QtWidgets.QPushButton(
            text="Connect", 
            checkable=True,
            toggled=self.on_connect
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
        lay.addLayout(hlay_send)
        lay.addWidget(self.output_text)

        self.serial = QtSerialPort.QSerialPort(
            'COM17',
            baudRate=921600,
            readyRead=self.receive
        )
        self.port_input.setText('COM17')
        self.baudrate_input.setText('921600')

        self.text_received = deque(maxlen=100)


    def message(self, message_str, message_time = 1000):
        if self.window:
            self.window.statusBar().showMessage(message_str, message_time)


    @QtCore.pyqtSlot()
    def receive(self):
        while self.serial.canReadLine():
            try:
                text = self.serial.readLine().data().decode()
            except UnicodeDecodeError:
                continue
            text = text.rstrip('\r\n')
            # print(text)

            used = False
            if self.callback:
                used = self.callback(text)

            if used == True and self.show_data_draw:
                continue

            self.text_received.append(text)
            self.output_text.clear()
            self.output_text.setPlainText('\r\n'.join(reversed(self.text_received)))



    @QtCore.pyqtSlot()
    def write(self, data):
        self.serial.write(data.encode())


    @QtCore.pyqtSlot(bool)
    def on_connect(self, checked):
        print(f'{"Connecting" if checked else "Disconnecting"} {self.port_input.text()}@{self.baudrate_input.text()}')
        self.connect_button.setText("Disconnect" if checked else "Connect")
        if checked:
            if not self.serial.isOpen():
                self.output_text.clear()
                self.serial.setPortName(self.port_input.text())
                self.port_input.setReadOnly(True)
                self.serial.setBaudRate(int(self.baudrate_input.text()))
                self.baudrate_input.setReadOnly(True)
                if not self.serial.open(QtCore.QIODevice.ReadWrite):
                    self.connect_button.setChecked(False)
                    self.message(f'Error connecting to {self.port_input.text()}', 2000)
                    return

            self.message(f'{self.port_input.text()}@{self.baudrate_input.text()} connected', 2000)
        else:
            self.serial.close()
            self.port_input.setReadOnly(False)
            self.baudrate_input.setReadOnly(False)

            self.message(f'{self.port_input.text()} disconnected', 2000)



if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    w = SerialWidget()
    w.show()
    sys.exit(app.exec_())
