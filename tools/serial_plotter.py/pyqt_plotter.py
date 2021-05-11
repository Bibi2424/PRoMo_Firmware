# From https://matplotlib.org/2.0.2/examples/user_interfaces/embedding_in_qt5.html

import os, sys, time
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from collections import deque
# Make sure that we are using QT5
mpl.use('Qt5Agg')
from PyQt5 import QtCore, QtWidgets, QtSerialPort

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas

from pyqt_serial import SerialWidget


progname = os.path.basename(sys.argv[0])
progversion = "0.1"


class MyMplCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""

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
        t = np.arange(0.0, 3.0, 0.01)
        s = np.sin(2*np.pi*t)
        self.axes.plot(t, s)


class MyDynamicMplCanvas(MyMplCanvas):
    """A canvas that updates itself every second with a new plot."""

    def __init__(self, *args, **kwargs):
        self.x = deque(maxlen=100)
        self.y = deque(maxlen=100)
        MyMplCanvas.__init__(self, *args, **kwargs)
        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.update_figure)
        timer.start(50)

        self.start_time = time.time()


    def compute_initial_figure(self):
        self.axes.plot(list(self.x), list(self.y))

    def add_data(self, x, y):
        self.x.append(self.start_time - x)
        self.y.append(y)

    def update_figure(self):
        # Build a list of 4 random integers between 0 and 10 (both inclusive)
        self.axes.cla()
        self.axes.plot(list(self.x), list(self.y), 'r')
        self.draw()


class ApplicationWindow(QtWidgets.QMainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowTitle("application main window")

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
        # sc = MyStaticMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        # l.addWidget(sc)
        self.dc = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        l.addWidget(self.dc)

        h.addLayout(l)
        ser = SerialWidget(parent=self.main_widget, callback=self.get_data)
        h.addWidget(ser)

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        self.statusBar().showMessage("All hail matplotlib!", 2000)

    def get_data(self, timestamp, data):
        # print(timestamp, data)
        self.dc.add_data(timestamp, data[0])

    def fileQuit(self):
        self.close()

    def closeEvent(self, ce):
        self.fileQuit()

    def about(self):
        QtWidgets.QMessageBox.about(self, "About",""" about """)


qApp = QtWidgets.QApplication(sys.argv)

aw = ApplicationWindow()
aw.setWindowTitle("%s" % progname)
aw.show()
sys.exit(qApp.exec_())
#qApp.exec_()