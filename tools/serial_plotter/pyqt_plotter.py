# From https://matplotlib.org/2.0.2/examples/user_interfaces/embedding_in_qt5.html

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


progname = "Serial Plotter"
progversion = "0.2"


# Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.).
class MyMplCanvas(FigureCanvas):

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = mpl.figure.Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)

        self.compute_initial_figure()

        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QtWidgets.QSizePolicy.Expanding,
                                   QtWidgets.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)


    def compute_initial_figure(self):
        pass


class MyStaticMplCanvas(MyMplCanvas):
    """Simple canvas with a sine plot."""

    def compute_initial_figure(self):
        pass
        # t = np.arange(0.0, 3.0, 0.01)
        # s = np.sin(2*np.pi*t)
        # self.axes.plot(t, s)



# A canvas that updates itself every second with a new plot.
class MyDynamicMplCanvas(MyMplCanvas):
    MAXLEN = 100

    def __init__(self, *args, **kwargs):
        self.x = deque(maxlen=self.MAXLEN)
        self.data = []
        MyMplCanvas.__init__(self, *args, **kwargs)
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
    def __init__(self, plot_names, show_data_draw = True):
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

        pid_group = QtWidgets.QHBoxLayout()
        l.addLayout(pid_group)

        pid_group.addWidget(QtWidgets.QLabel("p"))
        self.p = QtWidgets.QLineEdit("100")
        pid_group.addWidget(self.p)
        pid_group.addWidget(QtWidgets.QLabel("i"))
        self.i = QtWidgets.QLineEdit("0")
        pid_group.addWidget(self.i)
        pid_group.addWidget(QtWidgets.QLabel("d"))
        self.d = QtWidgets.QLineEdit("0")
        pid_group.addWidget(self.d)
        pid_group.addWidget(QtWidgets.QPushButton(text='upload', clicked = self.upload_pid))


        # TODO: Change to a collection that preserve order
        self.plots = {}

        for name in plot_names:
            dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
            l.addWidget(dc)
            self.plots[name] = dc
            print(f'Creating plot: \"{name}\"')

        h.addLayout(l)

        self.ser = SerialWidget(window=self, callback=self.get_data, show_data_draw = show_data_draw)
        h.addWidget(self.ser)

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        # self.statusBar().showMessage("All hail matplotlib!", 2000)


    def get_data(self, timestamp, text):

        if ':' not in text:
            return False
        
        name, data = text.split(':')

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
    def upload_pid(self):
        p = int(self.p.text())
        i = int(self.i.text())
        d = int(self.d.text())
        command = f"pid.set both {p} {i} {d}\n"
        print(command)
        self.ser.write(command)



def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--names', '-n', nargs='*', default=['Left', 'Right'], help='List of names for the graph')
    parser.add_argument('--show_data_draw', action='store_false', help='Will prevent graph data to appear in serial console')
    args = parser.parse_args()

    qApp = QtWidgets.QApplication(sys.argv)

    aw = ApplicationWindow(args.names, show_data_draw = args.show_data_draw)
    # aw.setWindowTitle("%s" % progname)
    aw.show()
    sys.exit(qApp.exec_())


if __name__ == '__main__':
    main()
    