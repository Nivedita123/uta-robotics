#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#define DIM 512

/******************************************************************/
/* This structure contains the coordinates of a box drawn with    */
/* the left mouse button on the image window.                     */
/* roi.x , roi.y  - left upper corner's coordinates               */
/* roi.width , roi.height - width and height of the box           */
/******************************************************************/
extern XRectangle roi;

int similar(int pixel1, int pixel2)
{
    return abs(pixel1 - pixel2) <= 20;
}

void generate_colors_for_labels(unsigned char colors[], int labels[], int size)
{
    int min = 9999;
    int max = -9999;
    int i;

    for (i = 0; i < size; i++)
    {
        int label = labels[i];
        // find min
        if (label < min)
        {
            min = label;
        }
        // find max
        if (label > max)
        {
            max = label;
        }
    }

    for (i = 0; i < size; i++)
    {
        if (min != max)
        {
            // normalize value
            colors[i] = (unsigned char)((labels[i] - min) * 255 / (max - min));
        }
        else
        {
            // no match
            colors[i] = 0;
        }
    }
}

void color_the_blobs(unsigned char colors[], int blobs[DIM][DIM], unsigned char proc_img[DIM][DIM], int row, int col)
{
    int i, j;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            proc_img[i][j] = colors[blobs[i][j]];
        }
    }
}

void remove_outliers(unsigned char img[DIM][DIM], int row, int col)
{
    int i, j;
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            int u_i = i - 1;
            int d_i = i + 1;
            int l_j = j - 1;
            int r_j = j + 1;

            int up_color = u_i >= 0 ? img[u_i][j] : -99;
            int down_color = d_i < row ? img[d_i][j] : -99;
            int left_color = l_j >= 0 ? img[i][l_j] : -99;
            int right_color = r_j < col ? img[i][r_j] : -99;
            int curr_color = img[i][j];

            if (!similar(curr_color, up_color) && !similar(curr_color, down_color) && !similar(curr_color, left_color) && !similar(curr_color, right_color))
            {
                img[i][j] = (unsigned char)left_color;
                printf("Outlier: %d at %d, %d\n", curr_color, i, j);
            }
        }
    }
}

int eq_pair_exists(int l1, int l2, int ep[500][2], int ep_size)
{
    int i;
    int found = 0;
    for (i = 0; i <= ep_size; i++)
    {
        if ((ep[i][0] == l1 && ep[i][1] == l2) || (ep[i][0] == l2 && ep[i][1] == l2))
        {
            found = 1;
            break;
        }
    }
    return found;
}

void generate_final_labels(int final_labels[500], int combined_eq_pairs[200][200], int combined_eq_pairs_size)
{
    int i, j;

    for (i = 0; i <= combined_eq_pairs_size; i++)
    {
        j = 0;
        while (combined_eq_pairs[i][j] != -1)
        {
            final_labels[combined_eq_pairs[i][j]] = i;
            j++;
        }
    }
}

int add_unique_labels(int last_label, int cep[50][200], int cep_size)
{
    int visited[last_label + 1];
    int i, j;

    for (i = 0; i <= last_label; i++)
    {
        visited[i] = 0;
    }

    for (i = 0; i <= cep_size; i++)
    {
        j = 0;
        while (cep[i][j] != -1)
        {
            int label = cep[i][j];
            visited[label] = -1;
            j++;
        }
    }

    for (i = 0; i <= last_label; i++)
    {
        if (visited[i] != -1)
        {
            cep_size++;
            cep[cep_size][0] = i;
            cep[cep_size][1] = -1;
        }
    }
    return cep_size;
}

