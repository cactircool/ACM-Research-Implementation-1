#include <iostream>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <errno.h>
#include <unordered_map>
#include <vector>
#include <bitset>

typedef std::unordered_map<std::string, std::vector<bool>> Secrets;

class ProgramDebugger {
private:
    pid_t target_pid;
    std::string program_name;
    Secrets secrets;

   
    // Get current register values
    struct user_regs_struct getRegisters() {
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, target_pid, nullptr, &regs) == -1) {
            std::cerr << "Error getting registers: " << strerror(errno) << std::endl;
            exit(1);
        }
        return regs;
    }
   
    // Print register state
    void printRegisterState() {
        struct user_regs_struct regs = getRegisters();

        for (auto &[secretName, secret] : secrets) {
            auto compareBits = [&](const std::vector<bool> &bits, auto val) {
                if (bits.size() < sizeof(val)) return false;
                for (int i = 0; i < sizeof(val); ++i) 
                    if ((val >> (i + 1) & 1) != bits[i])
                        return false;

                std::cerr << "\033[31mP/L-alert: Secret found in `" << secretName << "`!\033[0m" << std::endl;
                return true;
            };

            compareBits(secret, regs.cs);
            compareBits(secret, regs.ds);
            compareBits(secret, regs.eflags);
            compareBits(secret, regs.es);
            compareBits(secret, regs.fs);
            compareBits(secret, regs.fs_base);
            compareBits(secret, regs.gs);
            compareBits(secret, regs.gs_base);
            compareBits(secret, regs.orig_rax);
            compareBits(secret, regs.r10);
            compareBits(secret, regs.r11);
            compareBits(secret, regs.r12);
            compareBits(secret, regs.r12);
            compareBits(secret, regs.r14);
            compareBits(secret, regs.r15);
            compareBits(secret, regs.r8);
            compareBits(secret, regs.r9);
            compareBits(secret, regs.rax);
            compareBits(secret, regs.rbp);
            compareBits(secret, regs.rbx);
            compareBits(secret, regs.rcx);
            compareBits(secret, regs.rdi);
            compareBits(secret, regs.rdx);
            compareBits(secret, regs.rip);
            compareBits(secret, regs.rsi);
            compareBits(secret, regs.rsp);
            compareBits(secret, regs.ss);
        }
    }

public:
    ProgramDebugger(const std::string& program, const Secrets &secrets) : program_name(program), secrets(secrets) {}
   
    void start() {
        target_pid = fork();
       
        if (target_pid == -1) {
            std::cerr << "Fork failed" << std::endl;
            return;
        }
       
        if (target_pid == 0) {
            // Child process
            if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
                std::cerr << "Error in PTRACE_TRACEME" << std::endl;
                exit(1);
            }
           
            execl(program_name.c_str(), program_name.c_str(), nullptr);
            std::cerr << "Error executing program `" << program_name << "`: " << strerror(errno) << std::endl;
            exit(1);
        } else {
            // Parent process (debugger)
            int status;
            waitpid(target_pid, &status, 0);
           
            if (WIFSTOPPED(status)) {
                std::cout << "Program started and ready for debugging" << std::endl;
                debug();
            }
        }
    }
   
    void debug() {
        int status;
        bool running = true;
       
        while (running) {
            // Print current register state
            printRegisterState();

            // Single step
            if (ptrace(PTRACE_SINGLESTEP, target_pid, nullptr, nullptr) == -1) {
                std::cerr << "Error in single step" << std::endl;
                running = false;
                break;
            }
            waitpid(target_pid, &status, 0);
            
            if (WIFEXITED(status)) {
                std::cout << "Program finished executing" << std::endl;
                running = false;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [DEBUG_PROGRAM] (secretName secretValue)*" << std::endl;
        return 1;
    }

    Secrets secrets;
    for (int i = 2; i < argc; i += 2) {
        std::string s = argv[i + 1];
        std::vector<bool> result;
        result.reserve(s.size() * 8);
        for (char ch : s) {
            std::bitset<8> bits(ch);
            for (int i = 7; i >= 0; --i)
                result.push_back(bits[i]);
        }
        secrets[argv[i]] = result;
    }
   
    ProgramDebugger debugger(argv[1], {});
    debugger.start();
   
    return 0;
}