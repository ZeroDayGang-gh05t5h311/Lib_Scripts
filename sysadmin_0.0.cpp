#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <map>
#include <sstream>
#include <unistd.h> // for gethostname

using namespace std;

// Thread-safe logging utility
mutex logMutex;
ofstream logFile;

void logMessage(const string& message) {
    lock_guard<mutex> lock(logMutex);
    time_t now = time(0);
    string dt = ctime(&now);
    dt.pop_back();
    logFile << "[" << dt << "] " << message << endl;
    cout << message << endl;
}

// Abstract class
class OSUpdater {
public:
    virtual void updateCache() = 0;
    virtual void checkForUpdates() = 0;
    virtual void downloadUpdates() = 0;
    virtual void installUpdates() = 0;
    virtual void cleanUp() = 0;
    virtual ~OSUpdater() = default;
};

// MacOS implementation
class OSXUpdater : public OSUpdater {
private:
    void log(const string& msg) { logMessage("[MacOS] " + msg); }
public:
    void updateCache() override { log("Updating cache..."); system("brew update > /dev/null 2>&1"); }
    void checkForUpdates() override { log("Checking for updates..."); system("softwareupdate -l > /dev/null 2>&1"); }
    void downloadUpdates() override { log("Downloading updates..."); system("softwareupdate -d -a > /dev/null 2>&1"); }
    void installUpdates() override { log("Installing updates..."); system("softwareupdate -i -a > /dev/null 2>&1"); }
    void cleanUp() override { log("Cleaning up..."); system("brew cleanup > /dev/null 2>&1"); }
};

// Windows implementation
class WindowsUpdater : public OSUpdater {
private:
    void log(const string& msg) { logMessage("[Windows] " + msg); }
public:
    void updateCache() override { log("Updating cache..."); system("choco outdated > nul 2>&1"); }
    void checkForUpdates() override { log("Checking for updates..."); system("choco outdated > nul 2>&1"); }
    void downloadUpdates() override { log("Downloading updates..."); system("choco upgrade all -y --noop > nul 2>&1"); }
    void installUpdates() override { log("Installing updates..."); system("choco upgrade all -y > nul 2>&1"); }
    void cleanUp() override { log("Cleaning up..."); system("choco clean > nul 2>&1"); }
};

// Linux implementation
class LinuxUpdater : public OSUpdater {
private:
    void log(const string& msg) { logMessage("[Linux] " + msg); }

    bool commandExists(const string& cmd) {
        string check = "command -v " + cmd + " > /dev/null 2>&1";
        return (system(check.c_str()) == 0);
    }

public:
    void updateCache() override {
        log("Updating cache...");
        if (commandExists("apt-get")) system("sudo apt-get update > /dev/null 2>&1");
        else if (commandExists("dnf")) system("sudo dnf check-update > /dev/null 2>&1");
        else if (commandExists("zypper")) system("sudo zypper refresh > /dev/null 2>&1");
        else if (commandExists("pacman")) system("sudo pacman -Sy > /dev/null 2>&1");
        else log("No known package manager found.");
    }

    void checkForUpdates() override {
        log("Checking for updates...");
        if (commandExists("apt-get")) system("apt list --upgradable > /dev/null 2>&1");
        else if (commandExists("dnf")) system("dnf check-update > /dev/null 2>&1");
        else if (commandExists("zypper")) system("zypper lu > /dev/null 2>&1");
        else if (commandExists("pacman")) system("pacman -Qu > /dev/null 2>&1");
        else log("No known package manager found.");
    }

    void downloadUpdates() override {
        log("Downloading updates...");
        if (commandExists("apt-get")) system("sudo apt-get -d upgrade > /dev/null 2>&1");
        else if (commandExists("dnf")) system("sudo dnf upgrade --downloadonly > /dev/null 2>&1");
        else if (commandExists("zypper")) system("sudo zypper download > /dev/null 2>&1");
        else if (commandExists("pacman")) log("Pacman downloads during install.");
        else log("No known package manager found.");
    }

