#!/usr/bin/python
import socket, struct, time
import os.path

from pyonewire.master import ds2490
import time, struct
class OWAdapter:
    def __init__(self):
        self.master = ds2490.DS2490Master()
    def find(self):
        return self.master.Search(self.master.SEARCH_NORMAL, max=0)
    def dataBlock(self, data):
        return self.master.BlockIO(data)


adapter = OWAdapter()

#create an INET, STREAMing socket
serversocket = socket.socket(
    socket.AF_INET, socket.SOCK_STREAM)
#bind the socket to a public host,
# and a well-known port
serversocket.bind(('0.0.0.0', 55555))
#become a server socket
serversocket.listen(5)
while 1:
    (clientsocket, address) = serversocket.accept()
    buff = clientsocket.recv(1024)
    if len(buff)<=0: continue
    if buff[0]=='\x01': # find
        ret = adapter.find()
        print "ret", repr(ret)
        ret = struct.pack('!'+('Q'*len(ret)), *ret)
    elif buff[0]=='\x02': # block
        ret = adapter.dataBlock(buff[1:])
    clientsocket.send('\x00'+ret)
    clientsocket.close()
