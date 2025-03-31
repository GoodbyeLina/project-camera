import serial
import os
import time
from datetime import datetime
from PIL import Image
import io
import tkinter as tk
from tkinter import ttk

# 配置串口参数
PORT = 'COM8'
BAUDRATE = 115200
OUTPUT_DIR = r'C:\Users\97978\Desktop\esp_camera'
os.makedirs(OUTPUT_DIR, exist_ok=True)

def save_frame(data):
    try:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        # 保存为JPEG文件
        jpg_filename = os.path.join(OUTPUT_DIR, f"frame_{timestamp}.jpg")
        with open(jpg_filename, 'wb') as f:
            f.write(data)
        
        print(f"Saved JPEG to {jpg_filename}")
        
        # 显示图像
        try:
            img = Image.open(io.BytesIO(data))
            img.show()
        except Exception as e:
            print(f"Image display error: {str(e)}")
        
        return True
    except Exception as e:
        print(f"Save failed: {str(e)}")
        return False

class CameraViewer:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Camera Viewer")
        
        # 图像显示区域
        self.image_label = ttk.Label(root)
        self.image_label.pack()
        
        # 状态栏
        self.status_var = tk.StringVar()
        self.status_var.set("等待连接...")
        status_bar = ttk.Label(root, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.pack(fill=tk.X)
        
    def update_image(self, img_data):
        try:
            img = Image.open(io.BytesIO(img_data))
            img.thumbnail((640, 480))
            photo = ImageTk.PhotoImage(img)
            
            self.image_label.configure(image=photo)
            self.image_label.image = photo
            self.status_var.set(f"收到图像: {len(img_data)}字节")
        except Exception as e:
            self.status_var.set(f"图像显示错误: {str(e)}")

def main():
    root = tk.Tk()
    viewer = CameraViewer(root)
    
    def serial_thread():
        print(f"Output directory: {OUTPUT_DIR}")
        try:
            ser = serial.Serial(PORT, BAUDRATE, timeout=1)
            viewer.status_var.set(f"已连接 {PORT} @ {BAUDRATE} baud")
            
            buffer = bytearray()
            state = 'WAIT_HEADER'
            expected_len = 0
            last_activity = time.time()
            
            while True:
                if time.time() - last_activity > 5:
                    viewer.status_var.set("等待数据...")
                    state = 'WAIT_HEADER'
                    buffer = bytearray()
                    last_activity = time.time()
                
                if ser.in_waiting:
                    byte = ser.read(1)
                    last_activity = time.time()
                    
                    if state == 'WAIT_HEADER':
                        buffer.append(byte[0])
                        if len(buffer) >= 2 and buffer[-2] == 0xAA and buffer[-1] == 0x55:
                            state = 'WAIT_LENGTH'
                            buffer = bytearray()
                            
                    elif state == 'WAIT_LENGTH':
                        buffer.append(byte[0])
                        if len(buffer) == 4:
                            expected_len = (buffer[2] << 8) | buffer[3]
                            state = 'WAIT_DATA'
                            buffer = bytearray()
                            
                    elif state == 'WAIT_DATA':
                        buffer.append(byte[0])
                        if len(buffer) == expected_len:
                            root.after(0, viewer.update_image, bytes(buffer))
                            save_frame(bytes(buffer))
                            state = 'WAIT_HEADER'
                            buffer = bytearray()
                
                time.sleep(0.01)
                
        except Exception as e:
            viewer.status_var.set(f"错误: {str(e)}")
        finally:
            if 'ser' in locals() and ser.is_open:
                ser.close()
    
    import threading
    thread = threading.Thread(target=serial_thread, daemon=True)
    thread.start()
    
    root.mainloop()

if __name__ == '__main__':
    main()