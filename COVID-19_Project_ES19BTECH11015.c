/*

 ============================================================================

 Name        : COVID-19_Project_ES19BTECH11015.c

 Author      : Krishn Vishwas Kher (ES19BTECH11015)

 Language    : C

 Description : Simulation of the SIR epidemic, COVID-19 using the event-driven algorithm.

 Style       : ANSI

 Note        : Since this code can potentially produce a large output, it may be a good idea to set a higher limit on the
 size of the data that can be printed to the console, especially in case of IDEs.

 ============================================================================

 */

#include <stdio.h>

#include <stdlib.h>

#include <stdbool.h>

#include <time.h>

#include <stdarg.h>

#include <string.h>

#include <limits.h>

/* Defining the constants that we will need in the program. */
const int MAX_VERTICES = 10000;

const int MAX_EDGES = 3000;

const int Tmax = 300;

short int vertex_count;

short int max_edge_count;

/* Defining an enum for each of the states an individual can be in. */
typedef enum node_status {

	Infected = -1, Susceptible = 0, Recovered = 1

} node_status;

/* A generic node to represent a person in the graph. */
typedef struct Graph_node {

	short int identity_no; //Stores the identity number

	/* Initially stores the time at which the person will get infected, & then, later the time at which it will recover. */
	short int pred_inf_time;

	/* Stores the status of the person. */
	node_status var;

	/* Informs if the corresponding node is already in line to be processed in the priority queue, so as to avoid invalid
	 * addition of it later.
	 */
	bool in_queue;

	/* Symbolises a link between two nodes. */
	struct Graph_node *edge_link;

} Gnode;

/* A typical struct upon which the priority queue operates on. */
typedef struct Event {

	short int Gnode_identity_no;

	char action_type; //Stores if the action is 'transmit':= 'T' or 'recover':='R'

	node_status Gnode_var;

	short int occur_time; //Priority queue is based on ordering of these times

} Event;

/* For displaying which people are 'S', 'I', or 'R'. */
typedef struct SIR_list_node {

	short int Gnode_identity_no;

	node_status Gnode_var;

	struct SIR_list_node *next;

} SIR_node;

/* A utility function; we will need this max function later in the code. */
short int max(short int a, short int b) {
	if (a > b)
		return a;
	return b;
}

/* This function simulates the probability of getting a head when a coin is tossed for different values of probability as passed

 * on to the function. Here we have taken as a convention to denote heads by returning '1' and a tail with returning '0'. In

 * accordance with the project-statement, we have taken τ = 0.5 and γ  = 0.2.

 */

bool prob_gen(float τγ) {

	if (τγ == 0.5) {

		return rand() % 2;

	} else /* which means if(τγ==0.2)*/{

		return (rand() % 5 == 0);

	}

}

/* Creates a graph of the type mentioned in the assignment and passes a reference of the graph created in main(). */

