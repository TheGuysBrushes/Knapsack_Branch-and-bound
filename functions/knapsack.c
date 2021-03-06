#include "knapsack.h"

#include <stdlib.h>
#include <math.h>
#include <sys/timeb.h>
#include <string.h>

char verbose; /* defined in main.c */


void createNode(int n, int b, item *it, char intdata, char *x, char *constraint, TREE *newnode, TREE pred, int var_id, char sign, int rhs, double *bestobj, QUEUE *queue, unsigned int *nbnode)
{
    int frac_item; /* index (not the id) in array it[] of the item j */
    char status;   /* solveRelaxation status: 'i', 'u', or 'f'       */
    double objx;   /* solution value associated with solution x      */
    int j;

    *newnode = (TREE)calloc(1, sizeof(struct s_node));
    if(*newnode == NULL)
    {
    printf("\nMemory allocation problem, failing to create a tree newnode.\nExiting...\n");
    exit(EXIT_FAILURE);
    }
    (*newnode)->pred = pred;
    (*newnode)->var_id = var_id;
    (*newnode)->sign = sign;
    (*newnode)->rhs = rhs;
    (*newnode)->var_frac = -1; /* fracional variable unidentified yet */
    (*newnode)->status = 'n';

    /* Solving the node, for the purpose of determining its objective value: */
    /* (*newnode)->obj */

    generateConstraint(n, *newnode, constraint);

    status = solveRelaxation(n, b, it, constraint, x, &objx, &frac_item);
    if(verbose == 'v')
    {
	switch(status)
	{
	    case 'i': printf("\nsolveRelaxation solution is feasible (integer), with objx = %lf", objx);
	    break;
	    case 'u': printf("\nsolveRelaxation is unfeasible");
	    break;
	    case 'f': printf("\nsolveRelaxation solution is fractional with objx = %lf.\nItem %d having id %d, size %d and cost %d is used PARTIALLY in the solution, so branching is required.", objx, frac_item, it[frac_item].id, it[frac_item].a, it[frac_item].c);
	    break;
	    default: printf("\n\nUNEXPECTED CHAR VALUE RETURNED BY solveRelaxation: \"%c\"\n\n",status);
	}
    }
    (*newnode)->status = status;
    (*newnode)->obj = objx;

    
    //displaySol(n, b, it, x, objx);
    

    /* A new node is created only if its status is 'f' and its objective is stricly larger than *bestobj */
    if(status == 'f' && *bestobj < (intdata == '1'?floor(objx):objx))
    {
	(*nbnode)++;
	(*newnode)->var_frac = frac_item;
	/* Updating the parent node too */
	if((*newnode)->pred != NULL)
	    {
	    if((*newnode)->rhs == 0)
		(*newnode)->pred->suc0 = (*newnode);
	    else if((*newnode)->rhs == 1)
		(*newnode)->pred->suc1 = (*newnode);
	    else
		{
		printf("\nError in createNode(): cannot update the parent node because the passed rhs = %d whereas it should be 0 or 1.\n", rhs);
		}
	    }
	if(verbose == 'v')
	    displayNode(*newnode);
	/* Insert the current node in the queue, at the correct location */
	addToQueue(queue, *newnode);
	//printf("\nSize of queue = %d elements", sizeQueue(*queue));
    }
    else
    {
	if(verbose == 'v')
	    printf("\nNo node creation (status is '%c')", (*newnode)->status);
	if(status == 'f')
	    {
	    if(verbose == 'v')
		printf("\nDespite its status 'f', this node is pruned because its objective value is %lf, whereas the objective value of the best feasible solution found so far is %lf.", objx, *bestobj);
	    }
	else if(status == 'u')
	    {
	    /* Delete node, update its parent too. */
	    if(pred == NULL)
		{
		/* The root node is unfeasible */
		printf("\nThis knapsack problem instance is unfeasible.");
		}
	    }
	else if(status == 'i')
	    {
	    /* Compare objx to the best available integer solution,      */
	    /* and update it if necessary. The best solution is directly */
	    /* coded in the item structure, see char bestsol.            */
	    /* Delete node, update its parent too.                       */
	    if(objx > *bestobj)
		{
		*bestobj = objx;
		for(j = 0; j < n; j++)
		    it[j].bestsol = x[j];
		}
	    }

	/* Updating the parent node pred, if it exists */
	if(pred != NULL)
	{
	    if(rhs == 0)
		pred->suc0 = NULL;
	    else if(rhs == 1)
		pred->suc1 = NULL;
	    else {
// 		printdebug("\nError with rhs, that is %d while it should be either 0 or 1.\n", rhs);
	    }
	}
	/* Current node deletion */
	free(*newnode);
   }
}