    void installUpdates() override {
        log("Installing updates...");
        if (commandExists("apt-get")) system("sudo apt-get upgrade -y > /dev/null 2>&1");
        else if (commandExists("dnf")) system("sudo dnf upgrade -y > /dev/null 2>&1");
        else if (commandExists("zypper")) system("sudo zypper update -y > /dev/null 2>&1");
        else if (commandExists("pacman")) system("sudo pacman -Su --noconfirm > /dev/null 2>&1");
        else log("No known package manager found.");
    }

    void cleanUp() override {
        log("Cleaning up...");
        if (commandExists("apt-get")) system("sudo apt-get autoremove -y > /dev/null 2>&1");
        else if (commandExists("dnf")) system("sudo dnf autoremove -y > /dev/null 2>&1");
        else if (commandExists("zypper")) system("sudo zypper clean > /dev/null 2>&1");
        else if (commandExists("pacman")) system("sudo pacman -Rns $(pacman -Qdtq) --noconfirm > /dev/null 2>&1");
        else log("No known package manager found.");
    }
};

// Updater manager
class UpdaterManager {
private:
    unique_ptr<OSUpdater> updater;
    string osType;

    string detectOS() {
        if (system("ver > nul 2>&1") == 0) return "Windows";
        if (system("uname -s > /dev/null 2>&1") == 0) {
            string line;
            ifstream osRelease("/etc/os-release");
            if (osRelease.is_open()) {
                while (getline(osRelease, line)) {
                    if (line.find("NAME=") == 0) {
                        string val = line.substr(5);
                        if (!val.empty() && val.front() == '"') val = val.substr(1, val.size() - 2);
                        return val;
                    }
                }
            }
            return "Linux";
        }
        return "MacOS";
    }

public:
    UpdaterManager() {
        osType = detectOS();
        if (osType.find("MacOS") != string::npos) updater = make_unique<OSXUpdater>();
        else if (osType.find("Windows") != string::npos) updater = make_unique<WindowsUpdater>();
        else updater = make_unique<LinuxUpdater>();
    }

    string getOS() { return osType; }

    void performUpdate() {
        updater->updateCache();
        updater->checkForUpdates();
        updater->downloadUpdates();
        updater->installUpdates();
        updater->cleanUp();
    }
};

// Utility for system info
string gatherSystemInfo() {
    ostringstream ss;
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        ss << "Hostname: " << hostname << "\n";
    }
    char* user = getenv("USER");
    if (user) ss << "User: " << user << "\n";
    time_t now = time(0);
    ss << "Current Time: " << ctime(&now);
    return ss.str();
}

void showHelp() {
    cout << "Usage:\n";
    cout << "  --h [-f file]     Perform system update (log to file, default system_update.log)\n";
    cout << "  --os [-f file]    Show detected OS (or log to file)\n";
    cout << "  --info [-f file]  Show system info (or log to file)\n";
    cout << "  --help            Show this help message\n";
}

int main(int argc, char* argv[]) {
    UpdaterManager manager;

    // default log file if none provided
    string logFilename = "system_update.log";

    if (argc > 1) {
        string arg1 = argv[1];

        if (arg1 == "--h") {
            if (argc > 3 && string(argv[2]) == "-f") {
                logFilename = argv[3];
            }
            logFile.open(logFilename, ios::app);
            manager.performUpdate();
            logFile.close();
            return 0;
        }
        else if (arg1 == "--os") {
            string os = manager.getOS();
            if (argc > 3 && string(argv[2]) == "-f") {
                ofstream out(argv[3]);
                out << "Detected OS: " << os << endl;
            } else {
                cout << "Detected OS: " << os << endl;
            }
            return 0;
        }
        else if (arg1 == "--info") {
            string info = gatherSystemInfo();
            if (argc > 3 && string(argv[2]) == "-f") {
                ofstream out(argv[3]);
                out << info;
            } else {
                cout << info;
            }
            return 0;
        }
        else if (arg1 == "--help") {
            showHelp();
            return 0;
        }
        else {
            cout << "Unknown option: " << arg1 << "\n";
            showHelp();
            return 1;
        }
    }

    cout << "No arguments provided.\n";
    showHelp();
    return 1;
}
