// Daniel Teberian

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>

// Additional imports
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <limits.h>
#include <signal.h>

using namespace std;


// A counter for SIGINTs that are caught, set to 0 by default
volatile int sigint_counter = 0;

// Establish the history list
vector<string> history;
// The maximum length of the history list
const int HIST_MAX = 10;

// The number for the command (it is 1, since it is the most recent)
int cmd_num = 1;

// A handler for SIGINTs
void sig_handler(int signal)
{
	// If the signal is SIGINT
	if (signal == SIGINT)
	{
		// Increment the value of sigint_counter by 1
		sigint_counter++;
	}
}



void parse_args(string line, vector<string> &cmds){
    stringstream liness(line);
 
    string token;
    while (getline(liness, token, ' ')) {
        cmds.push_back(token);
    }
}


// Parses input into a vector of string tokens
void parse_cin_to_vec(const string& ln, vector<string>& tkns)
{
	// Clear any existing tokens
	tkns.clear();

	// Create a stringstream!
	stringstream strstream(ln);
	string tkn;

	// Extract tokens, separated by whitespace
	while (strstream >> tkn)
	{
		tkns.push_back(tkn);
	}
}


// Converts vector of C++ strings into an array of char pointers. This is done in order to meet the requirements for using execvp().
char** conv_vec_to_chararray(const vector<string>& tkns)
{
	// If there are no tokens
	if (tkns.empty())
	{
		// Don't do anything.
		return NULL;
	}

	// Allocate the appropriate amount of memory, given the array of char pointers.
	// NOTE: The size is increased by one in order to make room for the NULL terminator.
	char** argv = static_cast<char**>(malloc((tkns.size() + 1) * sizeof(char*)));

	if (argv == NULL)
	{
		perror("[ERR] MALLOC FAILED FOR ARGV");
		// Return NULL
		return NULL;
	}

	// Copy each string into a C-string
	for (size_t i = 0; i < tkns.size(); ++i)
	{
		// Use strdup in order to allocate memory and copy string
		argv[i] = strdup(tkns[i].c_str());

		// If argv has no value
		if (argv[i] == NULL)
		{
			perror("[ERR] STRDUP FAILED FOR ARGV");

			// Clean up any allocated memory
			for (size_t j = 0; j < i; ++j)
			{
				free(argv[j]);
			}

			free(argv);
			return NULL;
		}
	}

	// The array gets the ol' null terminator treatment
	argv[tkns.size()] = NULL;

	// Return argv
	return argv;
}


// Free any memory that was allocated by conv_vec_to_chararray
void dealloc_char_array(char** argv)
{
	// If no argument is specified
	if (argv == NULL)
	{
		return;
	}

	for (int i = 0; argv[i] != NULL; ++i)
	{
		free(argv[i]);
	}

	// Free the array of any pointers
	free(argv);
}

// Add an entry to the history list
void hist_add(const string& cmd)
{
	// Exclude "r" and "exit" from the history
	if (cmd == "r" || cmd.rfind("r ", 0) == 0 || cmd == "exit")
	{
		// Do nothing
		return;
	}

	hist_ls.push_back(cmd);

	if (hist_ls.size() > HIST_MAX)
	{
		hist_ls.erase(hist_ls.begin());
	}
}


// Display history ("hist")
void hist_display()
{
	// Print message
	cout << "HISTORY:\n";

	for (size_t i = 0; i < hist_ls.size(); ++i)
	{
		cout << " " << (i + 1) << ": " << hist_ls[i] << "\n";
	}
}