boolean read_first_line(FILE* file, int* items_nb, int* capacity)
{
    char line[31];
    
    if (file != NULL)
    {
	if (fgets(line, 30, file) == NULL){
	    fprintf(stderr, "File empty");
	    return 0;
	}

	char* values= strtok (line," ");
	if (values != NULL){
// 	    printdebug("1st value %s\n", values);
	    *items_nb= atoi(values);
	
	    // next value
	    values = strtok (NULL, " ");
	}
	else {
	    fprintf(stderr, "No capacity value");
	    return 0;
	}
	
	if (values != NULL){
// 	    printdebug("2nb value %s\n", values);
	    *capacity= atoi(values);
	    // useless to get next value
	}
	else {
	    fprintf(stderr, "No items number");
	    return 0;
	}
	
	return 1;
    } else { return 0;}
}

tab_items init_items(FILE* file, int items_nb)
{
    char line[31];
    
    if (file != NULL)
    {    
	tab_items items= (tab_items)malloc(items_nb*sizeof(item));
	
	int i;
	for (i= 0; i < items_nb; ++i){
	    if (fgets(line, 30, file) == NULL){
		fprintf(stderr, "File empty");
		return 0;
	    }
	    
	    // Value reading
	    char* values= strtok(line, " ");
	    if (values != NULL){
		printdebug ("item id : %s\n", values);	    
		items[i].id=	atoi(values);
		// next value
		values = strtok (NULL, " ");
	    } else {
		fprintf(stderr, "No id");
		return 0;
	    }
	    
	    if (values != NULL){
		printdebug("size : %s\n", values);
		items[i].a=   atoi(values);
		// next value
		values = strtok (NULL, " ");
	    } else {
		fprintf(stderr, "No size");
		return 0;
	    }
	    
	    if (values != NULL){
		printdebug("cost : %s\n", values);
		items[i].c=   atoi(values);
		// useless to get next value
	    } else {
		fprintf(stderr, "No cost");
		return 0;
	    }
	    
	    printdebug("DEBUG : Item [%d] created : size %d, cost %d\n", items[i].id, items[i].a, items[i].c);
	    
	}
	return items;
	
    } else {
	fprintf(stderr, "Items creation impossible because file can't be read");
	return NULL;
    }
}

void loadInstance(char* filename, int *n, int *b, item **it)
{
/* TO COMPLETE */
/* DONE */

// Reading
    FILE *file;
    file= fopen(filename,"r");
    
    if (file != NULL){
	// File record and instanciation of items 
	if (read_first_line(file, n, b)){
	    *it= init_items(file, *n);
	}
	else {
	    fprintf(stderr, "First line reading impossible");
	    // verification needed after call of loadInstance to ensure
	    // there was not any problem during reading
	}
    }
    else {
	fprintf(stderr, "File reading impossible");
	// verification needed after call of loadInstance to ensure
	// there was not any problem during reading
    }
//     printdebug("Je sors de loadInstance\n");
}

/*********************************************************/
/*                       comp_struct                     */
/*********************************************************/
static int comp_struct(const void* p1, const void* p2)
{
    /* TODO a verifier a l'execution */
    item i1 = *( (item*) p1);
    item i2 = *( (item*) p2);
    boolean isGreaterThan = ( ((float)i1.c/(float)i1.a)  < ((float)i2.c/(float)i2.a) );

    printdebug("\nL'item %d est plus grand que l'item %d", i1.id, i2.id);

    return isGreaterThan;
} /* end of comp_struct */

