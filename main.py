#!/usr/bin/env python

########################################################################################################################
from datetime import datetime

try:
    from Crypto.Cipher import AES
except ImportError as e:
    import pyaes

import random
import socket
import threading
import argparse
import time

TICK = 32.84
IR_TOKEN = 0x26


########################################################################################################################
def genDevice(devtype, host, mac):
    devices = {
        mp1: [0x4EB5,  # MP1
              0x4EF7  # Honyar oem mp1
              ]
    }

    # Look for the class associated to devtype in devices
    [deviceClass] = [dev for dev in devices if devtype in devices[dev]] or [None]
    if deviceClass is None:
        return Device(host=host, mac=mac, devtype=devtype)
    return deviceClass(host=host, mac=mac, devtype=devtype)


class Device:
    def __init__(self, host, mac, devtype, timeout=10):
        self.host = host
        self.mac = mac
        self.devtype = devtype
        self.timeout = timeout
        self.count = 0  #random.randrange (0xffff)
        self.key = bytearray(
            [0x09, 0x76, 0x28, 0x34, 0x3f, 0xe9, 0x9e, 0x23, 0x76, 0x5c, 0x15, 0x13, 0xac, 0xcf, 0x8b, 0x02])
        self.iv = bytearray(
            [0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58])
        self.id = bytearray([0, 0, 0, 0])
        self.cs = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.cs.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.cs.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.cs.bind(('', 0))
        self.type = "Unknown"
        self.lock = threading.Lock()

        if 'pyaes' in globals():
            self.encrypt = self.encrypt_pyaes
            self.decrypt = self.decrypt_pyaes
        else:
            self.encrypt = self.encrypt_pycrypto
            self.decrypt = self.decrypt_pycrypto

    def encrypt_pyaes(self, payload):
        aes = pyaes.AESModeOfOperationCBC(self.key, iv=bytes(self.iv))
        return b"".join([aes.encrypt(bytes(payload[i:i + 16])) for i in range(0, len(payload), 16)])

    def decrypt_pyaes(self, payload):
        aes = pyaes.AESModeOfOperationCBC(self.key, iv=bytes(self.iv))
        return b"".join([aes.decrypt(bytes(payload[i:i + 16])) for i in range(0, len(payload), 16)])

    def encrypt_pycrypto(self, payload):
        aes = AES.new(bytes(self.key), AES.MODE_CBC, bytes(self.iv))
        return aes.encrypt(bytes(payload))

    def decrypt_pycrypto(self, payload):
        aes = AES.new(bytes(self.key), AES.MODE_CBC, bytes(self.iv))
        return aes.decrypt(bytes(payload))

    def auth(self):
        payload = bytearray(0x50)
        payload[0x04] = 0x31
        payload[0x05] = 0x31
        payload[0x06] = 0x31
        payload[0x07] = 0x31
        payload[0x08] = 0x31
        payload[0x09] = 0x31
        payload[0x0a] = 0x31
        payload[0x0b] = 0x31
        payload[0x0c] = 0x31
        payload[0x0d] = 0x31
        payload[0x0e] = 0x31
        payload[0x0f] = 0x31
        payload[0x10] = 0x31
        payload[0x11] = 0x31
        payload[0x12] = 0x31
        payload[0x1e] = 0x01
        payload[0x2d] = 0x01
        payload[0x30] = ord('T')
        payload[0x31] = ord('e')
        payload[0x32] = ord('s')
        payload[0x33] = ord('t')
        payload[0x34] = ord(' ')
        payload[0x35] = ord(' ')
        payload[0x36] = ord('1')

        response = self.send_packet(0x65, payload)

        payload = self.decrypt(response[0x38:])

        if not payload:
            return False

        key = payload[0x04:0x14]
        if len(key) % 16 != 0:
            return False

        self.id = payload[0x00:0x04]
        self.key = key

        print("response: ", ''.join('{:02x}'.format(x) for x in bytearray(response)))
        print("payload: ", ''.join('{:02x}'.format(x) for x in bytearray(payload)))

        return True

    def get_type(self):
        return self.type

    def send_packet(self, command, payload):
        self.count = (self.count + 1) & 0xffff
        packet = bytearray(0x38)
        packet[0x00] = 0x5a
        packet[0x01] = 0xa5
        packet[0x02] = 0xaa
        packet[0x03] = 0x55
        packet[0x04] = 0x5a
        packet[0x05] = 0xa5
        packet[0x06] = 0xaa
        packet[0x07] = 0x55
        packet[0x24] = 0x2a
        packet[0x25] = 0x27
        packet[0x26] = command
        packet[0x28] = self.count & 0xff
        packet[0x29] = self.count >> 8
        packet[0x2a] = self.mac[0]
        packet[0x2b] = self.mac[1]
        packet[0x2c] = self.mac[2]
        packet[0x2d] = self.mac[3]
        packet[0x2e] = self.mac[4]
        packet[0x2f] = self.mac[5]
        packet[0x30] = self.id[0]
        packet[0x31] = self.id[1]
        packet[0x32] = self.id[2]
        packet[0x33] = self.id[3]

        # pad the payload for AES encryption
        if len(payload) > 0:
            numpad = (len(payload) // 16 + 1) * 16
            payload = payload.ljust(numpad, b"\x00")

        checksum = 0xbeaf
        for i in range(len(payload)):
            checksum += payload[i]
            checksum = checksum & 0xffff

        print("self.count", self.count)
        print("payload: ", ''.join('{:02x}'.format(x) for x in bytearray(payload)))
        payload = self.encrypt(payload)
        print("enc payload: ", ''.join('{:02x}'.format(x) for x in bytearray(payload)))

        packet[0x34] = checksum & 0xff
        packet[0x35] = checksum >> 8

        for i in range(len(payload)):
            packet.append(payload[i])

        checksum = 0xbeaf
        for i in range(len(packet)):
            checksum += packet[i]
            checksum = checksum & 0xffff
        packet[0x20] = checksum & 0xff
        packet[0x21] = checksum >> 8

        print("checksum: ", checksum)
        print("packet: ", ''.join('{:02x}'.format(x) for x in bytearray(packet)))

        start_time = time.time()
        with self.lock:
            while True:
                try:
                    self.cs.sendto(packet, self.host)
                    self.cs.settimeout(1)
                    response = self.cs.recvfrom(2048)
                    break
                except socket.timeout:
                    if (time.time() - start_time) > self.timeout:
                        raise
        return bytearray(response[0])


class mp1(Device):
    def __init__(self, host, mac, devtype):
        Device.__init__(self, host, mac, devtype)
        self.type = "MP1"

    def set_power_mask(self, sid_mask, state):
        """Sets the power state of the smart power strip."""

        packet = bytearray(16)
        packet[0x00] = 0x0d
        packet[0x02] = 0xa5
        packet[0x03] = 0xa5
        packet[0x04] = 0x5a
        packet[0x05] = 0x5a
        packet[0x06] = 0xb2 + ((sid_mask << 1) if state else sid_mask)
        packet[0x07] = 0xc0
        packet[0x08] = 0x02
        packet[0x0a] = 0x03
        packet[0x0d] = sid_mask
        packet[0x0e] = sid_mask if state else 0

        response = self.send_packet(0x6a, packet)

        err = response[0x22] | (response[0x23] << 8)

    def set_power(self, sid, state):
        """Sets the power state of the smart power strip."""
        sid_mask = 0x01 << (sid - 1)
        return self.set_power_mask(sid_mask, state)

    def switch(self, state):
        """Sets the power state of the smart power strip."""
        sid_mask = 0x0F
        return self.set_power_mask(sid_mask, state)

    def check_power_raw(self):
        """Returns the power state of the smart power strip in raw format."""
        packet = bytearray(16)
        packet[0x00] = 0x0a
        packet[0x02] = 0xa5
        packet[0x03] = 0xa5
        packet[0x04] = 0x5a
        packet[0x05] = 0x5a
        packet[0x06] = 0xae
        packet[0x07] = 0xc0
        packet[0x08] = 0x01

        response = self.send_packet(0x6a, packet)
        err = response[0x22] | (response[0x23] << 8)
        if err == 0:
            payload = self.decrypt(bytes(response[0x38:]))
            if type(payload[0x4]) == int:
                state = payload[0x0e]
            else:
                state = ord(payload[0x0e])
            return state

    def check_power(self):
        """Returns the power state of the smart power strip."""
        state = self.check_power_raw()
        data = {
            's1': bool(state & 0x01),
            's2': bool(state & 0x02),
            's3': bool(state & 0x04),
            's4': bool(state & 0x08)
        }
        return state


########################################################################################################################
def auto_int(x):
    return int(x, 0)


parser = argparse.ArgumentParser(fromfile_prefix_chars='@')
parser.add_argument("--device", help="device definition as 'type host mac'")
parser.add_argument("--devtype", type=auto_int, default=0x4EB5, help="type of device")
parser.add_argument("--host", help="host address")
parser.add_argument("--mac", help="mac address (hex reverse), as used by python-broadlink library")
parser.add_argument("--check", action="store_true", help="check current power state")
parser.add_argument("--turnon", help="turn on device")
parser.add_argument("--turnoff", help="turn off device")
parser.add_argument("--switch", action="store_true", help="switch state from on to off and off to on")
args = parser.parse_args()

if args.device:
    values = args.device.split()
    devtype = int(values[0], 0)
    host = values[1]
    mac = bytearray.fromhex(values[2])
elif args.mac:
    devtype = args.devtype
    host = args.host
    mac = bytearray.fromhex(args.mac)

if args.host or args.device:
    dev = genDevice(devtype, (#ifndef MAIN__H
#define MAIN__Hhost, 80), mac)
    dev.auth()

if args.check:
    if dev.check_power():
        print('* ON *')
    else:
        print('* OFF *')
if args.turnon:
    values = args.turnon.split()
    sid = int(values[0], 0)
    dev.set_power(sid, True)

    state = dev.check_power()
    if bool(state & 0x01 << (sid - 1)):
        print('== Turned * ON * ==')
    else:
        print('!! Still OFF !!')
if args.turnoff:
    values = args.turnoff.split()
    sid = int(values[0], 0)
    dev.set_power(sid, False)

    state = dev.check_power()
    if bool(state & 0x01 << (sid - 1)):
        print('!! Still ON !!')
    else:
        print('== Turned * OFF * ==')
if args.switch:
    if dev.check_power():
        dev.switch(False)
        print('* Switch to OFF *')
    else:
        dev.switch(True)
        print('* Switch to ON *')