// Logic for executing commands
void exec_ext_cmd(const vector<string>& args)
{
	// If no arguments are specified
	if (args.empty())
	{
		// Return
		return;
	}

	// Convert to char**, in order for execvp to work
	char** argv = conv_vec_to_chararray(args);
	if (argv == NULL)
	{
		// Error message should be printed by the function, anyway.
		return;
	}

	pid_t pid = fork();

	// If the forking fails
	if (pid == -1)
	{
		perror("[ERR] FORKING FAILED");
		dealloc_char_array(argv);
		return;
	}
	// Child process
	else if (pid == 0)
	{
		// Execute specified command
		execvp(argv[0], argv);

		// If execvp returns anything, something isn't working. :(

		cout << "[ERR] " << args[0] << " The program seems missing. Error code is: " << errno << endl;

		// _exit() is supposed to be better. Why not?
		// Exit with code 127, which corresponds to command not found
		_exit(127);
	}
	// Parent process
	else
	{
		int status;

		// Wait for the child process to finish executing
		pid_t term_pid = waitpid(pid, &status, 0);

		// Bad :(
		if (term_pid == -1)
		{
			perror("[ERR] WAITPID FAILED");
		}
		else
		{
			if (WIFEXITED(status))
			{
				int x_code = WEXITSTATUS(status);

				// If there is a non-zero exit code
				if (x_code != 0)
				{
					cout << "[ Program returned exit code " << x_code << "]\n";
				}
			}

		else if (WIFSIGNALED(status))
		{
			cout << "[ Program terminated by signal " << WTERMSIG(status) << "]\n";
		}

	// Deallocate memory from argv
	dealloc_char_array(argv);
}


int main(void)
{
	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		perror("[ERR] COULD NOT REGISTER SIGINT HANDLER");
		return EXIT_FAILURE;
	}

    while (1)
    {
	// Defined by limits.h, sets the maximum path length
	char cwd[PATH_MAX];

	if (getcwd(cwd, sizeof(cwd)) != nullptr)
	{
		cout << "GUISH (cmd #" << cmd_num << ") " << cwd " $ ";
	}
	else
	{
		perror("[ERR] FAILED GETCWD");
		cout << "guish (cmd #" << cmd_num << ") $ ";
	}

	cout.flush();

	string cmd_ln;

	if (!getline(cin, cmd_ln))
	{
		// Handles EOF and input errors
		if (cin.eof())
		{
			cout << "\n[INFO] SHELL EXITING.\n";
		}
		else
		{
			cerr << "[ERR] COULD NOT READ INPUT\n";
		}

		// Cleanup on exit, in the event of the input stream ending
		cout << "Total SIGINT signals caught: " << sigint_counter << endl;
		// No more shell
		break;
	}

	// Ignore any empty input
	if (cmd_ln.empty())
	{
		continue;
	}

	// Parse user input
	vector<string> cmd_args;
	parse_args(cmd_ln, cmd_args);

	string cmd_name = cmd_args[0];

        // prompt
        cout << "SHELL$: ";

        string cmd;
        getline(cin, cmd);

        // ignore empty input
        if (cmd == "" || cmd.size() == 0) 
            continue;

        cout <<"Received user commands: ";
        cout << cmd << endl;
        
        // built-in: exit
        if (cmd == "help"){
            cout << "GUISH - HELP MENU\n";
		cout << "exit - Exits the shell.\n";
		cout << "help - Displays this menu, explaining each command that is currently supported by guish.\n";
		cout << "hist - Displays the 10 most recent commands. NOTE: The hist command will, itself, be added to the history.\n";
		cout << "r <cmd> - Execute a command that is listed in the history. The command requires that the user specifies that number assigned to the command in the history.\n";
            continue;
        }else if(cmd == "exit"){
		cout << "Total SIGINT signals caught: " << sig_counter << endl;
            exit(0);
        }
	else if (cmd_name == "hist")
	{
		hist_add(cmd_ln);
		hist_show();
		cmd_num++;
		continue;
	}
	else if (cmd_name == "r")
	{
		string hist_exec_cmd;
		int hist_num = -1;

		if (cmd_args.size() > 1)
		{
			try
			{
				hist_num = stoi(cmd_args[1]);

        vector<string> cmd_args;
        parse_args(cmd, cmd_args);

        // fork child and execute program
        int pid = fork();
        int status;
        if (pid == 0)
        {
            char * argv[4] = {"ls", "-l", "-a", NULL};
            execvp(argv[0], argv);
        }
        else
        {
            // wait for program to finish and print exit status
            waitpid(pid, &status, 0);
        }
    }
}