/*
Queue implementation begin.
Reference: https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/

struct QNode
{
    int key;
    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue
{
    struct QNode *front, *rear;
};

// A utility function to create a new linked list node.
struct QNode *newNode(int k)
{
    struct QNode *temp = (struct QNode *)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enQueue(struct Queue *q, int k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

int isEmpty(struct Queue *q)
{
    return q->rear == NULL;
}

// Function to remove a key from given queue q
int deQueue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return -1;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    int val = temp->key;
    free(temp);

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    return val;
}

/*
Queue implementation end.
Reference: https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/

int combine_eq_pairs(int ep[500][2], int cep[200][200], int ep_size)
{
    int i, j;
    int cep_row = -1;

    for (i = 0; i < ep_size; i++)
    {
        if (ep[i][0] != -1 && ep[i][1] != -1)
        {
            cep_row++;
            int cep_col = -1;

            struct Queue *q = createQueue();

            enQueue(q, ep[i][0]);
            enQueue(q, ep[i][1]);

            ep[i][0] = -1; // mark visited
            ep[i][1] = -1; // mark visited

            while (!isEmpty(q))
            {
                cep_col++;

                int label = deQueue(q);

                cep[cep_row][cep_col] = label;

                for (j = i + 1; j < ep_size; j++)
                {
                    if (ep[j][0] != -1 && ep[j][1] != -1)
                    {
                        if (ep[j][0] == label)
                        {
                            enQueue(q, ep[j][1]);
                            ep[j][0] = -1;
                            ep[j][1] = -1;
                        }
                        if (ep[j][1] == label)
                        {
                            enQueue(q, ep[j][0]);
                            ep[j][0] = -1;
                            ep[j][1] = -1;
                        }
                    }
                }
            }
            cep_col++;
            cep[cep_row][cep_col] = -1;
        }
    }
    return cep_row;
}

void normalize(image, row, col) char image[DIM][DIM];
int row;
int col;
{
    double min = 9999.9;
    double max = -9999.9;
    int i, j;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            // find min
            if (image[i][j] < min)
            {
                min = image[i][j];
            }
            // find max
            if (image[i][j] > max)
            {
                max = image[i][j];
            }
        }
    }

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (min != max)
            {
                // normalize value
                image[i][j] = ((image[i][j] - min) * 255 / (max - min));
            }
            else
            {
                // no match
                image[i][j] = 0;
            }
        }
    }
}

/******************************************************************/
/* Main processing routine. This is called upon pressing the      */
/* Process button of the interface.                               */
/* image  - the original greyscale image                          */
/* size   - the actual size of the image                          */
/* proc_image - the image representation resulting from the       */
/*              processing. This will be displayed upon return    */
/*              from this function.                               */
/******************************************************************/
void process_image(image, size, proc_img) unsigned char image[DIM][DIM];
int size[2];
unsigned char proc_img[DIM][DIM];
{
    int row = size[0];
    int col = size[1];
    int i, j;
    int blobs[DIM][DIM];

    int eq_pairs[500][2];

    // pass 1

    int max_label = 0;
    int eq_pair_size = -1;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {

            int i_upper = i - 1;
            int j_left = j - 1;

            int x_upper = i_upper >= 0 ? image[i_upper][j] : -99;
            int x_left = j_left >= 0 ? image[i][j_left] : -99;
            int x_current = image[i][j];

            int label_upper = i_upper >= 0 ? blobs[i_upper][j] : -99;
            int label_left = j_left >= 0 ? blobs[i][j_left] : -99;

            if (similar(x_left, x_current))
            {

                blobs[i][j] = label_left;
            }
            if (similar(x_left, x_current) && similar(x_upper, x_current) && label_left != label_upper)
            {

                blobs[i][j] = label_left < label_upper ? label_left : label_upper;

                if (!eq_pair_exists(label_left, label_upper, eq_pairs, eq_pair_size))
                {
                    eq_pair_size++;
                    eq_pairs[eq_pair_size][0] = label_left;
                    eq_pairs[eq_pair_size][1] = label_upper;
                }
            }
            if (!similar(x_left, x_current) && similar(x_upper, x_current))
            {
                blobs[i][j] = label_upper;
            }
            if (!similar(x_left, x_current) && !similar(x_upper, x_current))
            {
                blobs[i][j] = max_label;
                max_label++;
            }
        }
    }

    int combined_eq_pairs[200][200];
    int combined_eq_pairs_size = combine_eq_pairs(eq_pairs, combined_eq_pairs, eq_pair_size);
    combined_eq_pairs_size = add_unique_labels(max_label, combined_eq_pairs, combined_eq_pairs_size);

    int final_labels[max_label + 1];
    generate_final_labels(final_labels, combined_eq_pairs, combined_eq_pairs_size);

    unsigned char colors[max_label + 1];
    generate_colors_for_labels(colors, final_labels, max_label + 1);

    //pass 2
    color_the_blobs(colors, blobs, proc_img, row, col);
    //remove_outliers(proc_img, row, col);
    normalize(proc_img, row, col);
    printf("\n");
}
