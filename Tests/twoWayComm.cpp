#include <iostream>
#include <unistd.h>
#include <cstring>

int main() {
    int pipe_to_child[2];  // parent -> child
    int pipe_to_parent[2]; // child -> parent

    pipe(pipe_to_child);
    pipe(pipe_to_parent);

    pid_t pipe_id = fork();

    if (pipe_id == 0) {
        // Child
        close(pipe_to_child[1]); // Close write end
        close(pipe_to_parent[0]); // Close read end

        char message_from_parent[100];
        read(pipe_to_child[0], message_from_parent, sizeof(message_from_parent));  // FIXED

        std::cout << "Child received: " << message_from_parent << std::endl;

        const char *reply = "Child is speaking!";
        write(pipe_to_parent[1], reply, strlen(reply) + 1);

        close(pipe_to_child[0]);
        close(pipe_to_parent[1]);
    } else {
        // Parent
        close(pipe_to_child[0]); // Close read end
        close(pipe_to_parent[1]); // Close write end

        const char *message = "Howdy child!";
        write(pipe_to_child[1], message, strlen(message) + 1);

        char reply[100];
        read(pipe_to_parent[0], reply, sizeof(reply));  // FIXED
        std::cout << "Parent received: " << reply << std::endl;

        close(pipe_to_child[1]);
        close(pipe_to_parent[0]);
    }

    return 0;
}
