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

void clip_region(image, region, x, y, row, col) unsigned char image[DIM][DIM];
double region[roi.height][roi.width];
int x;
int y;
int row;
int col;
{
    int i, j;

    for (i = 0; i < roi.height; i++)
    {
        for (j = 0; j < roi.width; j++)
        {
            if (x + i >= row || y + j >= col)
            {
                region[i][j] = 0.0;
            }
            else
            {
                region[i][j] = image[x + i][y + j];
            }
        }
    }
}

double calc_mean(matrix) double matrix[roi.height][roi.width];
{
    int i, j;
    double sum = 0;
    int n = roi.height * roi.width;

    for (i = 0; i < roi.height; i++)
    {
        for (j = 0; j < roi.width; j++)
        {
            sum += matrix[i][j];
        }
    }
    double mean = sum / n;
    return mean;
}

double calc_std_dev(matrix) double matrix[roi.height][roi.width];
{
    int i, j;
    double temp[roi.height][roi.width];
    double mean = calc_mean(matrix);

    for (i = 0; i < roi.height; i++)
    {
        for (j = 0; j < roi.width; j++)
        {
            temp[i][j] = pow(matrix[i][j] - mean, 2);
        }
    }
    double std_dev = sqrt(calc_mean(temp));
    return std_dev;
}

double zncc_convolute(region, template) double region[roi.height][roi.width];
double template[roi.height][roi.width];
{
    double template_std_dev = calc_std_dev(template);
    double template_mean = calc_mean(template);

    double region_std_dev = calc_std_dev(region);
    double region_mean = calc_mean(region);

    double total_pixels = roi.width * roi.height;
    double sum = 0.0;
    int i, j;

    for (i = 0; i < roi.height; i++)
    {
        for (j = 0; j < roi.width; j++)
        {
            sum += (region[i][j] - region_mean) * (template[i][j] - template_mean) * (1.0 / (region_std_dev * template_std_dev));
        }
    }

    double res = sum / total_pixels;
    return res;
    return 0.0;
}

void create_template(image, template) unsigned char image[DIM][DIM];
double template[roi.height][roi.width];
{
    int i, j;

    for (i = 0; i < roi.height; i++)
    {
        for (j = 0; j < roi.width; j++)
        {
            template[i][j] = (double)image[roi.x + i][roi.y + j];
        }
    }
}

void normalize(image, nrml_img, row, col) double image[DIM][DIM];
unsigned char nrml_img[DIM][DIM];
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
                nrml_img[i][j] = (unsigned char)((image[i][j] - min) * 255 / (max - min));
            }
            else
            {
                // no match
                nrml_img[i][j] = 0;
            }
        }
    }
}

void calc_normalized_cross_correlation(image, template, row, col, zncc_matrix) unsigned char image[DIM][DIM];
double template[roi.height][roi.width];
int row;
int col;
double zncc_matrix[DIM][DIM];
{
    int i, j;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            double region[roi.height][roi.width];
            clip_region(image, region, i, j, row, col);
            zncc_matrix[i][j] = zncc_convolute(region, template);
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
    int row = size[0]; // total rows
    int col = size[1]; // total cols

    double template[roi.height][roi.width]; // the template to match
    create_template(image, template);

    double zncc_matrix[DIM][DIM]; // correlation matrix (un-normalized)
    calc_normalized_cross_correlation(image, template, row, col, zncc_matrix);

    normalize(zncc_matrix, proc_img, row, col);
}
