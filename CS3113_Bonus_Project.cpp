#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <sys/wait.h>
#include <vector>

using namespace std;

// Struct to track level and position
struct NodeMetaData {
    int index;
    int level;
    int position;
    int start;
    int end;
};

NodeMetaData compute_node_meta_data(int index, int chunk_size, int total_leaves);
int create_process_tree(int index, int height, const vector<int>& input, int chunk_size, int total_leaves, int data_write_pipe = -1, int term_read_pipe = -1);

int main() {
    // read in some data
    int height, array_size;
    cin >> height;
    cin >> array_size;

    // create the array for input and populate it
    vector<int> input(array_size);
    for (int i = 0; i < array_size; i++) {
        cin >> input[i];
    }

    int number_of_leaf_processes = 1 << (height - 1);
    int padding = array_size;

    // find the amount of padding we need to add to the input array
    while (padding % number_of_leaf_processes != 0) {
        padding++;
    }

    // resize by the padding amount and fill these new cells with 0's
    input.resize(padding, 0);
    int chunk_size = padding / number_of_leaf_processes;

    int pipe_term[2];
    pipe(pipe_term);

    pid_t root_pid = fork();

    if (root_pid == 0) {
        close(pipe_term[1]);
        create_process_tree(0, height, input, chunk_size, number_of_leaf_processes, -1, pipe_term[0]);
    }

    // Main process
    close(pipe_term[0]);

    int dummy = 0;
    write(pipe_term[1], &dummy, sizeof(int));
    close(pipe_term[1]);

    waitpid(root_pid, nullptr, 0);

    return 0;
}

NodeMetaData compute_node_meta_data(int index, int chunk_size, int total_leaves) {

    // binary tree math 
    int level = (int)log2(index + 1) + 1;
    int position = index - ((1 << (level - 1)) - 1);
    int leaf_start = (1 << (int)log2(total_leaves)) - 1;

    int start = 0, end = -1;
    
    if (index >= leaf_start) {
        int j = index - leaf_start;
        start = j * chunk_size;
        end = (j + 1) * chunk_size - 1;
    }

    return {index, level, position, start, end};
}

int create_process_tree(int index, int height, const vector<int>& input, int chunk_size, int total_leaves, int data_write_pipe, int term_read_pipe) {
    NodeMetaData node_data = compute_node_meta_data(index, chunk_size, total_leaves);

    if (node_data.level == height) {
        // Leaf process
        int local_sum = 0;
        for (int i = node_data.start; i <= node_data.end; i++) {
            local_sum += input[i];
        }

        if (data_write_pipe != -1) {
            write(data_write_pipe, &local_sum, sizeof(int));
            close(data_write_pipe);
        }

        cout << "[PID " << getpid() << "] [Index " << node_data.index << "] [Level " << node_data.level
             << ", Position " << node_data.position << "] computed sum: " << local_sum << endl;

        // Wait for termination signal
        int dummy;
        read(term_read_pipe, &dummy, sizeof(int));
        close(term_read_pipe);

        cout << "[PID " << getpid() << "] [Index " << node_data.index << "] terminated." << endl;
        exit(0);
    }

    // Internal node
    int pipe_left_data[2], pipe_right_data[2];
    int pipe_left_term[2], pipe_right_term[2];

    pipe(pipe_left_data);
    pipe(pipe_right_data);
    pipe(pipe_left_term);
    pipe(pipe_right_term);

    pid_t left_child_pid = fork();
    if (left_child_pid == 0) {
        close(pipe_left_data[0]);
        close(pipe_left_term[1]);
        create_process_tree(2 * index + 1, height, input, chunk_size, total_leaves, pipe_left_data[1], pipe_left_term[0]);
    }

    pid_t right_child_pid = fork();
    if (right_child_pid == 0) {
        close(pipe_right_data[0]);
        close(pipe_right_term[1]);
        create_process_tree(2 * index + 2, height, input, chunk_size, total_leaves, pipe_right_data[1], pipe_right_term[0]);
    }

    // Parent process
    close(pipe_left_data[1]);
    close(pipe_right_data[1]);
    close(pipe_left_term[0]);
    close(pipe_right_term[0]);

    //  First: read results
    int left_sum = 0, right_sum = 0;
    read(pipe_left_data[0], &left_sum, sizeof(int));
    read(pipe_right_data[0], &right_sum, sizeof(int));
    close(pipe_left_data[0]);
    close(pipe_right_data[0]);

    int total_sum = left_sum + right_sum;

    //  Then: print after reading
    if (index == 0) {
        cout << "[PID " << getpid() << "] [Index " << node_data.index << "] [Level " << node_data.level
             << ", Position " << node_data.position << "] received: " << left_sum << ", " << right_sum
             << " → Final sum: " << total_sum << endl;
    } 
    else {
        cout << "[PID " << getpid() << "] [Index " << node_data.index << "] [Level " << node_data.level
             << ", Position " << node_data.position << "] received: " << left_sum << ", " << right_sum
             << " → sum: " << total_sum << endl;
    }

    //  Then: send termination signals
    int dummy = 0;
    write(pipe_left_term[1], &dummy, sizeof(int));
    write(pipe_right_term[1], &dummy, sizeof(int));
    close(pipe_left_term[1]);
    close(pipe_right_term[1]);

    //  Then: wait for children to terminate
    waitpid(left_child_pid, nullptr, 0);
    waitpid(right_child_pid, nullptr, 0);

    // If need to send sum up to parent
    if (data_write_pipe != -1) {
        write(data_write_pipe, &total_sum, sizeof(int));
        close(data_write_pipe);
    }

    // If need to wait for own termination signal (non-root nodes)
    if (term_read_pipe != -1) {
        int dummy2;
        read(term_read_pipe, &dummy2, sizeof(int));
        close(term_read_pipe);
    }

    cout << "[PID " << getpid() << "] [Index " << node_data.index << "] terminated." << endl;
    exit(0);
}
