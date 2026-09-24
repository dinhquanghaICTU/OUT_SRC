#!/usr/bin/env python3
import os
import sys
import time
import subprocess

PROJECT_DIR = "/home/quanghaictu/OUT_SRC/OUT_SRC_TOOL/tunnnguyen"
BUILD_DIR = "/home/quanghaictu/OUT_SRC/OUT_SRC_TOOL/build-tunnnguyen-Desktop-Debug"
EXECUTABLE = os.path.join(BUILD_DIR, "tunnnguyen")

WATCH_EXTS = {".cpp", ".h", ".ui", ".qrc", ".qss", ".txt"}

def get_latest_mtime():
    latest = 0
    for root, _, files in os.walk(PROJECT_DIR):
        for f in files:
            ext = os.path.splitext(f)[1]
            if ext in WATCH_EXTS:
                path = os.path.join(root, f)
                try:
                    mtime = os.path.getmtime(path)
                    if mtime > latest:
                        latest = mtime
                except OSError:
                    pass
    return latest

def main():
    print("=" * 60)
    print("🚀 TUNNGUYEN AUTO-RELOAD WATCHER IS ACTIVE!")
    print(f"📁 Watching: {PROJECT_DIR}")
    print("💡 Vừa bấm Ctrl+S là script tự build, kill app cũ và bật app mới!")
    print("=" * 60)

    app_process = None

    def launch_app():
        nonlocal app_process
        if app_process and app_process.poll() is None:
            print("⏹️ Killing old instance...")
            app_process.terminate()
            try:
                app_process.wait(timeout=1.5)
            except subprocess.TimeoutExpired:
                app_process.kill()
        
        env = os.environ.copy()
        if "DISPLAY" not in env:
            env["DISPLAY"] = ":0"
            
        print("▶️ Launching new app...")
        app_process = subprocess.Popen([EXECUTABLE], env=env)

    # Initial build and run
    print("📦 Building project...")
    res = subprocess.run(["cmake", "--build", BUILD_DIR])
    if res.returncode == 0:
        launch_app()
    else:
        print("❌ Initial build failed!")

    last_mtime = get_latest_mtime()

    while True:
        time.sleep(0.5)
        current_mtime = get_latest_mtime()
        if current_mtime > last_mtime:
            last_mtime = current_mtime
            # Wait a tiny bit for editor to finish writing file
            time.sleep(0.15)
            print("\n" + "=" * 40)
            print("🔄 Phát hiện Ctrl+S (File thay đổi)! Đang tự động build lại...")
            build_res = subprocess.run(["cmake", "--build", BUILD_DIR])
            if build_res.returncode == 0:
                print("✅ Build thành công! Khởi động app mới...")
                launch_app()
            else:
                print("❌ Build có lỗi! Giữ nguyên app cũ để bạn sửa tiếp...")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n👋 Đã dừng Auto-Reload Watcher.")
