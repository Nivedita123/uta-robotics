#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>

#define DIM 512

/******************************************************************/
/* This structure contains the coordinates of a box drawn with    */
/* the left mouse button on the image window.                     */
/* roi.x , roi.y  - left upper corner's coordinates               */
/* roi.width , roi.height - width and height of the box           */
/******************************************************************/
extern XRectangle roi;

/* method to apply given edge filter on the given image */
void apply_edge_filter(filter, img, filter_img, row, col) int filter[3][3];
unsigned char img[DIM][DIM];
int filter_img[DIM][DIM];
int row;
int col;
{
    int i, j;
    int p_i, p_j;
    int v_i, v_j;

    for (i = 0; i < row - 2; i++)
    {
        for (j = 0; j < col - 2; j++)
        {
            filter_img[i][j] = 0;
            for (p_i = i, v_i = 0; v_i < 3; p_i++, v_i++)
            {
                for (p_j = j, v_j = 0; v_j < 3; p_j++, v_j++)
                {
                    filter_img[i][j] += img[p_i][p_j] * filter[v_i][v_j];
                }
            }
        }
    }
}

/* method to normalize the filtered image */
void normalize(filter_img, proc_img, row, col) int filter_img[DIM][DIM];
unsigned char proc_img[DIM][DIM];
int row;
int col;
{
    int min = 999;
    int max = -999;
    int i, j;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            // find min
            if (filter_img[i][j] < min)
            {
                min = filter_img[i][j];
            }
            // find max
            if (filter_img[i][j] > max)
            {
                max = filter_img[i][j];
            }
        }
    }

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if (min != max)
            {
                // normalize image
                proc_img[i][j] = (unsigned char)((filter_img[i][j] - min) * 255 / (max - min));
            }
            else
            { // no edge found
                proc_img[i][j] = 0;
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
    int horizontal_filter[3][3] = {
        {-1, -1, -1},
        {0, 0, 0},
        {1, 1, 1}};

    int row = size[0];
    int col = size[1];
    int filter_img[DIM][DIM];
    int i, j;

    apply_edge_filter(horizontal_filter, image, filter_img, row, col);
    normalize(filter_img, proc_img, row - 2, col - 2);
}
