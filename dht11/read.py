#!/usr/bin/python
import socket, struct, time
import logging, os.path, sqlite3

class NetAdapter:
    def communicate(self, data):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('192.168.0.1', 55555))
        sock.send(data)
        ret = ''
        while 1:
            r = sock.recv(1024)
            if len(r)<=0: break
            ret += r
        return ret
    def dataBlock(self, data):
        return self.communicate('\x02'+data)[1:]
    def find(self):
        ret = self.communicate('\x01')
        devices = struct.unpack('!'+('Q'*((len(ret)-1)/8)), ret[1:])
        return devices
#        


def crc8(data,seed=0):
    for c in data:
        c = ord(c)
        for bit in xrange(8):
            mix = (c^seed)&1
            seed >>=1 ; c >>= 1
            if mix: seed ^=0x8C
    return seed

def bufferToHex(buffer, start=0, count=0):
    if count==0:
        count = len(buffer)-start
    accumulator = ''
    for item in range(count):
        accumulator += '%02X' % ord(buffer[start + item]) + ' '
    return accumulator

try:
    OW_CONVERT_T = '\x44'
    OW_MATCH_ROM = '\x55'
    OW_RECALL_E2 = '\xB8'
    OW_READ_SCRATCHPAD = '\xBE'
    OW_SKIP_ROM = '\xCC'
    OW_SEARCH_ROM='\xF0'
    OW3D_READ_MEM = '\xA5'

    a = NetAdapter()
    print ['%x'%x for x in a.find()]

    def blTouch(addr, memAddr, data):
        block = OW_SEARCH_ROM
        for byte in range(8*3):
            val = 0
            for bit in range(8):
                val >>=1
                index = (8*byte+bit)
                if addr&(1<<(index/3)):
                    val |= 128
            block += chr(val)
            #print '{0:08b}'.format(val)
        #print 'data', repr(block)
        block += OW3D_READ_MEM+struct.pack('<H', memAddr)
        block += data
        return a.dataBlock(block)[25+3:]
                
                
    addr = 0x1d1122334455668b
    #addr = 0x8b6655443322111d
    a.dataBlock(OW_MATCH_ROM+struct.pack('!Q', addr)+OW_CONVERT_T)
    time.sleep(1)
    ret = a.dataBlock(OW_MATCH_ROM+struct.pack('!Q', addr)+OW_READ_SCRATCHPAD+('\xFF'*(5+4)))
    print 'ret', bufferToHex(ret[-(5+4):])
        
        
except Exception, ex:
    print ex