Gnode** Graph_Create(short int vertex_count, short int max_edge_count) {

	/* Predetermining the number of vertices (vertex_count) and maximum number of edges a particular vertex in the graph can

	 * have (max_edge_count).

	 */

	Gnode **Graph;

	/* Generates a dynamic array of graph nodes of random size. */

	if ((Graph = (Gnode**) malloc(sizeof(Gnode*) * (vertex_count))) == NULL) {

		printf("Memory demand exceeds memory allocation limits.\n");

		exit(1);

	}

	/* Graph[v_cnt] represents the array which needs to be allocated potentially enough links to be able to link to at best,
	 * the number of vertices in the graph. This is essentially an adjacency matrix.
	 */

	for (int v_cnt = 0; v_cnt < vertex_count; v_cnt++) {

		if ((Graph[v_cnt] = (Gnode*) malloc(sizeof(Gnode) * (vertex_count)))
				== NULL) {

			printf("Memory allocation for edges failed.\n");

			exit(1);

		}

		/* Rather than storing the following information in each element inside each element corresponding to the array
		 * related to each vertex, this is rather a compact way to store the same.
		 */
		Graph[v_cnt]->identity_no = v_cnt;

		Graph[v_cnt]->pred_inf_time = SHRT_MAX; //Assuming initially one may never get sick

	}

	/* IMPORTANT initialisation of all the edge links to NULL, to avoid nasty logic errors in the output. */
	for (int v_cnt = 0; v_cnt < vertex_count; v_cnt++) {
		for (int e_cnt = 0; e_cnt < vertex_count; e_cnt++) {
			Graph[v_cnt][e_cnt].edge_link = NULL;
		}
	}

	/* Commence building links in the graph. */

	/* A trace of dynamic programming here :). memoize[v_cnt] stores the number of vertices v_cnt is linked to. This is mainly
	 * to make sure that at no point in time, for any vertex v_cnt, memoize[v_cnt] exceeds max_edge_count. memoize[v_cnt] is
	 * itself built dynamically in the loop(s) to follow, and hence the name 'memoizing'.  */
	short int memoize[vertex_count];

	memset(memoize, 0, sizeof(memoize)); //Initially sets all the elements in memoize array to 0.

	for (int v_cnt = 0; v_cnt < vertex_count; v_cnt++) {

		/* If memoize[v_cnt] is = max_edge_count, there is no point in linking any other vertices with it. Note that because of
		 * this condition, memoize[v_cnt] can NEVER exceed max_edge_count.*/
		if (memoize[v_cnt] < max_edge_count) {

			for (int e_cnt = 0; e_cnt < vertex_count; e_cnt++) {

				/* Self loops are obviously absurd. */
				if (e_cnt == v_cnt) {

					Graph[v_cnt][e_cnt].edge_link = NULL;

					/*In case we haven't already linked these two nodes. */
				} else if (Graph[v_cnt][e_cnt].edge_link == NULL) {

					/* A vertex is to be paired with another vertex with probability 0.50. */
					/* The max statement is just a compact way of saying that if(memoize[v_cnt] < max_edge_count &&
					 * memoize[e_cnt] < max_edge_count. */
					if (rand() % 2 == 1
							&& max(memoize[v_cnt], memoize[e_cnt])
									< max_edge_count) {
						/* Similar linking as shown in the project (Sanfoundry)-link. */
						Graph[v_cnt][e_cnt].edge_link = &Graph[e_cnt][0];

						Graph[e_cnt][v_cnt].edge_link = &Graph[v_cnt][0];

						++memoize[v_cnt];
						++memoize[e_cnt];


					} else {

						/* Self - explanatory. */
						Graph[v_cnt][e_cnt].edge_link = NULL;

						Graph[e_cnt][v_cnt].edge_link = NULL;

					}
					/* We make all the people initially, susceptible. */
					Graph[v_cnt]->var = Graph[e_cnt]->var = Susceptible;

				}

			}

		}

	}

	return Graph;

}

/* Dynamic array to be used in priority queue. */
Event *Heap;
/* Serves as last index for elemts in the heap. */
int last_index = 0;

void Priority_Queue_Push(Event node) {

	int propogate = last_index + 1; //Index propogate is where we have to start out searching the right place for our Event node

	/* Standard implementation of priority_queue_insert. In principle, we have to insert the newest invent at the back end of
	 * the heap, and upgrade it forward till it reaches its correct position. */
	while (propogate > 1 && node.occur_time < Heap[propogate / 2].occur_time) {

		Heap[propogate] = Heap[propogate / 2];

		propogate /= 2;

	}

	/* Insert in the right place. */
	Heap[propogate] = node;

	/* Making sure to increase the value of last index after new insertion. */
	++last_index;

}

/* Self - explanatory. */
void swap(Event *a, Event *b) {

	Event temp;

	temp = *a;

	*a = *b;

	*b = temp;

}

Event Priority_Queue_Pop() {

	/* We always want to return the value at the first index in the heap. Note that we are trying to use 1-index based heap
	 * implementation. This is for the sake of ease of implementation.
	 */

	/* A point to be noted here is that when there are elements with same priority in a priority queue, then amongst them, the
	 * one which was pushed in first is to be popped first, in accordance with the FIFO rule of queues. However, we have not
	 * implemented this here. Here this particular behaviour of a queue is actually not required since when the time of the
	 * events is same, it doesn't really matter which gets processed first. Events are mostly mutually independent; in the cases
	 * where they aren't, we have taken care to handle the times accordingly in a later function.
	 * Maintaining a strict FIFO behaviour is certainly possible, but will require coding a few more functions, none of which is
	 * important in the present context.
	 */
	Event return_value = Heap[1];

	Heap[1] = Heap[last_index];

	Heap[last_index].occur_time = SHRT_MAX;

	--last_index;

	int pointer_1 = 1, pointer_2 = 2 * pointer_1;

	/* Standard way of deleting from a heap. */
	while (pointer_2 <= last_index) {

		/* Set integer pointer to the index of min of both the children. */
		if (pointer_2 < last_index
				&& Heap[pointer_2 + 1].occur_time
						<= Heap[pointer_2].occur_time) {

			++pointer_2;

		}

		/* Go downwards in the tree, till we have found the right place for the last element placed in the first place and keep
		 * on swapping things as we go along to maintain the rule of the heap. */
		if (Heap[pointer_1].occur_time >= Heap[pointer_2].occur_time) {

			swap(&Heap[pointer_1], &Heap[pointer_2]);

			pointer_1 = pointer_2;

			pointer_2 *= 2;

		} else {

			break;

		}

	}

	return return_value;

}

