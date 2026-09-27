# 1. В качестве исходного образа взять alpine:3.14
FROM alpine:3.14

# 2. Добавить метку (LABEL description="QT TCP Server Container")
LABEL description="QT TCP Server Container"

# 3. Установить Qt5 и компилятор (qt5-qtbase-dev, g++)
RUN apk update && apk add --no-cache \
    qt5-qtbase-dev \
    g++ \
    make

# 4. Скопировать файлы проекта в контейнер в папку /home/server
WORKDIR /home/server
COPY ./server /home/server

# 5. Скомпилировать проект в файл с именем qt_tcp_server
RUN qmake server.pro && make

# 6. Открыть порт 5555 и запускать сервер автоматически
EXPOSE 5555
ENTRYPOINT ["/home/server/qt_tcp_server"]
