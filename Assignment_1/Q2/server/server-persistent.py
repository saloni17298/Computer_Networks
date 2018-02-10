import socket
import sys
import os

port = 9898
s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
host = ""

s.bind((host, port))
s.listen(5)

print 'Server listening....'

conn, addr = s.accept()

while True:
    filename = conn.recv(1024)
    print filename

    if not os.path.isfile(filename):
        print('Error : No such file %s' %filename)
        conn.close()
        s.close()
        sys.exit()

    else:
        f = open(filename,'rb')
        l = f.read(1024)

        while (l):
           conn.send(l)
           # print('Sent ',repr(l))
           l = f.read(1024)

        f.close()

print('Done sending')
conn.send('Thank you for connecting')
conn.close()
