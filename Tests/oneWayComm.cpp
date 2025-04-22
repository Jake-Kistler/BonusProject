// A very C like pipe test
// this one has the child communicate with the parent

#include <iostream>
#include <unistd.h>
#include <cstring> // old school with this one 

int main(){
    int pipe_file_des[2];
    pipe(pipe_file_des); // create the pipe

    pid_t pipe_id = fork(); // fork

    if(pipe_id == 0){
        // child processes
        close(pipe_file_des[0]); // no more reading
        const char *message = "This is the child speaking!";

        write(pipe_file_des[1], message, strlen(message) + 1);
        close(pipe_file_des[1]);
    }
    else{
        // working with the parent now
        close(pipe_file_des[1]); // no more writeing
        char buffer[100];

        read(pipe_file_des[0], buffer, sizeof(buffer));
        std::cout << "Parent got back: " << buffer << std::endl;
        close(pipe_file_des[0]);
    }

    return 0;
}