# Modified from https://courses.ideate.cmu.edu/16-223/f2020/text/code/MQTT-Monitor.html

import time

from collections import deque
from PyQt5 import QtCore, QtWidgets
import paho.mqtt.client as mqtt


class ReceiveSignal(QtCore.QObject):
    ''' Why a whole new class? See here: 
    https://stackoverflow.com/a/25930966/2441026 '''
    sig_mqtt_message = QtCore.pyqtSignal(mqtt.MQTTMessage)


class MQTTWidget(QtWidgets.QWidget):
    def __init__(self, window=None, callback=None, show_data_draw = True):
        super(MQTTWidget, self).__init__(window)
        self.window = window
        self.callback = callback
        self.show_data_draw = show_data_draw

        self.url_input = QtWidgets.QLineEdit()
        self.port_input = QtWidgets.QLineEdit()
        self.message_le = QtWidgets.QLineEdit()
        self.send_btn = QtWidgets.QPushButton(
            text="Send",
            clicked=lambda: self.write(self.message_le.text())
        )
        self.output_text = QtWidgets.QTextEdit(readOnly=True)
        self.connect_button = QtWidgets.QPushButton(
            text="Connect", 
            checkable=True,
            toggled=self.on_toggled
        )

        lay = QtWidgets.QVBoxLayout(self)
        hlay_connect_option = QtWidgets.QHBoxLayout()
        hlay_connect_option.addWidget(self.url_input)
        hlay_connect_option.addWidget(self.port_input)
        hlay_send = QtWidgets.QHBoxLayout()
        hlay_send.addWidget(self.message_le)
        hlay_send.addWidget(self.send_btn)

        lay.addLayout(hlay_connect_option)
        lay.addWidget(self.connect_button)
        lay.addLayout(hlay_send)
        lay.addWidget(self.output_text)

        self.message_signal = ReceiveSignal()
        self.message_signal.sig_mqtt_message.connect(self.process_message)

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.on_message = self.on_message

        self.url_input.setText('192.168.10.103')
        self.port_input.setText('1883')

        self.text_received = deque(maxlen=100)


    def on_connect(self, client, userdata, flags, rc):
        self.client.subscribe("PRoMo/#")


    def on_disconnect(self, client, userdata, rc):
        pass


    def on_message(self, client, userdata, msg):
        # Needed to process the message in the thread where I can modify widgets
        self.message_signal.sig_mqtt_message.emit(msg)


    def process_message(self, msg):
        if msg.topic == 'PRoMo/log':
            # print(type(msg))
            # print(f'{msg.topic}: {msg.payload}')
            try:
                text = msg.payload.decode().rstrip('\r\n')
            except UnicodeDecodeError as e:
                print(f'Error decoding: {msg.payload}')
                return

            used = False
            if self.callback:
                used = self.callback(time.time(), text)

            if used == True and self.show_data_draw:
                return

            self.text_received.append(text)
            self.output_text.clear()
            self.output_text.setPlainText('\r\n'.join(reversed(self.text_received)))


    def write(self, data, topic = 'PRoMo/command'):
        self.client.publish(topic, data)


    def on_toggled(self, checked):
        self.connect_button.setText("Disconnect" if checked else "Connect")
        if checked:
            self.conect_to_broker()
        else:
            self.disconnect_from_broker()


    def conect_to_broker(self):
        if not self.client.is_connected():
            self.output_text.clear()

            url = self.url_input.text()
            self.url_input.setReadOnly(True)
            port = int(self.port_input.text())
            self.port_input.setReadOnly(True)

            self.client.connect_async(url, port)
            self.client.loop_start()

            if self.window:
                self.window.statusBar().showMessage(f'{self.url_input.text()}@{self.url_input.text()} connected', 2000)


    def disconnect_from_broker(self):
        self.url_input.setReadOnly(False)
        self.port_input.setReadOnly(False)

        if self.client.is_connected():
            self.client.disconnect()
            if self.window:
                self.window.statusBar().showMessage("Disconnected", 1000)
        else:
            if self.window:
                self.window.statusBar().showMessage("Not connected.", 1000)
        self.client.loop_stop()



if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    w = MQTTWidget()
    w.show()
    sys.exit(app.exec_())
