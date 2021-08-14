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
    MAXLEN = 200

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
        for y in self.data:
            self.axes.plot(list(self.x), list(y))
        self.draw()



class ApplicationWindow(QtWidgets.QMainWindow):
    def __init__(self, plot_names):
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

        # TODO: Change to a collection that preserve order
        self.plots = {}

        for name in plot_names:
            dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
            l.addWidget(dc)
            self.plots[name] = dc
            print(f'Creating plot: \"{name}\"')


        # dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        # l.addWidget(dc)
        # self.plots['Encoder'] = dc
        # dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        # l.addWidget(dc)
        # self.plots['Target'] = dc
        # dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        # l.addWidget(dc)
        # self.plots['PID'] = dc

        h.addLayout(l)

        ser = SerialWidget(window=self, callback=self.get_data)
        h.addWidget(ser)

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        # self.statusBar().showMessage("All hail matplotlib!", 2000)


    def get_data(self, timestamp, text):

        if ':' not in text:
            return
        
        name, data = text.split(':')

        if name not in self.plots:
            return

        try:
            r = map(int, data.strip().split(' - '))
            data = list(r)
            print(f'{name} add {data}')
            self.plots[name].add_data(timestamp, data)
        except Exception as e:
            print(f'Error decoding: {text}')



def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--names', '-n', nargs='*', default=['Left', 'Right'], help='List of names for the graph')
    # parser.add_argument('--output-file', '-o', default=None, help='path to output file')
    # parser.add_argument('--image-name', '-n', default='image', help='name to be used for array and define name, and file name if not specified')
    args = parser.parse_args()

    qApp = QtWidgets.QApplication(sys.argv)

    aw = ApplicationWindow(args.names)
    # aw.setWindowTitle("%s" % progname)
    aw.show()
    sys.exit(qApp.exec_())


if __name__ == '__main__':
    main()
    