/* The function which helps us in printing the lists of S, I, R people. We do different things based on the value of operation
 * flag.
 */

void People_List(Event Individual, short int operation_flag) {

	/* Making static so that we can avoid making the pointers global. */
	static SIR_node *S_list_head = NULL; // Points to the head of the linked list holding 'S' people
	static SIR_node *S_list = NULL; // Points to the end of the linked list holding 'S' people
	static short int S_count = 0;

	static SIR_node *I_list_head = NULL; // Points to the head of the linked list holding 'I' people
	static SIR_node *I_list = NULL; // Points to the end of the linked list holding 'I' people
	static short int I_count = 0;

	static SIR_node *R_list_head = NULL; // Points to the head of the linked list holding 'R' people
	static SIR_node *R_list = NULL; // Points to the end of the linked list holding 'R' people
	static short int R_count = 0;

	/* In this case we just append all people in the S list, as initially all are susceptible. */
	if (operation_flag == 0) {

		/* Usual work as done in building a linked list. */
		SIR_node *append = (SIR_node*) malloc(sizeof(SIR_node));

		append->Gnode_identity_no = Individual.Gnode_identity_no;

		append->Gnode_var = Individual.Gnode_var;

		append->next = NULL;

		if (Individual.Gnode_var == Susceptible) {

			if (!S_list) {

				S_list_head = append;

				S_list = append;

			} else {

				S_list->next = append;

				S_list = append;

			}
			++S_count;
		}
		/* Here is where we interchange a person between lists, based on the status of the person stored in Event Individual.
		 * We also have access to the corresponding node in the graph, by virtue of this struct. */
	} else if (operation_flag == 1) {

		SIR_node *modify = NULL;

		SIR_node *assistant = NULL;

		switch (Individual.Gnode_var) {

		/* If he is infected and time of his infection is over, he has recovered and is thus moved accordingly. */
		case -1:

			/* Basic linked list manipulation. */
			modify = I_list_head;

			while (modify
					&& modify->Gnode_identity_no != Individual.Gnode_identity_no) {

				assistant = modify;

				modify = modify->next;

			}

			if (modify && assistant) {

				assistant->next = modify->next;
				/* Updating the I_list pointer to point to the updated end of the list. */
				if(assistant->next == NULL){
					I_list = assistant;
				}

			} else if (modify) {

				(modify->next == NULL) ? I_list = (I_list_head = modify->next) : (I_list_head = modify->next);
			}

			if (R_list_head) {

/*
				SIR_node* tvsl = R_list_head;
				while(tvsl->next){
					tvsl = tvsl->next;
				}
				tvsl->next = modify;
*/
				/* The below line is same as the above commented snippet of code, but just done more efficiently, in terms of
				 * time.
				 */
				R_list = R_list->next = modify;

			} else {

				R_list = R_list_head = modify;

			}

			if (modify) {

				modify->next = NULL;

				modify->Gnode_var = Recovered;

			}
			--I_count;
			++R_count;
			break;

			/* In case a person is susceptible and the time for his getting infected has come, transfer him from the S list to
			 * the I list. */
		case 0:

			/* Basic linked list manipulation. */
			modify = S_list_head;

			if (modify) {

				while (modify
						&& modify->Gnode_identity_no
								!= Individual.Gnode_identity_no) {

					assistant = modify;

					modify = modify->next;

				}

				if (modify && assistant) {

					assistant->next = modify->next;

				} else if (modify) {

					S_list_head = modify->next;

				}

				if (I_list_head) {

/*
					SIR_node* tvsl = I_list_head;
					while(tvsl->next){
						tvsl = tvsl->next;
					}
					tvsl->next = modify;
*/
					/* The below line is same as the above commented snippet of code, but just done more efficiently, in terms of
					 * time.
					 */
					I_list = I_list->next = modify;

				} else {

					I_list = I_list_head = modify;

				}

				if (modify) {

					modify->next = NULL;

					modify->Gnode_var = Infected;

				}

			}
			--S_count;
			++I_count;
			break;
			/* If he is already recovered, there is nothing to do really. */

		}

	} else {

		/* We just print the members of each of the linked lists. This is called if we have to only print the members of the
		 * linked lists. */
		SIR_node *data_reader = S_list_head;

		printf("Susceptible individuals (%d):- \n\n",S_count);

		if (!data_reader) {
			printf("None.\n");
		}

		while (data_reader) {
			printf("Individual-%d\n", data_reader->Gnode_identity_no + 1);
			data_reader = data_reader->next;
		}

		printf("\n");

		data_reader = I_list_head;

		printf("Infected individuals (%d):- \n\n",I_count);

		if (!data_reader) {
			printf("None.\n");
		}

		while (data_reader) {
			printf("Individual-%d\n", data_reader->Gnode_identity_no + 1);
			data_reader = data_reader->next;
		}

		printf("\n");

		data_reader = R_list_head;

		printf("Recovered individuals (%d):- \n\n",R_count);

		if (!data_reader) {
			printf("None.\n");
		}

		while (data_reader) {
			printf("Individual-%d\n", data_reader->Gnode_identity_no + 1);
			data_reader = data_reader->next;
		}

		printf("\n");

	}

}

