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
from matplotlib.figure import Figure

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

        self.canvas = MyDynamicMplCanvas(parent = self)

        hlayout.addWidget(QtWidgets.QLabel("y limit"))
        self.y_limit_minus = QtWidgets.QLineEdit(str(self.canvas.y_limit[0]))
        self.y_limit_minus.textChanged.connect(lambda x: self.update_y_limits('minus', x))
        hlayout.addWidget(self.y_limit_minus)
        self.y_limit_plus = QtWidgets.QLineEdit(str(self.canvas.y_limit[1]))
        self.y_limit_plus.textChanged.connect(lambda x: self.update_y_limits('plus', x))
        hlayout.addWidget(self.y_limit_plus)

        vlayout.addWidget(self.canvas)

        self.setLayout(vlayout)


    def get_canvas(self):
        return self.canvas


    @QtCore.pyqtSlot(str, str)
    def update_y_limits(self, side, text):
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



# A canvas that updates itself every second with a new plot.
class MyDynamicMplCanvas(FigureCanvas):
    MAXLEN = 100

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = self.fig.add_subplot(111)
        super(MyDynamicMplCanvas, self).__init__(self.fig)
        self.setParent(parent)

        self.x = deque(maxlen=self.MAXLEN)
        self.y_list = []
        self.plots = []
        self.is_paused = False
        self.y_limit = [-200, 4096]

        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.update_figure)
        timer.start(50)

        self.start_time = time.time()


    def change_max_len(self, new_max_len):
        self.x.maxlen = new_max_len


    def reset(self):
        self.x.clear()
        self.y_list = []
        self.update_figure(forced = True)
        self.start_time = time.time()


    def pause(self, is_paused):
        self.is_paused = is_paused


    def add_data(self, x, new_data):
        if self.is_paused:
            return

        while len(new_data) > len(self.y_list):
            self.y_list.append(deque([0] * len(self.x), maxlen=self.MAXLEN))
            self.plots.append(self.axes.plot([])[0])

        self.x.append(x - self.start_time)
        for y, d, in zip(self.y_list, new_data):
            y.append(d)


    @QtCore.pyqtSlot()
    def update_figure(self, forced = False):
        if self.is_paused and not forced:
            self.axes.set_ylim(self.y_limit)
            self.draw()
            return

        self.axes.cla()
        for i, y in enumerate(self.y_list):
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

        self.graphs_layout = QtWidgets.QVBoxLayout()

        target_speed = TargetSpeedWidget(get_values_cb = self.send_target_speed)
        self.graphs_layout.addWidget(target_speed)

        # Single PID Widget for both side
        pid_both = PIDWidget(get_values_cb = lambda x, y, z:self.upload_pid(x, y, z))
        self.graphs_layout.addWidget(pid_both)

        graph_control = GraphControl(reset_cb = self.reset_all, pause_cb = self.pause_all)
        self.graphs_layout.addWidget(graph_control)


        # TODO: Change to a collection that preserve order
        self.plots = {}

        for name in plot_names:
            dc = GraphWidget(name)
            self.graphs_layout.addWidget(dc)
            self.plots[name] = dc.get_canvas()
            print(f'Creating plot: \"{name}\"')

        h.addLayout(self.graphs_layout)

        if use_serial:
            self.ser = SerialWidget(window=self, callback=self.get_data, show_data_draw = show_data_draw)
        else:
            self.ser = MQTTWidget(window=self, callback=self.get_data, show_data_draw = show_data_draw)
        h.addWidget(self.ser)

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        if auto_connect:
            self.ser.on_connect(True)


    def pause_all(self, is_paused):
        for p in self.plots.values():
            p.pause(is_paused)


    def reset_all(self):
        for p in self.plots.values():
            p.reset()


    def process_new_points(self, text):
        try:
            text.replace(' ', '')
            data = text.split(',')
        except ValueError as e:
            print(f'Error spliting :{text}')
            return False

        if len(data) < 3:
            print(f'Not enough data to graph (min 3): {text}')

        graph_name = data.pop(0)
        timestamp = data.pop(0)

        if graph_name not in self.plots:
            dc = GraphWidget(graph_name)
            self.graphs_layout.addWidget(dc)
            self.plots[graph_name] = dc.get_canvas()
            print(f'Creating plot: \"{graph_name}\"')
            return False

        if not timestamp:
            timestamp = time.time()

        try:
            data = [float(x) for x in data]
            self.plots[graph_name].add_data(timestamp, data)
        except Exception as e:
            print(f'Error decoding: {text}')
            return False

        return True

    def get_data(self, text):
        # Expect a line: "@{Graph Name},{time:optional},{y1},{y2},{...},{yn}"
        if text.startswith('@') == True:
            return self.process_new_points(text[1:])
        return False


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
    parser.add_argument('--names', '-n', nargs='*', default=[''], help='List of names for the graph')
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
    