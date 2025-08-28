#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <thread>
#include <mutex>
#include <stdexcept>
using namespace std;

mutex logMutex;  // To prevent race conditions when logging from multiple threads

// Utility function for logging
void logMessage(const string& category, const string& message) {
    lock_guard<mutex> guard(logMutex); // Thread-safe logging
    ofstream logFile("update_log.txt", ios::app);
    if (!logFile) {
        cerr << "Error opening log file!" << endl;
        return;
    }
    time_t now = time(nullptr);
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    logFile << "[" << timeStr << "] [" << category << "] " << message << endl;
}

// Utility function to check if a command exists
bool commandExists(const string& cmd) {
    return system((cmd + " --version > /dev/null 2>&1").c_str()) == 0;
}

class OSUpdater {
public:
    virtual void checkForUpdates() = 0;
    virtual void performUpdate() = 0;
    virtual void handleDependencies() = 0;
    virtual void updateFirmware() = 0;
    virtual void updateCache() = 0;
    virtual void gatherSystemInfo() = 0;
    virtual ~OSUpdater() {}
};

class OSXUpdater : public OSUpdater {
public:
    void checkForUpdates() override {
        log("Checking for updates on macOS...");
        if (system("softwareupdate -l") != 0) {
            log("Failed to check for updates on macOS.");
        }
    }
    void performUpdate() override {
        log("Performing macOS update...");
        if (system("softwareupdate --install --all") != 0) {
            log("Failed to perform macOS update.");
        }
    }
    void handleDependencies() override {
        log("Handling macOS dependencies...");
        if (system("brew update && brew upgrade") != 0) {
            log("Failed to handle dependencies on macOS.");
        }
    }
    void updateFirmware() override {
        log("Updating firmware on macOS...");
        if (system("softwareupdate --fetch-full-installer") != 0) {
            log("Failed to update firmware on macOS.");
        }
        if (system("softwareupdate --install --all") != 0) {
            log("Failed to install macOS firmware.");
        }
    }
    void updateCache() override {
        log("Updating cache on macOS...");
        if (system("softwareupdate --fetch-full-installer") != 0) {
            log("Failed to update cache on macOS.");
        }
    }
    void gatherSystemInfo() override {
        log("Gathering system information for macOS...");
        if (system("system_profiler SPHardwareDataType SPSoftwareDataType SPDiskDataType") != 0) {
            log("Failed to gather system information on macOS.");
        }
        if (system("system_profiler SPFirmwareDataType") != 0) {
            log("Failed to gather firmware information on macOS.");
        }
        if (system("diskutil list") != 0) {
            log("Failed to gather disk information on macOS.");
        }
        if (system("brew list --versions") != 0) {
            log("Failed to list installed apps via brew on macOS.");
        }
    }
private:
    void log(const string& message) {
        logMessage("macOS", message);
    }
};

class WindowsUpdater : public OSUpdater {
public:
    void checkForUpdates() override {
        log("Checking for updates on Windows...");
        if (system("powershell -Command Get-WindowsUpdate") != 0) {
            log("Failed to check for updates on Windows.");
        }
    }
    void performUpdate() override {
        log("Performing Windows update...");
        if (system("powershell -Command Install-WindowsUpdate -AcceptAll -AutoReboot") != 0) {
            log("Failed to perform Windows update.");
        }
    }
    void handleDependencies() override {
        log("Handling Windows dependencies...");
        if (system("choco upgrade all -y") != 0) {
            log("Failed to handle dependencies on Windows.");
        }
    }
    void updateFirmware() override {
        log("Updating firmware on Windows...");
        if (system("fwupdmgr refresh") != 0) {
            log("Failed to refresh firmware on Windows.");
        }
        if (system("fwupdmgr update") != 0) {
            log("Failed to update firmware on Windows.");
        }
    }
    void updateCache() override {
        log("Updating cache on Windows...");
        if (system("powershell -Command Get-WindowsUpdate -Install") != 0) {
            log("Failed to update cache on Windows.");
        }
    }
    void gatherSystemInfo() override {
        log("Gathering system information for Windows...");
        if (system("systeminfo") != 0) {
            log("Failed to gather system info on Windows.");
        }
        if (system("wmic bios get smbiosbiosversion") != 0) {
            log("Failed to gather firmware info on Windows.");
        }
        if (system("wmic cpu get caption, deviceid, name, numberofcores, maxclockspeed") != 0) {
            log("Failed to gather CPU info on Windows.");
        }
        if (system("wmic diskdrive get model, size") != 0) {
            log("Failed to gather disk info on Windows.");
        }
        if (system("wmic product get name, version") != 0) {
            log("Failed to gather installed software on Windows.");
        }
        if (system("wmic nic get name, speed") != 0) {
            log("Failed to gather network info on Windows.");
        }
    }
private:
    void log(const string& message) {
        logMessage("Windows", message);
    }
};

