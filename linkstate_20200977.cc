#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

//output file
FILE* outputFile = fopen("output_ls.txt", "w");

//global variables
char msg[1100];
int dist[110];
int parent[110];

//finding minimum distance function
int min_dist(bool spt[], int V) {
    int min = INT_MAX, min_index;

    for (int v = 0; v < V; v++)
        if (!spt[v] && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}

//dijkstra algorithm function
void dijkstra(int** graph, int start, int V, int print) {
    bool spt[V]; //spt

    //initialization
    for (int i = 0; i < V; i++) {
        spt[i] = false;
        dist[i] = INT_MAX;
        parent[i] = -1;
    }
    dist[start] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = min_dist(spt, V);
        spt[u] = true;

        //update
        for (int v = 0; v < V; v++) {
            if (!spt[v] && graph[u][v] != -1 && dist[u] != INT_MAX) {
                if (dist[u] + graph[u][v] < dist[v]) {
                    parent[v] = u;
                    dist[v] = dist[u] + graph[u][v];
                }
                else if (dist[u] + graph[u][v] == dist[v] && u < parent[v]) {
                    parent[v] = u;
                }
            }
        }
    }

    //print result
    if (print == 1) {
        for (int i = 0; i < V; i++) {
            if (i != start) {
                int next_vertex = i;
                while (parent[next_vertex] != start && parent[next_vertex] != -1) {
                    next_vertex = parent[next_vertex];
                }
                if (dist[i] != INT_MAX) {
                    fprintf(outputFile, "%d %d %d\n", i, next_vertex, dist[i]);
                }
            }
            else {
                fprintf(outputFile, "%d %d %d\n", i, i, 0);
            }
        }
        fprintf(outputFile, "\n");
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
		printf("usage: linkstate topologyfile messagesfile changesfile\n");
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
	if (topology == NULL || messages==NULL || changes==NULL) {
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
            dijkstra(graph, i, v, 1);
        }

        //message delivery
        while (fscanf(messages, "%d %d", &src, &dest) == 2) {
            if (fgets(msg, sizeof(msg), messages) != NULL) {
                size_t len = strlen(msg);
                if (len > 0 && msg[len - 1] == '\n') {
                    msg[len - 1] = '\0';
                }
                dijkstra(graph, src, v, 0);
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

    printf("Complete. Output file written to output_ls.txt.\n");

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