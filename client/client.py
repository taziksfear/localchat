import socket
import threading
import sys

sip = '-'
sport = 54000

def waitmessage(sock):
    while True:
        try:
            data = sock.recv(4096)
            if not data:
                print("\n[!] отключено.")
                break
            print(f"\nсервер> {data.decode('utf-8')}", flush=True)
            print("Вы: ", end='', flush=True)
        except:
            print("\nнет соединения.")
            break

def main():
    cSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        print("Подключение...", flush=True)
        cSocket.connect((sip, sport))
        print("подключено", flush=True)
    except Exception as e:
        print(f"не удалось подключиться")
        print(f"Ошибка: {e}")
        return

    listenthread = threading.Thread(target=waitmessage, args=(cSocket))
    listenthread.daemon = True
    listenthread.start()

    print("Вы: ", end='', flush=True)

    while True:
        msg = input()
        if msg.lower() == 'exit': 
            break

        try:
            cSocket.send(msg.encode('utf-8'))
        except:
            print("ошибка отправки.")
            break

    cSocket.close()

if __name__ == "__main__":
    main()