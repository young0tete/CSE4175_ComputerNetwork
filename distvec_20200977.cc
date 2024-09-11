#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

//output file
FILE* outputFile = fopen("output_dv.txt", "w");

//global variables
char msg[1100];
int dist[110];
int parent[110];

//finding minimum distance function
void bellman_ford(int** graph, int start, int V) {
    //initialization
    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        parent[i] = -1;
    }
    dist[start] = 0;

    //relax edges |V| - 1 times
    for (int i = 1; i <= V - 1; i++) {
        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {
                if (graph[u][v] != -1 && dist[u] != INT_MAX && dist[u] + graph[u][v] <= dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                    parent[v] = u;
                }
            }
        }
    }
}

//routing path printing function
void routing_path(int parent[], int j, int destination) {
    if (parent[j] == -1) {
        fprintf(outputFile, "%d ", j);
        return;
    }

    routing_path(parent, parent[j], destination);
    if (j != destination) {
        fprintf(outputFile, "%d ", j);
    }
}

int main(int argc, char* argv[]) {
    int i, j, v, a, b, c;
    int src, dest;

    //argument input error
    if (argc != 4) {
        printf("usage: distvec topologyfile messagesfile changesfile\n");
        exit(1);
    }

    //read input files
    FILE* topology;
    FILE* messages;
    FILE* changes;
    topology = fopen(argv[1], "rb");
    messages = fopen(argv[2], "rb");
    changes = fopen(argv[3], "rb");

    //file open error
    if (topology == NULL || messages == NULL || changes == NULL) {
        printf("Error: open input file.\n");
        exit(1);
    }

    //initial graph
    fscanf(topology, "%d", &v);
    int** graph = (int**)malloc(v * sizeof(int*));
    for (i = 0; i < v; i++) {
        graph[i] = (int*)malloc(v * sizeof(int));
    }
    for (i = 0; i < v; i++) {
        for (j = 0; j < v; j++) {
            graph[i][j] = -1;
        }
    }

    while (fscanf(topology, "%d %d %d", &a, &b, &c) == 3) {
        graph[a][b] = c;
        graph[b][a] = c;
    }

    while (1) {
        //print routing table
        for (i = 0; i < v; i++) {
            bellman_ford(graph, i, v);
            for (int j = 0; j < v; j++) {
                if (j != i) {
                    int next_vertex = j;
                    while (parent[next_vertex] != i && parent[next_vertex] != -1) {
                        next_vertex = parent[next_vertex];
                    }
                    if (dist[j] != INT_MAX) {
                        fprintf(outputFile, "%d %d %d\n", j, next_vertex, dist[j]);
                    }
                }
                else {
                    fprintf(outputFile, "%d %d %d\n", j, j, 0);
                }
            }
            fprintf(outputFile, "\n");
        }

        //message delivery
        while (fscanf(messages, "%d %d", &src, &dest) == 2) {
            if (fgets(msg, sizeof(msg), messages) != NULL) {
                size_t len = strlen(msg);
                if (len > 0 && msg[len - 1] == '\n') {
                    msg[len - 1] = '\0';
                }
                bellman_ford(graph, src, v);
                if (dist[dest] == INT_MAX) {
                    fprintf(outputFile, "from %d to %d cost infinite hops unreachable message%s", src, dest, msg);
                }
                else {
                    fprintf(outputFile, "from %d to %d cost %d hops ", src, dest, dist[dest]);
                    routing_path(parent, dest, dest);
                    fprintf(outputFile, "message%s", msg);
                }
                fprintf(outputFile, "\n");
            }
        }
        fprintf(outputFile, "\n");
        fseek(messages, 0, SEEK_SET);

        //changes
        int readparam = fscanf(changes, "%d %d %d", &a, &b, &c);
        if (readparam == EOF) {
            break;
        }
        if (c == -999) {
            graph[a][b] = -1;
            graph[b][a] = -1;
        }
        else {
            graph[a][b] = c;
            graph[b][a] = c;
        }
    }

    printf("Complete. Output file written to output_dv.txt.\n");

    //free
    for (int i = 0; i < v; i++) {
        free(graph[i]);
    }
    free(graph);

    //close files
    fclose(topology);
    fclose(messages);
    fclose(changes);
    fclose(outputFile);

    return 0;
}