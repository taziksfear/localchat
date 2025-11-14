import socket
import threading
import sys

# Убираем лишний пробел в начале IP адреса!
sip = '127.0.0.1'
sport = 54000

def waitmessage(sock):
    while True:
        try:
            data = sock.recv(4096)
            if not data:
                print("\n[!] Отключено от сервера.")
                break
            # flush=True заставляет текст появляться мгновенно
            print(f"\nСЕРВЕР> {data.decode('utf-8')}", flush=True)
            print("Вы: ", end='', flush=True)
        except:
            print("\n[!] Ошибка соединения.")
            break

def main():
    cSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        print(f"Подключение к {sip}:{sport}...", flush=True)
        cSocket.connect((sip, sport))
        print("Успешно подключено! Пиши сообщения и жми Enter.", flush=True)
    except Exception as e:
        print(f"Не удалось подключиться к {sip}:{sport}")
        print(f"Ошибка: {e}")
        return

    listenthread = threading.Thread(target=waitmessage, args=(cSocket,))
    listenthread.daemon = True
    listenthread.start()

    # Первое приглашение к вводу
    print("Вы: ", end='', flush=True)

    while True:
        msg = input()
        if msg.lower() == 'exit': # Лучше использовать английский для команды выхода
            break

        try:
            cSocket.send(msg.encode('utf-8'))
        except:
            print("Ошибка отправки.")
            break

    cSocket.close()

# --- ВОТ ЧЕГО НЕ ХВАТАЛО: ---
if __name__ == "__main__":
    main()