void process_trans_SIR(Gnode **graph_reference, Event node) {

	short int offset = SHRT_MIN;

	Event new;

	for (int neighbour_node = 0; neighbour_node < vertex_count;
			neighbour_node++) {

		/* We loop to check all the neighbours of the graph. */
		if (graph_reference[node.Gnode_identity_no][neighbour_node].edge_link
				&& graph_reference[node.Gnode_identity_no][neighbour_node].edge_link
						== &graph_reference[neighbour_node][0]) {
			/* If the neighbour is susceptible, and has not yet been scheduled in the priority queue, we need to add him/her
			 * in the priority queue. */
			if (graph_reference[neighbour_node]->var == Susceptible
					&& graph_reference[neighbour_node]->in_queue == 0) {

				short int day = 0;
				/* Simulating the day at which the neighbour will get infected. */
				while (prob_gen(0.5) == 0 && day <= Tmax - node.occur_time){++day;}

				/* If the time comes out to be appropriate -- less than Tmax, and more than the infection time of the current
				 * person, which is the current time (node.occur_time), we insert an event in the queue corresponding to that
				 * person (to become infected). */
				if (day <= Tmax - node.occur_time) {

					new.Gnode_identity_no = neighbour_node;

					new.action_type = 'T';

					new.Gnode_var = graph_reference[neighbour_node]->var =
							Susceptible;

					new.occur_time = day + node.occur_time;

					graph_reference[neighbour_node]->pred_inf_time =
							new.occur_time;

					offset = max(offset, day + node.occur_time);

					/* The event is going to go in the queue, and hence the assignment. */
					graph_reference[neighbour_node]->in_queue = 1;
					Priority_Queue_Push(new);

				}

			}

		}

	}

	/* A person can only recover only after all his neighbours have been infected; offset is calculated accordingly. */
	if (offset < Tmax) {

		if (offset >= 0) {

			++offset;

		} else {
			/* If none of its neighbours manage to get in, then we set offset to the time just after it got infected, as that
			 * is the minimum time possible for it to be considered to be recovered. */
			offset = node.occur_time + 1;

		}

		/* The person corresponding to the Event node is going to recover so we simulate the time, make the action type recovery,
		 * update those in the graph and push in the event in the queue. */
		Event push_back;

		push_back.Gnode_identity_no = node.Gnode_identity_no;

		push_back.action_type = 'R';

		push_back.Gnode_var = Infected;

		while (prob_gen(0.2) == 0 && offset <= Tmax){++offset;}

		push_back.occur_time = offset;

		graph_reference[node.Gnode_identity_no]->pred_inf_time =
				push_back.occur_time;

		graph_reference[node.Gnode_identity_no]->var = Infected;
		Priority_Queue_Push(push_back);

	}

}

void process_rec_SIR(Gnode **graph_reference, Event Individual) {

	/* We update the status of the person in the graph. */
	graph_reference[Individual.Gnode_identity_no]->var = Recovered;
	/* We make sure he is transferred to 'R' list. */
	People_List(Individual, 1);

}

