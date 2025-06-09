// Daniel Teberian

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <limits.h>
#include <signal.h>

using namespace std;

volatile int sigint_counter = 0;
vector<string> history;
const int HIST_MAX = 10;
int cmd_num = 1;

void sig_handler(int signal) {
    if (signal == SIGINT) {
        sigint_counter++;
    }
}

void parse_args(string line, vector<string> &cmds) {
    stringstream liness(line);
    string token;
    while (getline(liness, token, ' ')) {
        cmds.push_back(token);
    }
}

void parse_cin_to_vec(const string& ln, vector<string>& tkns) {
    tkns.clear();
    stringstream strstream(ln);
    string tkn;
    while (strstream >> tkn) {
        tkns.push_back(tkn);
    }
}

char** conv_vec_to_chararray(const vector<string>& tkns) {
    if (tkns.empty()) {
        return NULL;
    }

    char** argv = static_cast<char**>(malloc((tkns.size() + 1) * sizeof(char*)));

    if (argv == NULL) {
        perror("\033[1;31m[ERR] MALLOC FAILED FOR ARGV\033[0m");
        return NULL;
    }

    for (size_t i = 0; i < tkns.size(); ++i) {
        argv[i] = strdup(tkns[i].c_str());
        if (argv[i] == NULL) {
            perror("\033[1;31m[ERR] STRDUP FAILED FOR ARGV\033[0m");
            for (size_t j = 0; j < i; ++j) {
                free(argv[j]);
            }
            free(argv);
            return NULL;
        }
    }

    argv[tkns.size()] = NULL;
    return argv;
}

void dealloc_char_array(char** argv) {
    if (argv == NULL) return;
    for (int i = 0; argv[i] != NULL; ++i) {
        free(argv[i]);
    }
    free(argv);
}

void hist_add(const string& cmd) {
    if (cmd == "r" || cmd.rfind("r ", 0) == 0 || cmd == "exit") return;
    history.push_back(cmd);
    if (history.size() > HIST_MAX) {
        history.erase(history.begin());
    }
}

void hist_display() {
    cout << "\033[0;37mHISTORY:\033[0m\n";
    for (size_t i = 0; i < history.size(); ++i) {
        cout << "\033[0;37m " << (i + 1) << ": " << history[i] << "\033[0m\n";
    }
}

void exec_ext_cmd(const vector<string>& args) {
    if (args.empty()) return;

    char** argv = conv_vec_to_chararray(args);
    if (argv == NULL) return;

    pid_t pid = fork();

    if (pid == -1) {
        perror("\033[1;31m[ERR] FORKING FAILED\033[0m");
        dealloc_char_array(argv);
        return;
    } else if (pid == 0) {
        execvp(argv[0], argv);
        cout << "\033[1;31m[ERR] " << args[0] << " The program seems missing. Error code is: " << errno << "\033[0m" << endl;
        _exit(127);
    } else {
        int status;
        pid_t term_pid = waitpid(pid, &status, 0);

        if (term_pid == -1) {
            perror("\033[1;31m[ERR] WAITPID FAILED\033[0m");
        } else {
            if (WIFEXITED(status)) {
                int x_code = WEXITSTATUS(status);
                if (x_code != 0) {
                    cout << "\033[0;37m[ Program returned exit code " << x_code << "]\033[0m\n";
                }
            } else if (WIFSIGNALED(status)) {
                cout << "\033[0;37m[ Program terminated by signal " << WTERMSIG(status) << "]\033[0m\n";
            }
        }
        dealloc_char_array(argv);
    }
}

// A secret, cool function
void covenant(const string& msg)
{
	const string colors[] =
	{
		// Red
		"\033[1;31m",
		// Yellow
		"\033[1;33m",
		// Green
		"\033[1;32m",
		// Cyan
		"\033[1;36m",
		// Blue
		"\033[1;34m",
		// Magenta
		"\033[1;35m"
		
		// Any ideas as to what this does?
	};
	
	size_t num_colors = sizeof(colors) / sizeof(colors[0]);
	
	for (size_t i = 0; i < msg.length(); ++i)
	{
		cout << colors[i % num_colors] << msg[i];
	}

	// Back to normal
	cout << "\033[0m" << endl;
}


