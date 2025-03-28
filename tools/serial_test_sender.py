import serial
import time
import random

PORT = 'COM8'
BAUDRATE = 115200

def send_test_data(ser, data_size):
    # 生成随机测试数据
    test_data = bytes([random.randint(0, 255) for _ in range(data_size)])
    
    # 构造帧头: AA 55 + 长度(2字节)
    header = bytes([0xAA, 0x55, (data_size >> 8) & 0xFF, data_size & 0xFF])
    
    # 发送数据
    ser.write(header)
    ser.write(test_data)
    print(f"Sent test frame: {data_size} bytes")

def main():
    ser = serial.Serial(PORT, BAUDRATE)
    print(f"Testing serial port {PORT}...")
    
    try:
        while True:
            # 发送不同大小的测试数据(100-1000字节)
            send_test_data(ser, random.randint(100, 1000))
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nTest stopped")
    finally:
        ser.close()

if __name__ == '__main__':
    main()