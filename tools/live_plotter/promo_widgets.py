from PyQt5 import QtCore, QtWidgets


class PIDWidget(QtWidgets.QWidget):
    def __init__(self, title = 'PID', get_values_cb = None, default = {'p':1.0, 'i':0.1, 'd':0.0}, *args, **kwargs):
        super(PIDWidget, self).__init__(*args, **kwargs)

        self.callback = get_values_cb

        pid_group = QtWidgets.QHBoxLayout()

        pid_group.addWidget(QtWidgets.QLabel(title + ':'))

        pid_group.addWidget(QtWidgets.QLabel("p"))
        self.p = QtWidgets.QLineEdit(str(default['p']))
        pid_group.addWidget(self.p)
        pid_group.addWidget(QtWidgets.QLabel("i"))
        self.i = QtWidgets.QLineEdit(str(default['i']))
        pid_group.addWidget(self.i)
        pid_group.addWidget(QtWidgets.QLabel("d"))
        self.d = QtWidgets.QLineEdit(str(default['d']))
        pid_group.addWidget(self.d)
        pid_group.addWidget(QtWidgets.QPushButton(text='upload', clicked = self.get_values))

        self.setLayout(pid_group)


    @QtCore.pyqtSlot()
    def get_values(self):
        p = float(self.p.text())
        i = float(self.i.text())
        d = float(self.d.text())
        if self.callback:
            self.callback(p, i, d)



class TargetSpeedWidget(QtWidgets.QWidget):
    def __init__(self, title = 'Target', get_values_cb = None, *args, **kwargs):
        super(TargetSpeedWidget, self).__init__(*args, **kwargs)

        self.callback = get_values_cb

        pid_group = QtWidgets.QHBoxLayout()

        pid_group.addWidget(QtWidgets.QLabel(title + ':'))

        pid_group.addWidget(QtWidgets.QLabel("left"))
        self.speed_left = QtWidgets.QLineEdit("100")
        pid_group.addWidget(self.speed_left)
        pid_group.addWidget(QtWidgets.QLabel("right"))
        self.speed_right = QtWidgets.QLineEdit("100")
        pid_group.addWidget(self.speed_right)
        pid_group.addWidget(QtWidgets.QPushButton(text='Reset', clicked = self.reset))
        pid_group.addWidget(QtWidgets.QPushButton(text='Send', clicked = self.set_target))

        self.setLayout(pid_group)


    @QtCore.pyqtSlot()
    def reset(self):
        if self.callback:
            self.callback(0, 0)


    @QtCore.pyqtSlot()
    def set_target(self):
        left = int(self.speed_left.text())
        right = int(self.speed_right.text())
        if self.callback:
            self.callback(left, right)


