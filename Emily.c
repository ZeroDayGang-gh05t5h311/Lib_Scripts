#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#define OS_WIN 1
#else
#define OS_WIN 0
#endif

// -------- Utilities --------
static void trim_newline(char *s){ if(!s) return; size_t n=strlen(s); if(n && (s[n-1]=='\n'||s[n-1]=='\r')) s[n-1]='\0'; }
static void prompt(const char* p){ printf("%s", p); fflush(stdout); }

static int getInputInt(const char *msg){
    char buf[256];
    prompt(msg);
    if(!fgets(buf,sizeof(buf),stdin)) return 0;
    return atoi(buf);
}
static void getInputStr(const char *msg, char *out, size_t cap){
    prompt(msg);
    if(fgets(out,cap,stdin)) trim_newline(out); else out[0]='\0';
}

// Capture an external command's stdout into a malloc'd buffer (caller frees).
static char* cmd_capture(const char *cmd){
    FILE *fp = popen(cmd, "r");
    if(!fp) return NULL;
    size_t cap = 4096, len = 0;
    char *buf = (char*)malloc(cap); if(!buf){ pclose(fp); return NULL; }
    buf[0]='\0';
    char tmp[1024];
    while(fgets(tmp,sizeof(tmp),fp)){
        size_t need = len + strlen(tmp) + 1;
        if(need > cap){ cap = need*2; char *nb=(char*)realloc(buf,cap); if(!nb){ free(buf); pclose(fp); return NULL; } buf=nb; }
        strcpy(buf+len, tmp); len += strlen(tmp);
    }
    pclose(fp);
    return buf;
}
static int cmd_run(const char *cmd){
    int rc = system(cmd);
    if(rc != 0) fprintf(stderr,"[ERROR] Command failed: %s (rc=%d)\n", cmd, rc);
    return rc;
}

// -------- SafeCalc (recursive-descent for +,-,*,/,%,^ and unary +/-; integer // as floor div) --------
typedef struct { const char *s; size_t i; } Parser;

static void skip_ws(Parser *p){ while(isspace((unsigned char)p->s[p->i])) p->i++; }
static int match(Parser *p, char c){ skip_ws(p); if(p->s[p->i]==c){ p->i++; return 1;} return 0; }

static double parse_expr(Parser* p); // forward

static double parse_number(Parser* p){
    skip_ws(p);
    char *end = NULL;
    double v = strtod(p->s + p->i, &end);
    if(end == p->s + p->i) { fprintf(stderr,"Error: expected number near '%.8s'\n", p->s + p->i); return 0.0; }
    p->i = (size_t)(end - p->s);
    return v;
}

// factor: (+|-) factor | number | '(' expr ')' 
static double parse_factor(Parser* p){
    skip_ws(p);
    if(match(p,'+')) return parse_factor(p);
    if(match(p,'-')) return -parse_factor(p);
    if(match(p,'(')){ double v=parse_expr(p); if(!match(p,')')) fprintf(stderr,"Error: missing ')'\n"); return v; }
    return parse_number(p);
}

// power: factor (^ factor)*
static double parse_power(Parser* p){
    double v = parse_factor(p);
    skip_ws(p);
    while(match(p,'^')){ // exponent
        double r = parse_factor(p);
        // pow
        // simple pow for doubles
        // include math.h would be ideal, but to keep deps minimal:
        // quick pow via libc:
        extern double pow(double, double);
        v = pow(v,r);
        skip_ws(p);
    }
    return v;
}

// term: power ((*|/|%) power)*
static double parse_term(Parser* p){
    double v = parse_power(p);
    skip_ws(p);
    for(;;){
        if(match(p,'*')){ v *= parse_power(p); }
        else if(match(p,'/')){ double r=parse_power(p); v /= r; }
        else if(match(p,'%')){ double r=parse_power(p); long long a=(long long)v, b=(long long)r; v = (double)(a % b); }
        else return v;
        skip_ws(p);
    }
}

// expr: term ((+|-) term | '//' term )*
static double parse_expr(Parser* p){
    double v = parse_term(p);
    skip_ws(p);
    for(;;){
        if(match(p,'+')) v += parse_term(p);
        else if(match(p,'-')) v -= parse_term(p);
        else if(p->s[p->i]=='/' && p->s[p->i+1]=='/'){ p->i+=2; double r=parse_term(p); // floor div (int)
            long long a=(long long)v, b=(long long)r; v = (double)(a / b);
        }
        else return v;
        skip_ws(p);
    }
}
static int safe_eval(const char *expr, double *out){
    Parser p = { expr, 0 };
    *out = parse_expr(&p);
    skip_ws(&p);
    if(p.s[p.i] != '\0'){ fprintf(stderr,"Error: trailing characters near '%s'\n", p.s + p.i); return 0; }
    return 1;
}

