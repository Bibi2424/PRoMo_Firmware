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


progname = "Live Plotter"
progversion = "0.3"



class PIDWidget(QtWidgets.QWidget):
    def __init__(self, title = 'PID', get_values_cb = None, *args, **kwargs):
        super(PIDWidget, self).__init__(*args, **kwargs)

        self.callback = get_values_cb

        pid_group = QtWidgets.QHBoxLayout()

        pid_group.addWidget(QtWidgets.QLabel(title + ':'))

        pid_group.addWidget(QtWidgets.QLabel("p"))
        self.p = QtWidgets.QLineEdit("100")
        pid_group.addWidget(self.p)
        pid_group.addWidget(QtWidgets.QLabel("i"))
        self.i = QtWidgets.QLineEdit("0")
        pid_group.addWidget(self.i)
        pid_group.addWidget(QtWidgets.QLabel("d"))
        self.d = QtWidgets.QLineEdit("0")
        pid_group.addWidget(self.d)
        pid_group.addWidget(QtWidgets.QPushButton(text='upload', clicked = self.get_values))

        self.setLayout(pid_group)


    def get_values(self):
        p = int(self.p.text())
        i = int(self.i.text())
        d = int(self.d.text())
        if self.callback:
            self.callback(p, i, d)



class GraphWidget(QtWidgets.QWidget):
    def __init__(self, name = 'plot', *args, **kwargs):
        super(GraphWidget, self).__init__(*args, **kwargs)

        layout = QtWidgets.QVBoxLayout()

        layout.addWidget(QtWidgets.QLabel(name))
        self.canvas = MyDynamicMplCanvas(parent = self)
        layout.addWidget(self.canvas)

        self.setLayout(layout)


    def get_canvas(self):
        return self.canvas



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
        self.x = deque(maxlen=self.MAXLEN)
        self.data = []
        super(MyDynamicMplCanvas, self).__init__(*args, **kwargs)
        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.update_figure)
        timer.start(50)

        self.start_time = time.time()


    def change_max_len(self, new_max_len):
        self.x.maxlen = new_max_len


    def compute_initial_figure(self):
        self.axes.plot([], [])


    def add_data(self, x, data):
        while len(data) > len(self.data):
            self.data.append(deque([0] * len(self.x), maxlen=self.MAXLEN))
        self.x.append(x - self.start_time)
        for y, d, in zip(self.data, data):
            y.append(d)


    def update_figure(self):
        self.axes.cla()
        for i, y in enumerate(self.data):
            self.axes.plot(list(self.x), list(y), label = f'{i}')
            self.axes.legend()
            self.axes.set_ylim([-400, 400])
        self.draw()



class ApplicationWindow(QtWidgets.QMainWindow):
    def __init__(self, plot_names, use_serial = False, show_data_draw = True):
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

        # Single PID Widget for both side
        pid_both = PIDWidget(get_values_cb = lambda x, y, z:self.upload_pid(x, y, z))
        l.addWidget(pid_both)

        # One PID Widget per side
        # pid_left = PIDWidget('Left', get_values_cb = lambda x, y, z:self.upload_pid(x, y, z, side = 'left'))
        # pid_right = PIDWidget('Right', get_values_cb = lambda x, y, z:self.upload_pid(x, y, z, side = 'right'))
        # l.addWidget(pid_left)
        # l.addWidget(pid_right)



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

        # self.statusBar().showMessage("All hail matplotlib!", 2000)


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


    # @QtCore.pyqtSlot()
    def upload_pid(self, p, i, d, side = None):
        if side == 'left':
            command = f"pid.set left {p} {i} {d}\n"
        elif side == 'right':
            command = f"pid.set right {p} {i} {d}\n"
        else:
            command = f"pid.set both {p} {i} {d}\n"
        print(command)
        self.ser.write(command)



def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--names', '-n', nargs='*', default=['Left', 'Right'], help='List of names for the graph')
    parser.add_argument('--show-data-draw', action='store_false', help='Will prevent graph data to appear in serial console')
    parser.add_argument('--use-serial', action='store_true', help='By default, will use mqtt, set this to use serial')
    args = parser.parse_args()

    qApp = QtWidgets.QApplication(sys.argv)

    aw = ApplicationWindow(args.names, use_serial=args.use_serial, show_data_draw = args.show_data_draw)
    # aw.setWindowTitle("%s" % progname)
    aw.show()
    sys.exit(qApp.exec_())


if __name__ == '__main__':
    main()
    