#include <iostream>
#include <ctime>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    //create copies of stdin/stdout, dup()
    vector<pid_t> bg_pids;
    int nstdin = dup(0); //Put in file 3
    int nstdout = dup(1); //Put in file 4

    char prevdir[4096];
    char dir[4096];

    for (;;) {
        //Implement iteration over vector of bg pid (vector also declared outside loop)
        // waitpid() - using flag for nonblocking


        // Implement username with getlogin()
        // Implement date/time with TODO
        // Implement curdir with getcwd()
        // need date/time, username, and absolute path to current dir
        //cout << YELLOW << "Shell" << NC << " ";
        time_t t;
        time(&t);
        string time = ctime(&t);
        string ftime = time.substr(4, 15);
        
        char* username = getenv("USER");

        getcwd(dir, sizeof(dir));

        for (unsigned int i = 0; i < bg_pids.size(); i++) {
            if (WIFSIGNALED(waitpid(bg_pids[i], 0, WNOHANG))) {
                cout << RED << "Process has exited" << NC << endl;
                bg_pids.erase(bg_pids.begin() + i);
                i--;
            }
        }

        cout << YELLOW << ftime << " " << username << ":" << dir << "$" << NC << " ";

        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }


        // chdir()
        // if dir (cd <dir>) is "-", then go to previous working directory
        // variable storing previous working directory (it needs to be declared outside loop)


        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        if (tknr.commands[0]->args[0] == "cd") {
            if (tknr.commands[0]->args[1] == "-") {
                chdir(prevdir);
                getcwd(prevdir, sizeof(prevdir));
            } else {
                getcwd(prevdir, sizeof(prevdir));
                chdir(tknr.commands[0]->args[1].c_str());
            }
            continue;
        }

        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }


        // for piping
        // for cmd : commands
        //     call pipe() to make pipe
        //     fork() - in child, redirect stdout; in parent, redirect stdin
        // add checks for first/last command

        // fork to create child
    
        int p[2];

        for (long unsigned i = 0; i < tknr.commands.size(); i++) {
        
            pipe(p);

            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }

            // Add check for bg process - add pid to vector if bg and don't waitpid() in parent

            if (pid == 0) {  // if child, exec to run command
                // implement multiple arguments, -iterate over args of current command to make char* array
                //char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};

                char** args = new char*[tknr.commands[i]->args.size() + 1];
                for (size_t j = 0; j < tknr.commands[i]->args.size(); j++) {
                    args[j] = (char*) (tknr.commands[i]->args[j]).c_str();
                }
                args[tknr.commands[i]->args.size()] = nullptr;

                // if current command is redirected, then open file and dup2 stdin/stdout that's being redirected
                // implement it safely for both at the same time

                if (i < tknr.commands.size()-1) {
                    dup2(p[1], 1);
                    close(p[0]);
                }

                if (tknr.commands[i]->hasInput()) {
                    int fd = open(tknr.commands[i]->in_file.c_str(), O_RDONLY);
                    dup2(fd, 0);
                }

                if (tknr.commands[i]->hasOutput()) {
                    int fd = open(tknr.commands[i]->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    dup2(fd, 1);
                }

                if (execvp(args[0], args) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else {  // if parent, wait for child to finish
                dup2(p[0], 0);
                close(p[1]);

                if (tknr.commands[i]->isBackground()) {
                    bg_pids.push_back(pid);
                } else {
                    if (i == tknr.commands.size()-1) {
                        int status = 0;
                        waitpid(pid, &status, 0);
                        if (status > 1) {  // exit if child didn't exec properly
                            exit(status);
                        }
                    }
                }
            }

            //Restore stdin/stdout (variable would be outside the loop)
        }
        dup2(nstdin, 0);
        dup2(nstdout, 1);
    }
}