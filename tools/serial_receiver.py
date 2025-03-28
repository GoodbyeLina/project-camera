import serial
import os
import time
from datetime import datetime

# 配置串口参数
PORT = 'COM8'
BAUDRATE = 115200
OUTPUT_DIR = r'C:\Users\97978\Desktop\esp_camera'
os.makedirs(OUTPUT_DIR, exist_ok=True)

def save_frame(data):
    try:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        # 保存原始二进制文件
        bin_filename = os.path.join(OUTPUT_DIR, f"frame_{timestamp}.bin")
        with open(bin_filename, 'wb') as f:
            f.write(data)
        
        # 保存可读文本文件
        txt_filename = os.path.join(OUTPUT_DIR, f"frame_{timestamp}.txt")
        with open(txt_filename, 'w') as f:
            # 写入十六进制格式
            f.write("Hex dump:\n")
            for i in range(0, len(data), 16):
                chunk = data[i:i+16]
                hex_str = ' '.join(f"{b:02X}" for b in chunk)
                ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
                f.write(f"{i:04X}: {hex_str.ljust(47)}  {ascii_str}\n")
        
        print(f"Saved binary to {bin_filename}")
        print(f"Saved text dump to {txt_filename}")
        return True
    except Exception as e:
        print(f"Save failed: {str(e)}")
        return False

def main():
    print(f"Output directory: {OUTPUT_DIR}")
    print("Checking directory permissions...")
    try:
        test_file = os.path.join(OUTPUT_DIR, "test_permission.tmp")
        with open(test_file, 'wb') as f:
            f.write(b'test')
        os.remove(test_file)
        print("Directory permission check passed")
    except Exception as e:
        print(f"Directory permission error: {str(e)}")
        return

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Listening on {PORT} at {BAUDRATE} baud...")
        
        buffer = bytearray()
        state = 'WAIT_HEADER'
        expected_len = 0
        last_activity = time.time()
        
        while True:
            # 检查超时
            if time.time() - last_activity > 5:
                print("Timeout waiting for data, resetting state...")
                state = 'WAIT_HEADER'
                buffer = bytearray()
                last_activity = time.time()
            
            if ser.in_waiting:
                byte = ser.read(1)
                last_activity = time.time()
                
                if state == 'WAIT_HEADER':
                    buffer.append(byte[0])
                    if len(buffer) >= 2:
                        print(f"Header bytes: {buffer[-2]:02X} {buffer[-1]:02X}")
                        if buffer[-2] == 0xAA and buffer[-1] == 0x55:
                            print("Header matched (AA 55)")
                            state = 'WAIT_LENGTH'
                        else:
                            buffer = bytearray()
                            
                elif state == 'WAIT_LENGTH':
                    buffer.append(byte[0])
                    if len(buffer) == 4:
                        expected_len = (buffer[2] << 8) | buffer[3]
                        print(f"Expecting {expected_len} bytes of data")
                        state = 'WAIT_DATA'
                        buffer = bytearray()
                        
                elif state == 'WAIT_DATA':
                    buffer.append(byte[0])
                    if len(buffer) == expected_len:
                        print(f"Received {len(buffer)} bytes")
                        if save_frame(buffer):
                            print("Frame saved successfully")
                        state = 'WAIT_HEADER'
                        buffer = bytearray()
            
            time.sleep(0.01)
                
    except serial.SerialException as e:
        print(f"Serial port error: {str(e)}")
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"Error: {str(e)}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    main()