import socket, select
 
class Netcat:
    """ Python 'netcat like' module """

    def __init__(self, ip, port):
        self.buff = ""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((ip, port))

    def read(self, length = 1024):
        """ Read 1024 bytes off the socket """

        return self.socket.recv(length).decode('utf-8')
 
    def read_until(self, data):
        """ Read data into the buffer until we have data """

        while not data in self.buff:
            r, w, e = select.select([self.socket], [], [self.socket], 0.1)
            if not r:
                return ''
            self.buff += self.socket.recv(1024).decode('utf-8')

 
        pos = self.buff.find(data)
        rval = self.buff[:pos + len(data)]
        self.buff = self.buff[pos + len(data):]
 
        return rval
 
    def write(self, data):
        self.socket.send(data.encode('utf-8'))
    
    def close(self):
        self.socket.close()


import threading

nc = Netcat('192.168.4.1', 80)
running = True

def netreader():
    while running:
        print(nc.read_until('\n'), end='')



t = threading.Thread(target=netreader)
t.start()

nc.write('\n') # intro

while True:
    i = input()
    if i == 'quit':
        running = False
        exit()
    if len(i)>0 and i[0] in 'LRWN':
        nc.write( i + '\n') 
