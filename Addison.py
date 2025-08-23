#!/usr/bin/python3
import random, sys, os
class tool:
    @staticmethod
    def getInput(ios, arg):
        if ios:  # If ios is True, return as integer
            return int(input(f"{arg}"))
        else:
            return input(f"{arg}")  # Return as string
    @staticmethod
    def cmd(arg):
        os.system(f"{arg}")
    @staticmethod
    def rtn_rdm(imin, imax):
        return random.randint(imin, imax)  # Return a random number set by the params
    @staticmethod
    def mdir():
        dname = tool.getInput(False, "Directory name please: ")
        tool.cmd(f"mkdir {dname}")
        print(f"OK, have made a directory called: '{dname}'\nPATH: {os.getcwd()}")
    @staticmethod
    def read(fname):
        try:
            with open(fname, "r") as tmp_var:
                tmp_var.read()
        except FileNotFoundError:
            print("File not found!")
    @staticmethod
    def write(fname):
        tmp_data = tool.getInput(False, "> ")
        try:
            with open(fname, "a+") as tmp_var:
                tmp_var.write(tmp_data)
        except FileNotFoundError:
            print("File not found!")
    @staticmethod
    def appendFile():
        apfname = tool.getInput(False, "Filename please.\n$: ")
        tofile = tool.getInput(False, "To add to the file...$: ")
        with open(apfname, "a") as fileOpen:
            fileOpen.write(f"\n{tofile}")
        print("Written to file...")
    @staticmethod
    def sfile(filename, search):
        os.system(f"cat {filename} | grep -i {search}")
    @staticmethod
    def mkpasswd():
        letters = "abcdefghijklmnopqrstuvwxyz"
        numbs = "0123456789"
        special = "!^*£$"
        passwd = random.choice(letters)  # Must start with a letter
        for _ in range(7):  # Target length 8
            randSel = random.randint(0, 2)
            if randSel == 0:
                passwd += random.choice(letters)
            elif randSel == 1:
                passwd += random.choice(numbs)
            else:
                passwd += random.choice(special)
        print(f"Password is: {passwd}!\nThe length is: {len(passwd)}!")
        return passwd
    @staticmethod
    def guess():
        print("Ok... this is a guessing game, you will be told the range of numbers, there are 5 ranges of difficulty")
        rdm_diff_select = [[0, 2], [0, 4], [0, 5], [0, 6], [0, 9]]
        tmp_str = "Please select a difficulty: 1-5 (default is 2)."
        tmp = 2
        try:
            tmp = tool.getInput(True, tmp_str)
        except ValueError:
            print("Value Error... you tried I guess.")
        finally:
            player_int = tool.rtn_rdm(*rdm_diff_select[tmp - 1])
            cpu_int = tool.rtn_rdm(*rdm_diff_select[tmp - 1])
            print(f"Your number is: {player_int}.\nComputer's is: {cpu_int}.")
    @staticmethod
    def calc():
        try:
            tmp = tool.getInput(False, "Please type a sum, E.G: '1+1'")
            etmp = eval(tmp)
        except NameError:
            print("Name error, did you enter a sum correctly?")
        finally:
            print(f"{etmp}")
    @staticmethod
    def local():
        fname = "local_system_infomation.txt\n"
        tool.cmd(f"w -i -p > {fname}")
        tool.cmd(f"who -a >>  {fname}")
        tool.cmd(f"service --status-all >> {fname}")
        tool.cmd(f"netstat -tuln >> {fname}")
    @staticmethod
    def osi():
        print("""
        6) Application: Network process to application. DNS WWW/HTTP/HTTPS/P2P,E-Mail,POP,SMTP,Telnet,FTP.
        5) Presentation: Data representation and encryption. HTML,DOC,JPEG,MP3,AVI.
        4) Session: Inter host communication. TCP,SIP,RTP.
        3) Transport: End-To-End connections and reliability.TCP,UDP,SCTP,SSL,TLS.
        2) Path Determination and logical addressing: IP,IPSec,ICMP,IGMP,OSPF.
        1) Data Link: Ethernet, 802.11,MAC/LLC,VLAN,ATM,HDP,PPP,Q,921,Token Ring,ARP.
        0) Physical: Media signal and binary transmission. RS-232,RJ45,V,100BASE-TX,SDH,DSL,802.11
        """)
    @staticmethod
    def ohd():
        os.system("man ascii")
    @staticmethod
    def wdh():
        domain = tool.getInput(False, "Domain name please(just the name and the tld(e.g google.com).\n$: ")
        std = tool.getInput(False, "Would you like to save it to disk? (y/n)").lower()
        fileName = tool.getInput(False, "Please pick a filename: ")
        if std in ["yes", "y"]:
            std = True
        else:
            std = False
        if not std:
            print("Just getting the information...")
            tool.cmd(f"whois {domain}")
            tool.cmd(f"dig  {domain}")
            tool.cmd(f"host {domain}")
        else:
            tool.cmd(f"whois > {fileName}")
            tool.cmd(f"dig >> {fileName}")
            tool.cmd(f"host >> {fileName}")
    @staticmethod
    def pchk():
        tool.cmd(f"netstat -pnltu | grep {tool.getInput(True, 'Port Please: ')}")
    cmds = [
        "help: this help list.",
        "mdir: makes a directory.",
        "read: opens and reads a file.",
        "write: writes to a file.",
        "append: Appends to a file.",
        "sfile: search a file",
        "password: makes a random password.",
        "guess: runs a guessing game.",
        "calc: a simple calculation function.",
        "local: prints local system information.",
        "osi: displays information about the OSI model.",
        "ohd: displays octal/hex/decimal conversions.",
        "wdh: does lookups for a given domain.",
        "pchk: checks services running on a specific port."
    ]
    @staticmethod
    def icmd():
        print("Hi, welcome to the console. Yes, you can have 'help'.")
        tmp = tool.getInput(False, "> ")
        if tmp == "exit":
            exit()
        elif tmp == "help":
            for each in tool.cmds:
                print(f"{each}")
        elif tmp == "read":
            tool.read(tool.getInput(False, "Filename:\n> "))
        elif tmp == "write":
            tool.write(tool.getInput(False, "Filename:\n> "))
        elif tmp == "append":
            tool.appendFile()
        elif tmp == "mkpasswd":
            print(tool.mkpasswd())
        elif tmp == "guess":
            tool.guess()
        elif tmp == "calc":
            tool.calc()
        elif tmp == "local":
            tool.local()
        elif tmp == "osi":
            tool.osi()
        elif tmp == "ohd":
            tool.ohd()
        elif tmp == "sfile":
            fn = tool.getInput(False,"Filename: ")
            s = tool.getInput(False,"Search String: ")
            tool.sfile()
        elif tmp == "wdh":
            tool.wdh()
        elif tmp == "pchk":
            tool.pchk()
tmp = " "  # Keeps the loop running with minimal resources
while tmp != "exit":
    tmp = tool.icmd()
