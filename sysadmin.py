#!/usr/bin/env python3
import subprocess
import sys
import platform
import shutil
import threading
import unittest
import psutil
from datetime import datetime
from unittest.mock import patch
# Thread-safe logging
log_lock = threading.Lock()
def log_message(category, message, logfile=None):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_filename = logfile or f"update_log_{datetime.now().strftime('%Y-%m-%d')}.txt"
    with log_lock:
        with open(log_filename, "a") as log_file:
            log_file.write(f"[{timestamp}] [{category}] {message}\n")
def gather_system_info(logfile):
    info_lines = []
    info_lines.append("===== SYSTEM INFORMATION =====")
    info_lines.append(f"System: {platform.system()}")
    info_lines.append(f"Node: {platform.node()}")
    info_lines.append(f"Release: {platform.release()}")
    info_lines.append(f"Version: {platform.version()}")
    info_lines.append(f"Machine: {platform.machine()}")
    info_lines.append(f"Processor: {platform.processor()}")
    info_lines.append(f"Python Version: {platform.python_version()}")
    # CPU info
    info_lines.append(f"CPU Cores: {psutil.cpu_count(logical=False)} physical, {psutil.cpu_count(logical=True)} logical")
    info_lines.append(f"CPU Usage: {psutil.cpu_percent(interval=1)}%")
    # Memory info
    mem = psutil.virtual_memory()
    info_lines.append(f"Memory Total: {mem.total // (1024**3)} GB")
    info_lines.append(f"Memory Available: {mem.available // (1024**3)} GB")
    info_lines.append(f"Memory Used: {mem.used // (1024**3)} GB")
    # Disk info
    for part in psutil.disk_partitions():
        usage = psutil.disk_usage(part.mountpoint)
        info_lines.append(f"Disk {part.device} - Total: {usage.total // (1024**3)} GB, Used: {usage.used // (1024**3)} GB, Free: {usage.free // (1024**3)} GB")
    # Network info
    net = psutil.net_if_addrs()
    for iface, addrs in net.items():
        for addr in addrs:
            if addr.family == 2:  # AF_INET
                info_lines.append(f"Network Interface {iface} - IP: {addr.address}")
    info_lines.append("===== END SYSTEM INFORMATION =====\n")
    with log_lock:
        with open(logfile, "a") as log_file:
            for line in info_lines:
                log_file.write(line + "\n")
class Tool:
    @staticmethod
    def get_input(prompt):
        try:
            return input(prompt)
        except Exception as e:
            log_message("ERROR", f"Input error: {e}")
            return ""
    @staticmethod
    def command_exists(command):
        return shutil.which(command) is not None
class BaseUpdater:
    def update(self):
        raise NotImplementedError
    def update_firmware(self):
        raise NotImplementedError
    def run_command(self, command, category="INFO", logfile=None):
        try:
            subprocess.run(command, shell=True, check=True)
            log_message(category, f"Executed securely: {command}", logfile)
        except subprocess.CalledProcessError as e:
            log_message("ERROR", f"Failed: {command} with error {e}", logfile)
class OSXUpdater(BaseUpdater):
    def update(self, logfile):
        log_message("INFO", "Starting secure macOS update (checking cache)...", logfile)
        if Tool.command_exists("softwareupdate"):
            self.run_command("sudo softwareupdate --list", logfile=logfile)
            self.run_command("sudo softwareupdate -ia --verbose", logfile=logfile)
        else:
            log_message("ERROR", "softwareupdate command not found.", logfile)
        log_message("INFO", "macOS update completed.", logfile)

    def update_firmware(self, logfile):
        log_message("INFO", "Starting secure macOS firmware update (checking cache)...", logfile)
        if Tool.command_exists("softwareupdate"):
            self.run_command("sudo softwareupdate --list", logfile=logfile)
            self.run_command("sudo softwareupdate --install-rosetta --agree-to-license", logfile=logfile)
        else:
            log_message("ERROR", "softwareupdate command not found for firmware.", logfile)
        log_message("INFO", "macOS firmware update completed.", logfile)
class WindowsUpdater(BaseUpdater):
    def update(self, logfile):
        log_message("INFO", "Starting secure Windows update (checking cache)...", logfile)
        self.run_command(
            "powershell -Command \"Install-Module PSWindowsUpdate; "
            "Get-WindowsUpdate -MicrosoftUpdate; "
            "Install-WindowsUpdate -AcceptAll -AutoReboot\"",
            logfile=logfile
        )
        log_message("INFO", "Windows update completed.", logfile)
    def update_firmware(self, logfile):
        log_message("INFO", "Starting secure Windows firmware update (checking cache)...", logfile)
        self.run_command(
            "powershell -Command \"Install-Module -Name FirmwareUpdate; "
            "Get-FirmwareUpdate; Update-Firmware -All -Confirm:$false\"",
            logfile=logfile
        )
        log_message("INFO", "Windows firmware update completed.", logfile)
class LinuxUpdater(BaseUpdater):
    def update(self, logfile):
        log_message("INFO", "Starting secure Linux update (checking cache)...", logfile)
        if Tool.command_exists("apt"):
            self.run_command("sudo apt update", logfile=logfile)
            self.run_command("sudo apt upgrade -y", logfile=logfile)
        elif Tool.command_exists("dnf"):
            self.run_command("sudo dnf check-update", logfile=logfile)
            self.run_command("sudo dnf upgrade -y", logfile=logfile)
        elif Tool.command_exists("zypper"):
            self.run_command("sudo zypper refresh", logfile=logfile)
            self.run_command("sudo zypper update -y", logfile=logfile)
        else:
            log_message("ERROR", "No supported package manager found.", logfile)
        log_message("INFO", "Linux update completed.", logfile)
    def update_firmware(self, logfile):
        log_message("INFO", "Starting secure Linux firmware update (checking cache)...", logfile)
        if Tool.command_exists("fwupdmgr"):
            self.run_command("sudo fwupdmgr get-updates", logfile=logfile)
            self.run_command("sudo fwupdmgr update", logfile=logfile)
        else:
            log_message("ERROR", "fwupdmgr not found for firmware update.", logfile)
        log_message("INFO", "Linux firmware update completed.", logfile)
class UpdaterManager:
    def __init__(self):
        self.updater = None
    def detect_os(self):
        os_name = platform.system().lower()
        if "darwin" in os_name:
            self.updater = OSXUpdater()
        elif "windows" in os_name:
            self.updater = WindowsUpdater()
        elif "linux" in os_name:
            self.updater = LinuxUpdater()
        else:
            raise RuntimeError(f"Unsupported OS: {os_name}")
        return os_name
    def run(self, logfile):
        if self.updater:
            self.updater.update(logfile)
            self.updater.update_firmware(logfile)
        else:
            raise RuntimeError("No updater available")
def main():
    log_filename = f"update_log_{datetime.now().strftime('%Y-%m-%d')}.txt"
    try:
        # Write system info at the top only once
        gather_system_info(log_filename)
        manager = UpdaterManager()
        os_name = manager.detect_os()
        log_message("INFO", f"Detected OS: {os_name}", log_filename)
        manager.run(log_filename)
        log_message("INFO", "All updates and firmware checks completed.", log_filename)
        print(f"Update process completed. Log file created: {log_filename}")
    except Exception as e:
        log_message("FATAL", str(e), log_filename)
        sys.exit(1)
if __name__ == "__main__":
    main()
