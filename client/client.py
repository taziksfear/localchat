import sys
import socket
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QTextEdit, QLineEdit, QPushButton, QLabel
from PyQt6.QtCore import QThread, pyqtSignal

# Мы обязаны создать отдельный класс для потока потому что если слушать сервер в главном окне то оно крах-бабах-капут
class Worker(QThread):
    new_text = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.sock = None
        self.ip = "127.0.0.1"
        self.is_running = True

    def run(self):
        # Эта функция запускается, когда мы пишем worker.start()
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.ip, 54000)) # Порт жестко :hot-face: :cum: задан
            self.new_text.emit("Успешное подключение!")
        except:
            self.new_text.emit("Не удалось подключиться к серверу!")
            return

        # Бесконечный цикл прослушивания
        while self.is_running:
            try:
                data = self.sock.recv(4096) # Ждем сообщение
                if not data:
                    break # Если пусто, значит сервер упал
                
                message = data.decode("utf-8")
                self.new_text.emit(message) # Отправляем текст в окно
            except:
                break
        
        self.sock.close()

    def SendData(self, text):
        if self.sock:
            try:
                self.sock.send(text.encode("utf-8"))
            except:
                pass


# Класс окна
class MyWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Чат")
        self.resize(300, 500)

        # Главный контейнер
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        
        # Вертикальный слой (все элементы друг под другом)
        self.layout = QVBoxLayout()
        main_widget.setLayout(self.layout)

        # ЭЛЕМЕНТЫ ИНТЕРФЕЙСА

        self.label_ip = QLabel("Введите IP сервера:")
        self.layout.addWidget(self.label_ip)

        self.input_ip = QLineEdit("127.0.0.1")
        self.layout.addWidget(self.input_ip)

        # Кнопка подключения
        self.btn_connect = QPushButton("Подключиться")
        self.btn_connect.clicked.connect(self.ConnectClick) # Привязываем нажатие
        self.layout.addWidget(self.btn_connect)

        # Поле переписки (большое)
        self.text_area = QTextEdit()
        self.text_area.setReadOnly(True) # Нельзя писать руками в историю
        self.layout.addWidget(self.text_area)

        # Поле ввода сообщения
        self.input_msg = QLineEdit()
        self.layout.addWidget(self.input_msg)

        # Кнопка отправки
        self.btn_send = QPushButton("Отправить")
        self.btn_send.clicked.connect(self.SendClick)
        self.layout.addWidget(self.btn_send)

        # Создаем нашего работника (поток), но пока не запускаем
        self.worker = Worker()
        # Говорим: "когда у работника появится текст, вызови функцию AddText"
        self.worker.new_text.connect(self.AddText)

    def ConnectClick(self):
        # Берем IP из поля ввода
        ip_address = self.input_ip.text()
        self.worker.ip = ip_address
        self.worker.start() # ЗАПУСК ПОТОКА
        
        # Блокируем кнопку, чтобы не жали дважды
        self.btn_connect.setEnabled(False) 
        self.input_ip.setEnabled(False)

    def SendClick(self):
        text = self.input_msg.text()
        if len(text) > 0:
            self.worker.SendData(text) # Отправляем через поток
            self.text_area.append(f"Вы: {text}") # Пишем себе в окно
            self.input_msg.clear() # Очищаем поле ввода

    def AddText(self, text):
        # Эта функция просто добавляет текст в большое поле
        self.text_area.append(text)


# Стандартный запуск приложения
app = QApplication(sys.argv)
window = MyWindow()
window.show()
sys.exit(app.exec())