#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <sys/wait.h>
#include <vector>

int main(){

    // bring in the data from the text file
    int height, array_size;
    cin >> height;
    cin >> array_size;

    // create the array or vector in this case
    vector<int> input(array_size);

    // fill the vector with the data from the text file
    for(int i = 0; i < array_size; i++){
        cin >> input[i];
    }

    int number_of leaf_processes = 1 << (height - 1);
    int padding = array_size;

    while(padding % number_of_leaf_processes != 0){
        padding++;
    }

    input.resize(padding, 0); // resize by padding adding 0's in these new cells
    int chunk_size = padding / number_of_leaf_processes;

    // call function to create the pipes
    return 0;
}