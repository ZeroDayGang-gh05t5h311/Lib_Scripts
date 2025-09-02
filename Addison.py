#!/usr/bin/python3

import random, sys, os, subprocess, ast, operator

# -------- SAFE CALCULATOR -------- #
class SafeCalc:
    OPS = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
        ast.FloorDiv: operator.floordiv,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
    }

    @staticmethod
    def eval_expr(expr):
        """Safely evaluate a math expression using AST parsing."""
        node = ast.parse(expr, mode="eval").body
        return SafeCalc._eval(node)

    @staticmethod
    def _eval(node):
        if isinstance(node, ast.BinOp):
            left = SafeCalc._eval(node.left)
            right = SafeCalc._eval(node.right)
            return SafeCalc.OPS[type(node.op)](left, right)
        elif isinstance(node, ast.UnaryOp):
            operand = SafeCalc._eval(node.operand)
            return SafeCalc.OPS[type(node.op)](operand)
        elif isinstance(node, ast.Num):  # Python 3.7 and below
            return node.n
        elif isinstance(node, ast.Constant):  # Python 3.8+
            if isinstance(node.value, (int, float)):
                return node.value
            else:
                raise ValueError("Only numbers allowed")
        else:
            raise TypeError(f"Unsupported expression: {ast.dump(node)}")

# -------- MAIN TOOL CLASS -------- #
class tool:
    @staticmethod
    def getInput(ios, arg):
        if ios:  # If ios is True, return as integer
            return int(input(f"{arg}"))
        else:
            return input(f"{arg}")  # Return as string

    @staticmethod
    def cmd(args, capture=False):
        """Run a system command safely with subprocess."""
        try:
            if isinstance(args, str):
                args = args.split()
            if capture:
                result = subprocess.run(args, capture_output=True, text=True, check=True)
                return result.stdout
            else:
                subprocess.run(args, check=True)
        except subprocess.CalledProcessError as e:
            print(f"[ERROR] Command failed: {e}")

    @staticmethod
    def mdir():
        dname = tool.getInput(False, "Directory name please: ")
        tool.cmd(["mkdir", "-p", dname])   # -p avoids crash if dir exists
        print(f"OK, have made a directory called: '{dname}'\nPATH: {os.getcwd()}")

    @staticmethod
    def read(fname):
        try:
            with open(fname, "r") as tmp_var:
                print(tmp_var.read())
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
        try:
            result = tool.cmd(["grep", "-i", search, filename], capture=True)
            print(result if result else "No matches found.")
        except Exception:
            print("No matches found.")

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

        print(f"Password is: {passwd}\nLength: {len(passwd)}")
        return passwd

    @staticmethod
    def guess():
        print("Ok... guessing game, 5 difficulty levels")
        rdm_diff_select = [[0, 2], [0, 4], [0, 5], [0, 6], [0, 9]]
        tmp = 2
        try:
            tmp = tool.getInput(True, "Please select difficulty 1-5 (default 2): ")
        except ValueError:
            print("Value Error... defaulting to 2")

        player_int = random.randint(*rdm_diff_select[tmp - 1])
        cpu_int = random.randint(*rdm_diff_select[tmp - 1])
        print(f"Your number is: {player_int}.\nComputer's is: {cpu_int}.")

    @staticmethod
    def calc():
        try:
            expr = tool.getInput(False, "Please type a sum, e.g. '1+2*3': ")
            result = SafeCalc.eval_expr(expr)
            print(f"= {result}")
        except Exception as e:
            print(f"Error: {e}")

    @staticmethod
    def local():
        fname = "local_system_information.txt"
        with open(fname, "w") as f:
            f.write(tool.cmd(["w", "-i", "-p"], capture=True) or "")
            f.write(tool.cmd(["who", "-a"], capture=True) or "")
            f.write(tool.cmd(["service", "--status-all"], capture=True) or "")
            f.write(tool.cmd(["netstat", "-tuln"], capture=True) or "")
        print(f"System info written to {fname}")

    @staticmethod
    def osi():
        print("""
6) Application: DNS, HTTP/HTTPS, Email, FTP
5) Presentation: Data representation (HTML,DOC,JPEG,MP3)
4) Session: Inter host communication (TCP,SIP,RTP)
3) Transport: End-to-End (TCP,UDP,TLS)
2) Network: IP, ICMP, OSPF
1) Data Link: Ethernet, 802.11, ARP
0) Physical: Binary transmission (RJ45, DSL, Wi-Fi)
        """)

    @staticmethod
    def ohd():
        tool.cmd(["man", "ascii"])

    @staticmethod
    def wdh():
        domain = tool.getInput(False, "Domain name please (e.g google.com):\n$: ")
        save = tool.getInput(False, "Save to disk? (y/n): ").lower()
        fileName = tool.getInput(False, "Please pick a filename: ")

        if save in ["yes", "y"]:
            with open(fileName, "w") as f:
                f.write(tool.cmd(["whois", domain], capture=True) or "")
                f.write(tool.cmd(["dig", domain], capture=True) or "")
                f.write(tool.cmd(["host", domain], capture=True) or "")
            print(f"Results saved to {fileName}")
        else:
            print(tool.cmd(["whois", domain], capture=True))
            print(tool.cmd(["dig", domain], capture=True))
            print(tool.cmd(["host", domain], capture=True))

    @staticmethod
    def pchk():
        port = tool.getInput(True, "Port Please: ")
        output = tool.cmd(["netstat", "-pnltu"], capture=True)
        if output:
            for line in output.splitlines():
                if f":{port}" in line:
                    print(line)

    cmds = [
        "help: this help list.",
        "mdir: makes a directory.",
        "read: opens and reads a file.",
        "write: writes to a file.",
        "append: appends to a file.",
        "sfile: search a file.",
        "mkpasswd: makes a random password.",
        "guess: runs a guessing game.",
        "calc: a simple calculator.",
        "local: prints local system information.",
        "osi: displays OSI model info.",
        "ohd: displays ASCII conversions.",
        "wdh: whois/dig/host lookups.",
        "pchk: checks services on a port.",
    ]

    @staticmethod
    def icmd():
        print("Hi, welcome to the console. Type 'help' for options.")
        tmp = tool.getInput(False, "> ")

        if tmp == "exit":
            return "exit"
        elif tmp == "help":
            for each in tool.cmds:
                print(each)
        elif tmp == "mdir":
            tool.mdir()
        elif tmp == "read":
            tool.read(tool.getInput(False, "Filename:\n> "))
        elif tmp == "write":
            tool.write(tool.getInput(False, "Filename:\n> "))
        elif tmp == "append":
            tool.appendFile()
        elif tmp == "sfile":
            fn = tool.getInput(False,"Filename: ")
            s = tool.getInput(False,"Search String: ")
            tool.sfile(fn, s)
        elif tmp == "mkpasswd":
            tool.mkpasswd()
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
        elif tmp == "wdh":
            tool.wdh()
        elif tmp == "pchk":
            tool.pchk()

        return " "

# -------- MAIN LOOP -------- #
tmp = ""
while tmp != "exit":
    tmp = tool.icmd()
