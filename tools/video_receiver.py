import socket
import time
import os
import argparse
from datetime import datetime
import logging

def save_video_segment(data, segment_num, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    filename = os.path.join(
        output_dir,
        f"video_segment_{segment_num}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.mjpeg"
    )
    with open(filename, 'wb') as f:
        f.write(data)
    logger.info(f"Saved {len(data)} bytes to {filename}")
    print(f"Saved {len(data)} bytes to {filename}")

def setup_logging(output_dir):
    os.makedirs(output_dir, exist_ok=True)
    log_file = os.path.join(output_dir, 'video_receiver.log')
    logging.basicConfig(
        filename=log_file,
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    return logging.getLogger()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', '-o', default='../video_recordings',
                      help='Output directory for video segments')
    args = parser.parse_args()
    
    logger = setup_logging(args.output)
    logger.info("Starting video receiver")
    HOST = '0.0.0.0'  # 监听所有网络接口
    PORT = 8080
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        logger.info(f"Listening on port {PORT}...")
        logger.info("Local IP addresses:")
        hostname = socket.gethostname()
        local_ip = socket.gethostbyname(hostname)
        logger.info(f"  Primary IP: {local_ip}")
        print(f"Listening on port {PORT}...")
        print(f"Local IP addresses:")
        print(f"  Primary IP: {local_ip}")
        
        segment_num = 0
        segment_data = bytearray()
        
        while True:
            conn, addr = s.accept()
            print(f"Connected by {addr}")
            
            try:
                while True:
                    data = conn.recv(4096)
                    if not data:
                        break
                    segment_data.extend(data)
                    
                    # 简单检测帧边界
                    if b'--frame' in data[-100:]:
                        save_video_segment(segment_data, segment_num, args.output)
                        segment_num += 1
                        segment_data = bytearray()
                        
            except ConnectionResetError:
                logger.warning("Client disconnected")
                print("Client disconnected")
            finally:
                conn.close()

if __name__ == "__main__":
    main()