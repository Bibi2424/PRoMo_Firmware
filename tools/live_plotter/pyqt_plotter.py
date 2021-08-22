# Inspiration from https://matplotlib.org/2.0.2/examples/user_interfaces/embedding_in_qt5.html

import os, sys, time
import argparse
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from collections import deque
# Make sure that we are using QT5
mpl.use('Qt5Agg')
from PyQt5 import QtCore, QtWidgets, QtSerialPort

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas

from pyqt_serial import SerialWidget
from pyqt_mqtt import MQTTWidget
from promo_widgets import PIDWidget, TargetSpeedWidget


progname = "Live Plotter"
progversion = "0.3"


class GraphControl(QtWidgets.QWidget):
    def __init__(self, reset_cb = None, pause_cb = None, *args, **kwargs):
        super(GraphControl, self).__init__(*args, **kwargs)

        self.reset_cb = reset_cb
        self.pause_cb = pause_cb

        pid_group = QtWidgets.QHBoxLayout()

        self.pause_button = QtWidgets.QPushButton(text='Pause', checkable = True, toggled = self.pause_toggled)
        pid_group.addWidget(self.pause_button)
        pid_group.addWidget(QtWidgets.QPushButton(text='Reset', clicked = self.reset))

        self.setLayout(pid_group)


    @QtCore.pyqtSlot()
    def reset(self):
        if self.reset_cb:
            self.reset_cb()


    @QtCore.pyqtSlot(bool)
    def pause_toggled(self, checked):
        self.pause_button.setText("Play" if checked else "Pause")
        print(checked)
        if self.pause_cb:
            self.pause_cb(checked)



class GraphWidget(QtWidgets.QWidget):
    def __init__(self, name = 'plot', *args, **kwargs):
        super(GraphWidget, self).__init__(*args, **kwargs)

        vlayout = QtWidgets.QVBoxLayout()
        hlayout = QtWidgets.QHBoxLayout()
        vlayout.addLayout(hlayout)

        hlayout.addWidget(QtWidgets.QLabel(name))
        # reset_button = QtWidgets.QPushButton(text = 'Reset', clicked = self.on_click)
        # hlayout.addWidget(reset_button)
        hlayout.addWidget(QtWidgets.QLabel("y limit"))
        self.y_limit_minus = QtWidgets.QLineEdit("-200")
        self.y_limit_minus.textChanged.connect(lambda x: self.update_y_limits('minus', x))
        hlayout.addWidget(self.y_limit_minus)
        self.y_limit_plus = QtWidgets.QLineEdit("200")
        self.y_limit_plus.textChanged.connect(lambda x: self.update_y_limits('plus', x))
        hlayout.addWidget(self.y_limit_plus)

        self.canvas = MyDynamicMplCanvas(parent = self)
        vlayout.addWidget(self.canvas)

        self.setLayout(vlayout)


    def get_canvas(self):
        return self.canvas


    @QtCore.pyqtSlot(str, str)
    def update_y_limits(self, side, text):
        print(text)
        try:
            if side == 'plus':
                self.canvas.y_limit[1] = int(text)
            elif side == 'minus':
                self.canvas.y_limit[0] = int(text)
        except ValueError:
            pass


    @QtCore.pyqtSlot()
    def on_click(self):
        self.canvas.reset()




# Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.).
class MyMplCanvas(FigureCanvas):

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = mpl.figure.Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)

        super(MyMplCanvas, self).__init__(fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QtWidgets.QSizePolicy.Expanding,
                                   QtWidgets.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)


    def compute_initial_figure(self):
        pass



class MyStaticMplCanvas(MyMplCanvas):

    def compute_initial_figure(self):
        pass



# A canvas that updates itself every second with a new plot.
class MyDynamicMplCanvas(MyMplCanvas):
    MAXLEN = 100

    def __init__(self, *args, **kwargs):
        super(MyDynamicMplCanvas, self).__init__(*args, **kwargs)

        self.x = deque(maxlen=self.MAXLEN)
        self.data = []
        self.is_paused = False
        self.y_limit = [-200, 200]

        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.update_figure)
        timer.start(50)

        self.start_time = time.time()


    def change_max_len(self, new_max_len):
        self.x.maxlen = new_max_len


    def compute_initial_figure(self):
        self.axes.plot([], [])


    def reset(self):
        self.x.clear()
        self.data = []
        self.update_figure(forced = True)


    def pause(self, is_paused):
        self.is_paused = is_paused


    def add_data(self, x, data):
        if self.is_paused:
            return

        while len(data) > len(self.data):
            self.data.append(deque([0] * len(self.x), maxlen=self.MAXLEN))
        self.x.append(x - self.start_time)
        for y, d, in zip(self.data, data):
            y.append(d)


    @QtCore.pyqtSlot()
    def update_figure(self, forced = False):
        if self.is_paused and not forced:
            return

        self.axes.cla()
        for i, y in enumerate(self.data):
            self.axes.plot(list(self.x), list(y), label = f'{i}')
            self.axes.legend()
        self.axes.set_ylim(self.y_limit)
        self.draw()



