#!/usr/bin/env python3
"""Capture physical-device serial output; avoid HUPCL reset on monitor close."""
import argparse,time,termios
from pathlib import Path
import serial

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--port',default='/dev/cu.usbmodem1101')
    parser.add_argument('--seconds',type=float,default=60)
    parser.add_argument('--output',required=True)
    args=parser.parse_args()
    port=serial.Serial();port.port=args.port;port.baudrate=115200;port.timeout=.2
    port.dtr=False;port.rts=False;port.open()
    attributes=termios.tcgetattr(port.fileno());attributes[2]&=~termios.HUPCL
    termios.tcsetattr(port.fileno(),termios.TCSANOW,attributes)
    target=Path(args.output);target.parent.mkdir(parents=True,exist_ok=True)
    deadline=time.monotonic()+args.seconds
    try:
        with target.open('wb') as out:
            while time.monotonic()<deadline:
                data=port.read(16384)
                if data:out.write(data);out.flush()
    finally:port.close()
    print('Captured',target.stat().st_size,'bytes to',target)
if __name__=='__main__':main()
