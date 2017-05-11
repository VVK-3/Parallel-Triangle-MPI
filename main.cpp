#include "auxiliary.h"
#include "constants.h"
#include <mpi.h>

Node recv_node(MPI_Status &status);

int main() {

    int rank, world;
    double start, start2;
	MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    /* Split the input graph to parts (subgraphs)*/
    int *boundaries = new int[world];
    if (rank == 0) {
		start  = MPI_Wtime();
        cout << "Splitting graph (partitioning file):" << endl;
        boundaries = partition_file(FILENAME, DELIMETER, world);
        cout << "done!" << endl;
		start2  = MPI_Wtime();
        cout << "Process reading chunk..." << endl << endl;
    }
    MPI_Bcast(boundaries, world, MPI_INT, 0, MPI_COMM_WORLD);

    /* Each Processor reads it's chuck*/
    map<int, Node> nodes = parse_file(CHUNK_PREFIX + to_string(rank) + ".txt", DELIMETER);
    cout << "process " << rank << " starts computing..." << endl;


    //Core of the Paper's Algorithm (Figure 3)

    /**
    * For each node v in the Vertex set Vi
    * calculate the triangles
    */
    int local_triangles = 0;
    int completed = 0;
    int *send = nullptr;

    for (auto const &node : nodes) {

        int last_proc = -1; // reset last_proc

        // for each neighbors of the node
        for (int i = 0; i < node.second.neighbors.size(); ++i) {

            int n_id = node.second.neighbors[i];

            //if the neighbor exists in Vi the find the triangles
            if (nodes.find(n_id) != nodes.end()) {
                local_triangles += intersectionCount(node.second.neighbors, nodes[n_id].neighbors);
            }

                // if the neighbor does not exist in Vi, then send the node to the processor
                // who has the neighbor as a core node
            else {
                int proc = node_owner(boundaries, world, n_id);

                //send to processor that holds the node if not sent already
                if (rank != proc && last_proc != proc) {

                    //if there is a pending request wait for it to finish
                    //to avoid corrupting the buffer
                    if (request != MPI_REQUEST_NULL) {
                        MPI_Wait(&request, &status);
                        free(send);
                    }

                    // now send the node data
                    send = serializeNode(node.second);
                    int buffer_size = (int) (node.second.neighbors.size() + 1);
                    MPI_Isend(send, buffer_size, MPI_INT, proc, DATA_TAG, MPI_COMM_WORLD, &request);
                    last_proc = proc;
                }
            }

        }// _endfor each neighbors of the node

        //read all incoming data messages
        int flag = 1;
        while (flag) {
            MPI_Iprobe(MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                Node n = recv_node(status);
                local_triangles += surrogateCount(nodes, n);
            }
        }

    }// _endfor each node v in the Vertex set Vi


    /* broadcast message with notifier tag*/
    for (int i = 0; i < world; ++i) {
        //no reason to send data, just the NOTIFIER_TAG
        if (rank != i)
            MPI_Isend(NULL, 0, MPI_BYTE, i, NOTIFIER_TAG, MPI_COMM_WORLD, &request);
    }


    /* wait for all the other processors to finish,
    * as they may send data for surrogate count
    */

    int flag = 0;
    while (completed < world - 1) {

        //check for notifier message from any processor
        MPI_Iprobe(MPI_ANY_SOURCE, NOTIFIER_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            MPI_Recv(NULL, 0, MPI_BYTE, MPI_ANY_SOURCE, NOTIFIER_TAG, MPI_COMM_WORLD, &status);
            completed++;
        }

        //read all incoming data messages
        while (flag) {
            MPI_Iprobe(MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                Node n = recv_node(status);
                local_triangles += surrogateCount(nodes, n);
            }
        }
    }


    MPI_Barrier(MPI_COMM_WORLD);



    /*Reduce all of the local sums into the global sum*/
    int total_triangles;
    MPI_Reduce(&local_triangles, &total_triangles, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print the result
    if (rank == 0) {
        cout << endl << "finished ..." << endl;
        cout << endl << "TOTAL TRIANGLES=" << total_triangles << endl;
		cout << endl << "time taken to partition = " << MPI_Wtime() - start << endl;
		cout << endl << "time taken to count = " << MPI_Wtime() - start2 << endl;
    }

    MPI_Finalize();
    return 0;
}

Node recv_node(MPI_Status &status) {
    int count = 0;
    MPI_Get_count(&status, MPI_INT, &count);
    int *buffer = new int[count];
    MPI_Recv(buffer, count, MPI_INT, MPI_ANY_SOURCE, DATA_TAG, MPI_COMM_WORLD, &status);
    Node node = deserializeNode(buffer, count);
    free(buffer);
    return node;
}