void Fast_SIR(Gnode **root) {

	/* The below snippet of code is mainly an initialisation of the nodes in the graph, although some parts of it have been
	 * done earlier in the code.  */

	Event store_info;
	bool initial_inf_count=0;
	for (int v_cnt = 0; v_cnt < vertex_count; v_cnt++) {

		root[v_cnt]->var = Susceptible;

		store_info.Gnode_identity_no = v_cnt;

		store_info.Gnode_var = root[v_cnt]->var;

		store_info.occur_time = SHRT_MAX;

		store_info.action_type = 'T';

		root[v_cnt]->in_queue = 0;

		/* We simulate initial_infecteds randomly by making them susceptible as for others, but in addition making their
		 * pred_inf_time 0, which is like equivalent to saying that they are initially infected. */

		if (rand() % 2) {

			store_info.occur_time = 0;

			root[v_cnt]->pred_inf_time = 0;

			root[v_cnt]->in_queue = 1;
			Priority_Queue_Push(store_info);
			initial_inf_count=1;
		}

		/* Storing everyone in the susceptible linked list. */
		People_List(store_info, 0);

	}

	/* In the case where it so happens that no event gets pushed in the priority queue, which has very low probability, but is
	 * not impossible, we force in the first node to be the initially infected individual.
	 */
	if(!initial_inf_count){
		store_info.Gnode_identity_no = 0;
		store_info.Gnode_var = Susceptible;
		store_info.action_type = 'T';
		store_info.occur_time = root[0]->pred_inf_time = 0;
		root[0]->in_queue=  1;
		Priority_Queue_Push(store_info);
	}

	/* Print everyone therein. */
	printf("====== Display of all the individuals =======\n");
	People_List(store_info, -1);
	printf("=============================================\n");

	/* Practically in direct accordance with the algorithm mentioned in the project PDF.  */
	/* We print the list only if there is a change in the list --- the topmost time in the priority-queue, as otherwise it is
	 * unchanged from the previous time.
	 * While we keep on making necessary changes while the loop is functioning, we only print the updated lists when there is
	 * a change in the time of the last event processed and current event being processed.
	 */

	short int previous_day = 00;
	while (Heap[1].occur_time != SHRT_MAX) {


		Event top = Priority_Queue_Pop();

		if(top.occur_time == previous_day){

		if (top.action_type == 'T') {
			if (top.Gnode_var == Susceptible) {
				People_List(top, 1);
				process_trans_SIR(root, top);
			}

		} else if (top.action_type == 'R') {

			process_rec_SIR(root, top);

		}

	}else if(top.occur_time - previous_day>0){

		printf("Day %d:-\n", previous_day + 1);
		People_List(top,-1);

		for(int day = previous_day+1; day<top.occur_time; day++){
			printf("Day %d (Unchanged):-\n", day + 1);
			People_List(top,-1);

		}

		if (top.action_type == 'T') {
			if (top.Gnode_var == Susceptible) {
				People_List(top, 1);
				process_trans_SIR(root, top);
			}

		} else if (top.action_type == 'R') {

			process_rec_SIR(root, top);
		}


	}
		previous_day = top.occur_time;

	}

	/* The last day will not be printed in the queue iterations, though the necessary changes to be done on the last day, will
	 * be execute inside the loop. Hence we print the last day outside the loop. */
	printf("Day %d:-\n", previous_day + 1);
	People_List(store_info,-1);
}

int main(void) {

	/* Seeding the rand() with the current time. */
	srand(time(NULL));

	vertex_count = 1 + rand() % MAX_VERTICES;

	max_edge_count = 1 + rand() % MAX_EDGES;

	//printf("Vertex_count: %d & Max_edge_count: %d\n", vertex_count, max_edge_count); if you so wish.

	Gnode **Invoke = Graph_Create(vertex_count, max_edge_count);

	/* Allocating enough memory to the heap. Note that 2*vertex_count is an upper bound for this since the number of events
	 * in the queue can be at most these many, when every individual's I - time and R - time are both to be processed. */
	Heap = (Event*) malloc(sizeof(Event) * vertex_count * 2);

	Fast_SIR(Invoke);

	return EXIT_SUCCESS;

}
/* Note: In the code-logic above, we have simulated keeping in mind the consideration that when a person gets infected, all of
 * the person's neighbours will get infected on the same day or afterwards. Also, that same person may recover after all the
 * neighbours have recovered or might be before that as well, but definitely strictly after all its neighbours have recovered.*/