int main(void) {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("\033[1;31m[ERR] COULD NOT REGISTER SIGINT HANDLER\033[0m");
        return EXIT_FAILURE;
    }

    while (1) {
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            cout << "\033[1;32mGUISH (cmd #" << cmd_num << ") " << cwd << " $ \033[0m";
        } else {
            perror("\033[1;31m[ERR] FAILED GETCWD\033[0m");
            cout << "\033[1;32mguish (cmd #" << cmd_num << ") $ \033[0m";
        }

        cout.flush();

        string cmd_ln;
        if (!getline(cin, cmd_ln)) {
            if (cin.eof()) {
                cout << "\033[1;34m\n[INFO] SHELL EXITING.\033[0m\n";
            } else {
                cerr << "\033[1;31m[ERR] COULD NOT READ INPUT\033[0m\n";
            }

            cout << "\033[0;37mTotal SIGINT signals caught: " << sigint_counter << "\033[0m\n";
            break;
        }

        if (cmd_ln.empty()) continue;

        vector<string> cmd_args;
        parse_args(cmd_ln, cmd_args);

        string cmd_name = cmd_args[0];

        if (cmd_name == "help") {
            cout << "\033[0;37mGUISH - HELP MENU\033[0m\n";
            cout << "\033[0;37mexit - Exits the shell.\033[0m\n";
            cout << "\033[0;37mhelp - Displays this menu, explaining each command that is currently supported by guish.\033[0m\n";
            cout << "\033[0;37mhist - Displays the 10 most recent commands. NOTE: The hist command will, itself, be added to the history.\033[0m\n";
            cout << "\033[0;37mr <cmd> - Execute a command that is listed in the history. The command requires that the user specifies that number assigned to the command in the history.\033[0m\n";
            continue;
        } else if (cmd_name == "exit") {
            cout << "\033[0;37mTotal SIGINT signals caught: " << sigint_counter << "\033[0m\n";
            exit(0);
        } else if (cmd_name == "hist") {
            hist_add(cmd_ln);
            hist_display();
            cmd_num++;
            continue;
		}
		else if (cmd_name == "noah")
		{
			hist_add(cmd_ln);
			covenant("NEVER AGAIN! :D");
			cmd_num++;
			continue;
        } else if (cmd_name == "r") {
            string hist_exec_cmd;
            int hist_num = -1;

            if (cmd_args.size() > 1) {
                try {
                    hist_num = stoi(cmd_args[1]);
                } catch (const invalid_argument&) {
                    cout << "\033[1;31m[ERR] INVALID NUMBER FORMAT.\nCorrect usage: r <number>\033[0m\n";
                    cmd_num++;
                    continue;
                } catch (const out_of_range&) {
                    cout << "\033[1;31m[ERR] NUMBER OUT OF RANGE.\nCorrect usage: r <number>\033[0m\n";
                    cmd_num++;
                    continue;
                }
            }

            if (hist_num == -1) {
                if (history.empty()) {
                    cout << "\033[1;31m[ERR] NO HISTORY AVAILABLE.\033[0m\n";
                    cmd_num++;
                    continue;
                }
                hist_exec_cmd = history.back();
            } else {
                if (hist_num < 1 || hist_num > static_cast<int>(history.size())) {
                    cout << "\033[1;31m[ERR] HISTORY NUMBER " << hist_num << " OUT OF RANGE\nCorrect usage: r <number>\033[0m\n";
                    cmd_num++;
                    continue;
                }
                hist_exec_cmd = history[hist_num - 1];
            }

            cout << "\033[0;37mRe-executing: " << hist_exec_cmd << "\033[0m\n";
            hist_add(hist_exec_cmd);
            vector<string> hist_cmd_args;
            parse_args(hist_exec_cmd, hist_cmd_args);

            if (!hist_cmd_args.empty()) {
                if (hist_cmd_args[0] == "exit") {
                    cout << "\033[0;37mTotal SIGINT signals caught: " << sigint_counter << "\033[0m\n";
                    exit(0);
                } else if (hist_cmd_args[0] == "hist") {
                    hist_display();
                } else {
                    exec_ext_cmd(hist_cmd_args);
                }
            }

            cmd_num++;
            continue;
        }

        hist_add(cmd_ln);
        exec_ext_cmd(cmd_args);
        cmd_num++;
    }

    return 0;
}