char solveRelaxation(int n, int b, item *it, char *constraint, char *x, double *objx, int *frac_item)
{
    /* TODO TO COMPLETE */
    
    /**
     * Because we use this fonction on an array of item sorted by decreasing utility, the first fractionnal item is the best item to fills the bag.
     * When we meet this one, we won't need any of the remaining items (so we put their value in x to '0')
     */
    
    //if the total size of all the items presents in the knapsack exceeds the knapsack size, the problem is infeasible
    int num_constraints, total_constraint_items = 0;
    unsigned int totalPoidsItems = 0;
    for(num_constraints= 0; num_constraints < n; ++num_constraints){
	if(constraint[num_constraints] == '1'){
	    x[num_constraints] = '1';
	    (*objx)+= it[num_constraints].c;
	    ++total_constraint_items;
	    printdebug("\ntotalPoidsItems = %d -> ajout de l'item %d de poids %d", totalPoidsItems, it[num_constraints].id, it[num_constraints].a);
	    totalPoidsItems+= it[num_constraints].a;
	    if(totalPoidsItems > b) {
		return 'u';
	    }
	}
    }
    
    printdebug("\n*****************\nIl y a %d items dans constraint\n*****************\n", total_constraint_items);
    //if the execution is at this point, there is some space left in the bag

    int indiceIt;
    for(indiceIt= 0; indiceIt < n; ++indiceIt){
	if(constraint[indiceIt] == 'F'){
	    // Check if the item fits in the bag
	    if(totalPoidsItems+it[indiceIt].a < b){
		//the item can be added to the knapsack
		printdebug("\nThe item %d can be added to the knapsack : %d et %d = %d < %d", indiceIt, totalPoidsItems, it[indiceIt].a, totalPoidsItems+it[indiceIt].a, b);
		totalPoidsItems+= it[indiceIt].a;
		//the item added to the knapsack will become a condition to check up
		x[indiceIt]= '1';
// 		(*objx)+= (double)it[indiceIt].c;
	    } else if (totalPoidsItems+it[indiceIt].a == b){
		    //the item fills the knapsack until its maximum size ; we add it into the knapsack
		    (*objx)+= (double)it[indiceIt].c;
		    (*frac_item)= -1;
		    return 'i';
	    } else {
		//the item is too large, we will have to add some part of it (fractionnal item)
		printdebug("\nOn passe au fractionnal avec l'item %d qui depasserait du sac (%d > %d)", indiceIt, totalPoidsItems+it[indiceIt].a, b);
		
		int subst= indiceIt;
		int repriseAvanceeIndice;
		for(repriseAvanceeIndice= subst+1; repriseAvanceeIndice < n; ++repriseAvanceeIndice){
		    printdebug("\nOn met a 0 la case : %d sur %d dans constraint", repriseAvanceeIndice, n);
		    x[repriseAvanceeIndice]= '0';
		}
		
		x[indiceIt]= '?';
		(*frac_item)= indiceIt;
		//computing the proportion of the item that is added to the objective value and multiply it by the cost of the item
		double res= ( ((double)(b-totalPoidsItems)) / (it[indiceIt].a) );
		res *= (it[indiceIt].c);
		printdebug("\nFractionnal adding : %f", res);
		(*objx)+= res;
		return 'f';
	    }
	}//if the item is not free, we don't use it
    }//end of the process of the items
    
    return '\0';
}

void BB(int n, int b, item *it, double *bestobj)
{
    char intdata; /* set to '1' if all items profits are integer */
    char *constraint; /* constraint[j] = 'F' if item j (in the index number after sorting the index by decreasing utility) is not subject to a branching constraint (free). '0' if item j is not selected, '1' if it is selected */
    char *x; /* solution returned by solveRelaxation, '0', '1', '?' */
    /* such that x[j] = '?' ie the fractional item. */
    double initial_best_obj; /* for storing *bestobj before solving an open problem, and see if its value has increased or not. If yes, we call prune() */
    TREE p, root = NULL;   /* tree node */
    QUEUE q, queue = NULL; /* queue of open problems (i.e. nodes for which */
    /* the objective value is known, and that have a fractional item)   */
    unsigned int nbTreeNodes = 0;


    constraint = (char*)calloc(n, sizeof(char));
    x = (char*)calloc(n, sizeof(char));
    if(constraint == NULL || x == NULL)
    {
	printf("\nMemory allocation problem.\nExiting...\n");
	exit(EXIT_FAILURE);
    }

    /* Setting intdata */
    intdata = integerProfit(n, it);

    /* Sorting the items by decreasing utility */

    /** TO COMPLETE **/
    qsort(it, n, sizeof(item), comp_struct);

    /* Branch-and-Bound starts here */


    /* creating root node */
    createNode(n, b, it, intdata, x, constraint, &root, NULL, -1, 's', -1, bestobj, &queue, &nbTreeNodes);
    //printf("\nThe queue of open problems has %d elements.", sizeQueue(queue));
    //printf("\nThe best obj is %lf", *bestobj);

    /* Here, we should decide what to do with the queue of open problems. */


    while (queue != NULL)
    {
	/* Solve the first open problem in the queue. */
	initial_best_obj = *bestobj;
	q = queue;
	createNode(n, b, it, intdata, x, constraint, &p, q->ptrnode, q->ptrnode->var_frac, '=', 0, bestobj, &queue, &nbTreeNodes);
	createNode(n, b, it, intdata, x, constraint, &p, q->ptrnode, q->ptrnode->var_frac, '=', 1, bestobj, &queue, &nbTreeNodes);
	deleteNodeQueue(&queue, q);

	/* At this point, we should check that an integer solution has not been found, because in that case, it could be advantageous to delete (from the queue and in the tree) those open problems that have a .obj value less than or equal to *bestobj */
	if(initial_best_obj < *bestobj)
	    prune(&queue, bestobj, intdata);


	/* DEBUG */
	if(verbose == 'v')
	{
	    printf("\nIn BB, this is displayTree, called for a 'root' node which address is %p.", root);
	    displayTree(root);
	    printf("\nIn BB, the size of the queue is %d", sizeQueue(queue));
	    displayQueue(queue);
	    /* DEBUG END */
	}

    } /* end while(queue != NULL) */

    printf("\nNumber of tree nodes: %u", nbTreeNodes);
    free(constraint);
    free(x);
}