class LinuxUpdater : public OSUpdater {
private:
    string distro;
public:
    LinuxUpdater(const string& distroType) : distro(distroType) {}
    void checkForUpdates() override {
        log("Checking for updates on " + distro + "...");
        if (distro == "Ubuntu" || distro == "Debian" || distro == "Mint") {
            if (!commandExists("apt-get")) {
                log("apt-get command not found, skipping update.");
                return;
            }
            if (system("sudo apt-get update") != 0) {
                log("Failed to update on " + distro);
            }
        } else if (distro == "RedHat" || distro == "CentOS" || distro == "Fedora") {
            if (!commandExists("dnf")) {
                log("dnf command not found, skipping update.");
                return;
            }
            if (system("sudo dnf check-update") != 0) {
                log("Failed to check for updates on " + distro);
            }
        } else if (distro == "Arch") {
            if (!commandExists("pacman")) {
                log("pacman command not found, skipping update.");
                return;
            }
            if (system("sudo pacman -Sy --noconfirm") != 0) {
                log("Failed to check for updates on " + distro);
            }
        } else {
            log("Unsupported Linux distribution detected for updates.");
        }
    }
    void performUpdate() override {
        log("Performing update on " + distro + "...");
        if (distro == "Ubuntu" || distro == "Debian" || distro == "Mint") {
            if (!commandExists("apt-get")) {
                log("apt-get command not found, skipping update.");
                return;
            }
            if (system("sudo apt-get upgrade -y") != 0) {
                log("Failed to upgrade on " + distro);
            }
        } else if (distro == "RedHat" || distro == "CentOS" || distro == "Fedora") {
            if (!commandExists("dnf")) {
                log("dnf command not found, skipping update.");
                return;
            }
            if (system("sudo dnf upgrade -y") != 0) {
                log("Failed to upgrade on " + distro);
            }
        } else if (distro == "Arch") {
            if (!commandExists("pacman")) {
                log("pacman command not found, skipping update.");
                return;
            }
            if (system("sudo pacman -Syu --noconfirm") != 0) {
                log("Failed to upgrade on " + distro);
            }
        } else {
            log("Unsupported Linux distribution detected for updates.");
        }
    }
    void handleDependencies() override {
        log("Handling dependencies on " + distro + "...");
        if (distro == "Ubuntu" || distro == "Debian" || distro == "Mint") {
            if (!commandExists("apt-get")) {
                log("apt-get command not found, skipping dependency handling.");
                return;
            }
            if (system("sudo apt-get dist-upgrade -y") != 0) {
                log("Failed to handle dependencies on " + distro);
            }
        } else if (distro == "RedHat" || distro == "CentOS" || distro == "Fedora") {
            if (!commandExists("dnf")) {
                log("dnf command not found, skipping dependency handling.");
                return;
            }
            if (system("sudo dnf distro-sync -y") != 0) {
                log("Failed to handle dependencies on " + distro);
            }
        } else if (distro == "Arch") {
            if (!commandExists("pacman")) {
                log("pacman command not found, skipping dependency handling.");
                return;
            }
            if (system("sudo pacman -S archlinux-keyring --noconfirm") != 0) {
                log("Failed to handle dependencies on " + distro);
            }
        } else {
            log("Unsupported Linux distribution detected for dependency handling.");
        }
    }
    void updateFirmware() override {
        log("Updating firmware on " + distro + "...");
        if (!commandExists("fwupdmgr")) {
            log("fwupdmgr not found, skipping firmware update.");
            return;
        }
        if (system("fwupdmgr refresh") != 0) {
            log("Failed to refresh firmware on " + distro);
        }
        if (system("fwupdmgr update") != 0) {
            log("Failed to update firmware on " + distro);
        }
    }
    void updateCache() override {
        log("Updating cache on " + distro + "...");
        if (distro == "Ubuntu" || distro == "Debian" || distro == "Mint") {
            if (!commandExists("apt-get")) {
                log("apt-get command not found, skipping cache update.");
                return;
            }
            if (system("sudo apt-get update") != 0) {
                log("Failed to update cache on " + distro);
            }
        } else if (distro == "RedHat" || distro == "CentOS" || distro == "Fedora") {
            if (!commandExists("dnf")) {
                log("dnf command not found, skipping cache update.");
                return;
            }
            if (system("sudo dnf makecache") != 0) {
                log("Failed to update cache on " + distro);
            }
        } else if (distro == "Arch") {
            if (!commandExists("pacman")) {
                log("pacman command not found, skipping cache update.");
                return;
            }
            if (system("sudo pacman -Sy --noconfirm") != 0) {
                log("Failed to update cache on " + distro);
            }
        }
    }
    void gatherSystemInfo() override {
        log("Gathering system information for " + distro + "...");
        if (system("uname -r") != 0) {
            log("Failed to gather kernel version on " + distro);
        }
        if (system("lscpu") != 0) {
            log("Failed to gather CPU info on " + distro);
        }
        if (system("free -h") != 0) {
            log("Failed to gather memory info on " + distro);
        }
        if (system("lsblk") != 0) {
            log("Failed to gather disk details on " + distro);
        }
        if (system("df -h") != 0) {
            log("Failed to gather disk usage on " + distro);
        }
        if (system("fwupdmgr get-devices") != 0) {
            log("Failed to gather firmware info on " + distro);
        }
        if (system("dpkg -l") != 0) {
            log("Failed to list installed packages (Debian-based) on " + distro);
        }
        if (system("rpm -qa") != 0) {
            log("Failed to list installed packages (RedHat-based) on " + distro);
        }
        if (system("pacman -Q") != 0) {
            log("Failed to list installed packages (Arch-based) on " + distro);
        }
        if (system("ifconfig -a") != 0) {
            log("Failed to gather network info on " + distro);
        }
        if (system("lspci") != 0) {
            log("Failed to gather hardware info on " + distro);
        }
        if (system("cat /etc/os-release") != 0) {
            log("Failed to gather OS version on " + distro);
        }
    }
private:
    void log(const string& message) {
        logMessage("Linux", message);
    }
};

