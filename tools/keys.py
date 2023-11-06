import keyboard
import serial
import serial.tools.list_ports
import argparse

BAUD_RATE = 9600

def run(args):
    ser = serial.Serial(args.port, BAUD_RATE)

    keys = ['w','s','a','d','k','j']

    code = 0

    while True:
        key = keyboard.read_key()
        if key not in keys:
            continue

        idx = keys.index(key)

        if keyboard.is_pressed(key):
            new_code = code | (1 << idx)
        else:
            new_code = code & ~(1 << idx)

        if code == new_code:
            continue

        code = new_code
        
        ser.write(bytes([code]))

def main():
    parser = argparse.ArgumentParser(description='Serial Keyboard Controller')
    parser.add_argument('-p', '--port', type=str)

    args = parser.parse_args()

    if not args.port:
        print('Set port with -p or --port')
        print('Available ports:')
        for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
            print('   ', port)
            print('       ', desc)
        return

    run(args)

if __name__ == '__main__':
    main()