// -------- Features --------
static void mdir(){
    char name[512];
    getInputStr("Directory name please: ", name, sizeof(name));
#if OS_WIN
    char cmd[1024]; snprintf(cmd,sizeof(cmd),"mkdir \"%s\"", name);
#else
    char cmd[1024]; snprintf(cmd,sizeof(cmd),"mkdir -p \"%s\"", name);
#endif
    cmd_run(cmd);
    char cwd[1024]; if(getcwd(cwd,sizeof(cwd))) printf("OK, have made a directory called: '%s'\nPATH: %s\n", name, cwd);
}

static void read_file(){
    char fname[512];
    getInputStr("Filename:\n> ", fname, sizeof(fname));
    FILE *f = fopen(fname, "r");
    if(!f){ puts("File not found!"); return; }
    char buf[1024];
    while(fgets(buf,sizeof(buf),f)) fputs(buf, stdout);
    fclose(f);
}

static void write_file(){
    char fname[512]; getInputStr("Filename:\n> ", fname, sizeof(fname));
    char text[2048]; getInputStr("> ", text, sizeof(text));
    FILE *f = fopen(fname, "a+");
    if(!f){ puts("File not found!"); return; }
    fputs(text, f);
    fclose(f);
}

static void append_file(){
    char fname[512]; getInputStr("Filename please.\n$: ", fname, sizeof(fname));
    char text[2048]; getInputStr("To add to the file...$: ", text, sizeof(text));
    FILE *f = fopen(fname, "a");
    if(!f){ puts("File not found!"); return; }
    fputc('\n', f); fputs(text, f); fclose(f);
    puts("Written to file...");
}

static void sfile(){
    char fname[512]; getInputStr("Filename: ", fname, sizeof(fname));
    char search[512]; getInputStr("Search String: ", search, sizeof(search));
#if OS_WIN
    char cmd[2048]; snprintf(cmd,sizeof(cmd),"findstr /I /C:\"%s\" \"%s\"", search, fname);
#else
    char cmd[2048]; snprintf(cmd,sizeof(cmd),"grep -i -- \"%s\" \"%s\"", search, fname);
#endif
    char *out = cmd_capture(cmd);
    if(out && out[0]){ printf("%s", out); free(out); } else { puts("No matches found."); if(out) free(out); }
}

static void mkpasswd(){
    const char *letters="abcdefghijklmnopqrstuvwxyz";
    const char *numbs="0123456789";
    const char *special="!^*£$";
    char pass[64] = {0};
    srand((unsigned)time(NULL));
    pass[0] = letters[rand()%26];
    for(int i=1;i<8;i++){
        int sel = rand()%3;
        if(sel==0) pass[i]=letters[rand()%26];
        else if(sel==1) pass[i]=numbs[rand()%10];
        else pass[i]=special[rand()%5];
    }
    pass[8]='\0';
    printf("Password is: %s\nLength: %zu\n", pass, strlen(pass));
}

static void guess(){
    puts("Ok... guessing game, 5 difficulty levels");
    int ranges[5][2]={{0,2},{0,4},{0,5},{0,6},{0,9}};
    int d = getInputInt("Please select difficulty 1-5 (default 2): ");
    if(d<1||d>5){ puts("Value Error... defaulting to 2"); d=2; }
    srand((unsigned)time(NULL));
    int lo=ranges[d-1][0], hi=ranges[d-1][1];
    int player = lo + rand()%(hi-lo+1);
    int cpu    = lo + rand()%(hi-lo+1);
    printf("Your number is: %d.\nComputer's is: %d.\n", player, cpu);
}

static void calc(){
    char expr[1024];
    getInputStr("Please type a sum, e.g. '1+2*3': ", expr, sizeof(expr));
    double v=0.0;
    if(safe_eval(expr,&v)) printf("= %.15g\n", v);
    else printf("Error: could not evaluate.\n");
}

static void local_info(){
    const char *fname = "local_system_information.txt";
    FILE *f = fopen(fname,"w");
    if(!f){ puts("Could not open output file."); return; }
#if OS_WIN
    const char *cmds[] = { "whoami", "tasklist", "netstat -ano" };
#else
    const char *cmds[] = { "w -i -p", "who -a", "service --status-all", "netstat -tuln" };
#endif
    for(size_t i=0;i<sizeof(cmds)/sizeof(cmds[0]);++i){
        char *out = cmd_capture(cmds[i]);
        if(out){ fputs(out,f); free(out); }
    }
    fclose(f);
    printf("System info written to %s\n", fname);
}