class UpdaterManager {
private:
    unique_ptr<OSUpdater> updater;

    // Small helper: remove quotes from osType if present
    string stripQuotes(const string& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            return s.substr(1, s.size() - 2);
        }
        return s;
    }

public:
    void detectOS() {
        string osType;

        // Check for macOS
        ifstream osFile("/System/Library/CoreServices/SystemVersion.plist");
        if (osFile) {
            osType = "macOS";
        }
        // Check for Windows
        else if (system("ver > nul 2>&1") == 0) {
            osType = "Windows";
        }
        // Check for Linux
        else if (system("uname -s | grep -i 'Linux' > /dev/null 2>&1") == 0) {
            ifstream file("/etc/os-release");
            string line;
            while (getline(file, line)) {
                if (line.find("NAME=") != string::npos) {
                    osType = stripQuotes(line.substr(line.find('=') + 1));
                    break;
                }
            }
        }

        if (osType == "macOS") {
            updater = make_unique<OSXUpdater>();
        } else if (osType == "Windows") {
            updater = make_unique<WindowsUpdater>();
        } else if (osType == "Ubuntu" || osType == "Debian" || osType == "Linux Mint" ||
                   osType == "RedHat" || osType == "CentOS" || osType == "Fedora" || osType == "Arch") {
            updater = make_unique<LinuxUpdater>(osType);
        } else {
            logMessage("Error", "Unsupported OS detected.");
            return;
        }
        logMessage("OS Detected", "OS detected: " + osType);
    }

    void performUpdate() {
        if (updater) {
            try {
                updater->updateCache();
                updater->checkForUpdates();
                updater->performUpdate();
                updater->handleDependencies();
                updater->updateFirmware();
            } catch (const exception& e) {
                logMessage("Error", "Error during update: " + string(e.what()));
            }
        }
    }

    void gatherSystemInfo() {
        if (updater) {
            updater->gatherSystemInfo();
        }
    }
};

// ----------------------
// Main with argv options
// ----------------------
int main(int argc, char* argv[]) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--h" || arg == "--help") {
            cout << "Usage: updater [options]\n"
                 << "Options:\n"
                 << "  --h, --help   Show this help message\n"
                 << "No options: runs OS detection, gathers system info, and performs update.\n";
            return 0;
        }
    }

    UpdaterManager manager;
    manager.detectOS();        // Detect the OS
    manager.gatherSystemInfo();// Gather detailed system info
    manager.performUpdate();   // Run update sequence
    return 0;
};
