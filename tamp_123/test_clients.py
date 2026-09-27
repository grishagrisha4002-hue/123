#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для тестирования TCP-сервера:
1. Подключает 5 клиентов (максимальный лимит).
2. Задает им ники: Client_1 .. Client_5.
3. Клиенты по очереди отправляют веерные сообщения в чат.
4. Один из клиентов меняет ник через команду /name НовыйНик.
5. Подключается 6-й клиент (лишний) — сервер отклоняет его с сообщением о занятости.
6. Один клиент отключается, после чего освобождается слот.
"""

import socket
import time
import threading

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 5555

def client_listener(sock, name):
    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break
            for line in data.decode("utf-8", errors="replace").splitlines():
                if line.strip():
                    print(f"[{name} <- СЕРВЕР]: {line.strip()}")
    except:
        pass

def main():
    print("=" * 60)
    print("   ЗАПУСК ТЕСТИРОВАНИЯ СЕРВЕРА (5 КЛИЕНТОВ + 1 ЛИШНИЙ)")
    print("=" * 60)

    sockets = []
    
    # 1. Подключение 5 клиентов
    for i in range(1, 6):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_HOST, SERVER_PORT))
        sockets.append(s)
        t = threading.Thread(target=client_listener, args=(s, f"Клиент_{i}"), daemon=True)
        t.start()
        time.sleep(0.5)

        # Отправляем ник
        nick = f"Клиент_{i}"
        s.sendall(f"{nick}\n".encode("utf-8"))
        time.sleep(0.5)

    print("\n--- Все 5 клиентов успешно подключены и зарегистрировали ники ---\n")
    time.sleep(1)

    # 2. Клиенты по очереди отправляют сообщения в чат (веерно)
    print("--- Тест веерной отправки сообщений ---")
    for i, s in enumerate(sockets, start=1):
        msg = f"Привет всем от Клиента_{i}!"
        print(f"[Клиент_{i} -> ЧАТ]: {msg}")
        s.sendall(f"{msg}\n".encode("utf-8"))
        time.sleep(1)

    # 3. Смена ника у Клиента 2 через команду /name
    print("\n--- Тест смены ника через команду /name ---")
    print("[Клиент_2 -> ЧАТ]: /name СуперБобёр")
    sockets[1].sendall(b"/name \xd0\xa1\xd1\x83\xd0\xbf\xd0\xb5\xd1\x80\xd0\x91\xd0\xbe\xd0\xb1\xd1\x91\xd1\x80\n")
    time.sleep(1.5)

    # 4. Попытка подключения 6-го (лишнего) клиента
    print("\n--- Тест подключения 6-го (лишнего) клиента ---")
    extra_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        extra_sock.connect((SERVER_HOST, SERVER_PORT))
        print("[Клиент_6]: Подключился, ожидаем ответ сервера...")
        data = extra_sock.recv(4096)
        print(f"[Клиент_6 <- СЕРВЕР]: {data.decode('utf-8', errors='replace').strip()}")
        # Сервер должен закрыть соединение
        data_after = extra_sock.recv(4096)
        if not data_after:
            print("[Клиент_6]: Соединение успешно разорвано сервером (лимит 5 достигнут).")
    except Exception as e:
        print(f"[Клиент_6]: Ошибка или разрыв: {e}")
    finally:
        extra_sock.close()

    time.sleep(1)

    # 5. Отключение одного клиента
    print("\n--- Отключение Клиента_1 ---")
    sockets[0].close()
    time.sleep(1)

    print("\n" + "=" * 60)
    print("   ТЕСТИРОВАНИЕ ЗАВЕРШЕНО")
    print("=" * 60)

    # Закрываем остальные
    for s in sockets[1:]:
        try:
            s.close()
        except:
            pass

if __name__ == "__main__":
    main()