static void osi(){
    puts(
"6) Application: DNS, HTTP/HTTPS, Email, FTP\n"
"5) Presentation: Data representation (HTML,DOC,JPEG,MP3)\n"
"4) Session: Inter host communication (TCP,SIP,RTP)\n"
"3) Transport: End-to-End (TCP,UDP,TLS)\n"
"2) Network: IP, ICMP, OSPF\n"
"1) Data Link: Ethernet, 802.11, ARP\n"
"0) Physical: Binary transmission (RJ45, DSL, Wi-Fi)\n");
}

static void ohd(){
#if OS_WIN
    puts("ASCII table not available via 'man' on Windows.");
#else
    cmd_run("man ascii");
#endif
}

static void wdh(){
    char domain[512]; getInputStr("Domain name please (e.g google.com):\n$: ", domain, sizeof(domain));
    char save[32];     getInputStr("Save to disk? (y/n): ", save, sizeof(save));
    char fname[512];   getInputStr("Please pick a filename: ", fname, sizeof(fname));

    int to_disk = (save[0]=='y'||save[0]=='Y');
#if OS_WIN
    const char *whois_cmd_fmt = "whois %s";
    const char *dig_cmd_fmt   = "nslookup %s";
    const char *host_cmd_fmt  = "nslookup %s";
#else
    const char *whois_cmd_fmt = "whois %s";
    const char *dig_cmd_fmt   = "dig %s";
    const char *host_cmd_fmt  = "host %s";
#endif
    char cmd1[1024], cmd2[1024], cmd3[1024];
    snprintf(cmd1,sizeof(cmd1),whois_cmd_fmt,domain);
    snprintf(cmd2,sizeof(cmd2),dig_cmd_fmt,domain);
    snprintf(cmd3,sizeof(cmd3),host_cmd_fmt,domain);

    if(to_disk){
        FILE *f = fopen(fname,"w"); if(!f){ puts("Could not open file."); return; }
        char *o1=cmd_capture(cmd1); if(o1){ fputs(o1,f); free(o1); }
        char *o2=cmd_capture(cmd2); if(o2){ fputs(o2,f); free(o2); }
        char *o3=cmd_capture(cmd3); if(o3){ fputs(o3,f); free(o3); }
        fclose(f);
        printf("Results saved to %s\n", fname);
    }else{
        char *o1=cmd_capture(cmd1); if(o1){ printf("%s",o1); free(o1); }
        char *o2=cmd_capture(cmd2); if(o2){ printf("%s",o2); free(o2); }
        char *o3=cmd_capture(cmd3); if(o3){ printf("%s",o3); free(o3); }
    }
}

static void pchk(){
    int port = getInputInt("Port Please: ");
#if OS_WIN
    char *out = cmd_capture("netstat -ano");
#else
    char *out = cmd_capture("netstat -pnltu");
#endif
    if(!out){ return; }
    char needle[64]; snprintf(needle,sizeof(needle),":%d",port);
    char *line = strtok(out, "\n");
    while(line){
        if(strstr(line, needle)) printf("%s\n", line);
        line = strtok(NULL, "\n");
    }
    free(out);
}

// -------- Command loop --------
static const char* help_list[] = {
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
    "exit: exit the console."
};

static void print_help(){
    for(size_t i=0;i<sizeof(help_list)/sizeof(help_list[0]);++i) puts(help_list[i]);
}

int main(void){
    char cmd[128];
    while(1){
        printf("Hi, welcome to the console. Type 'help' for options.\n");
        getInputStr("> ", cmd, sizeof(cmd));
        if(strcmp(cmd,"exit")==0) break;
        else if(strcmp(cmd,"help")==0) print_help();
        else if(strcmp(cmd,"mdir")==0) mdir();
        else if(strcmp(cmd,"read")==0) read_file();
        else if(strcmp(cmd,"write")==0) write_file();
        else if(strcmp(cmd,"append")==0) append_file();
        else if(strcmp(cmd,"sfile")==0) sfile();
        else if(strcmp(cmd,"mkpasswd")==0) mkpasswd();
        else if(strcmp(cmd,"guess")==0) guess();
        else if(strcmp(cmd,"calc")==0) calc();
        else if(strcmp(cmd,"local")==0) local_info();
        else if(strcmp(cmd,"osi")==0) osi();
        else if(strcmp(cmd,"ohd")==0) ohd();
        else if(strcmp(cmd,"wdh")==0) wdh();
        else if(strcmp(cmd,"pchk")==0) pchk();
        else puts("Unknown command.");
        // keep loop running
    }
    return 0;
}
/*
Compilation notes:
  Linux/macOS:  gcc tool.c -o tool -lm
  Windows (MinGW):  gcc tool.c -o tool
    - External utilities (whois, dig, host, netstat, man) may not exist on Windows.
*/
