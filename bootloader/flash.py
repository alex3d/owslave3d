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

def bufferToHex(buffer, start=0, count=0):
    if count==0:
        count = len(buffer)-start
    accumulator = ''
    for item in range(count):
        accumulator += '%02X' % ord(buffer[start + item]) + ' '
    return accumulator

def blTouch(addr, memAddr, data):
    addr = struct.unpack( '>Q', struct.pack('<Q', addr) )[0]
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
    return block
    return a.dataBlock(block)[25+3:]

try:
    OW_MATCH_ROM = '\x55'
    OW_SEARCH_ROM='\xF0'
    OW3D_READ_MEM = '\xA5'
    OW_RESET = '\xCF'

    a = NetAdapter()
    print ["%x"%x for x in a.find()]

    addr = 0x1d1122334455668b
                
    #a.dataBlock(OW_MATCH_ROM+struct.pack('<Q', addr)+OW_RESET)

    # wait for bootloader
    while 1:
        ret = blTouch(addr, 0x9000, ('\xFF'*32))
        print "block", bufferToHex(ret)
        time.sleep(0.5)
        if '\x1E\x59\x91\x58\x0A' == ret[:5]: break
    
    #flash!
    f = open("build/main.bin","rb")
    memAddr = 0
    t0 = time.time()
    while 1:
        data = f.read(32)
        if not data: break
        data += '\xff'*(32-len(data))
        ret = blTouch(addr, memAddr, data+'\xFF')
        print "write", bufferToHex(data)
        if memAddr!=0:
            ret = blTouch(addr, 0x8000 | memAddr, '\xff'*32)
            print "write", bufferToHex(ret)
            if(ret!=data):
                raise Exception('validation failed!')
        print 'addr', memAddr
        memAddr += 32
    print 'elapsed %.3f'%(time.time()-t0)
        
except Exception, ex:
    print ex