class ApplicationWindow(QtWidgets.QMainWindow):
    def __init__(self, plot_names, use_serial = False, show_data_draw = True, auto_connect = False):
        QtWidgets.QMainWindow.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowTitle("Serial Plotter")

        # self.file_menu = QtWidgets.QMenu('&File', self)
        # self.file_menu.addAction('&Quit', self.fileQuit,
        #                          QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        # self.menuBar().addMenu(self.file_menu)

        # self.help_menu = QtWidgets.QMenu('&Help', self)
        # self.menuBar().addSeparator()
        # self.menuBar().addMenu(self.help_menu)

        # self.help_menu.addAction('&About', self.about)

        self.main_widget = QtWidgets.QWidget(self)

        h = QtWidgets.QHBoxLayout(self.main_widget)

        l = QtWidgets.QVBoxLayout()

        target_speed = TargetSpeedWidget(get_values_cb = self.send_target_speed)
        l.addWidget(target_speed)

        # Single PID Widget for both side
        pid_both = PIDWidget(get_values_cb = lambda x, y, z:self.upload_pid(x, y, z))
        l.addWidget(pid_both)

        # One PID Widget per side
        # pid_left = PIDWidget('Left', get_values_cb = lambda x, y, z:self.upload_pid(x, y, z, side = 'left'))
        # pid_right = PIDWidget('Right', get_values_cb = lambda x, y, z:self.upload_pid(x, y, z, side = 'right'))
        # l.addWidget(pid_left)
        # l.addWidget(pid_right)

        graph_control = GraphControl(reset_cb = self.reset_all, pause_cb = self.pause_all)
        l.addWidget(graph_control)


        # TODO: Change to a collection that preserve order
        self.plots = {}

        for name in plot_names:
            dc = GraphWidget(name)
            # dc = MyDynamicMplCanvas(parent = self.main_widget)
            l.addWidget(dc)
            self.plots[name] = dc.get_canvas()
            print(f'Creating plot: \"{name}\"')

        h.addLayout(l)

        if use_serial:
            self.ser = SerialWidget(window=self, callback=self.get_data, show_data_draw = show_data_draw)
        else:
            self.ser = MQTTWidget(window=self, callback=self.get_data, show_data_draw = show_data_draw)
        h.addWidget(self.ser)

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        if auto_connect:
            self.ser.on_toggled(True)


    def pause_all(self, is_paused):
        for p in self.plots.values():
            p.pause(is_paused)


    def reset_all(self):
        for p in self.plots.values():
            p.reset()


    def get_data(self, timestamp, text):

        if ':' not in text:
            return False
        
        try:
            name, data = text.split(':')
        except ValueError as e:
            print(f'Error spliting :{text}')
            return

        if name not in self.plots:
            return False

        try:
            r = map(int, data.strip().split(' - '))
            data = list(r)
            self.plots[name].add_data(timestamp, data)
        except Exception as e:
            print(f'Error decoding: {text}')
            return False

        return True


    @QtCore.pyqtSlot()
    def upload_pid(self, p, i, d, side = None):
        if side == 'left':
            command = f"pid.set left {p} {i} {d}\n"
        elif side == 'right':
            command = f"pid.set right {p} {i} {d}\n"
        else:
            command = f"pid.set both {p} {i} {d}\n"
        print(command)
        self.ser.write(command)


    @QtCore.pyqtSlot()
    def send_target_speed(self, left, right):
        command = f"target.speed {left} {right}\n"
        print(command)
        self.ser.write(command)



def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--names', '-n', nargs='*', default=['Left', 'Right'], help='List of names for the graph')
    parser.add_argument('--show-data-draw', action='store_false', help='Will prevent graph data to appear in serial console')
    parser.add_argument('--use-serial', action='store_true', help='By default, will use mqtt, set this to use serial')
    parser.add_argument('--auto-connect', action='store_true', help='Use this flag to connect on open')
    args = parser.parse_args()

    qApp = QtWidgets.QApplication(sys.argv)

    aw = ApplicationWindow(args.names, use_serial=args.use_serial, show_data_draw = args.show_data_draw, auto_connect = args.auto_connect)
    # aw.setWindowTitle("%s" % progname)
    aw.show()
    sys.exit(qApp.exec_())


if __name__ == '__main__':
    main()
    