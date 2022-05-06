#ifndef __dynamic_task_cpp__
#define __dynamic_task_cpp__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "mpi.h"
#include "project_sub.h"

int main(int argc, char* argv[])
{
	const int buf_row_size = 10;
	const int buf_col_size = 400;
	int np, me, col, i, j, m, n, count, k, proc_id;
	int map[400][400];
	int blocklengths[] = { 1,1,1 };
	int data_tag, result_tag, terminator_tag, source_tag;
	coop temp_co;
	float scale_real, scale_img, real_min, real_max, disp_width, disp_height, img_max, img_min;
	complex c;
	const int nitems = 3;
	MPI_Datatype types[3] = { MPI_INT, MPI_INT, MPI_INT };
	MPI_Datatype mpi_coor_type;
	MPI_Aint	 offsets[3];
	coop* list;

	offsets[0] = offsetof(coop, x);
	offsets[1] = offsetof(coop, y);
	offsets[2] = offsetof(coop, color);

	disp_width = 400;
	disp_height = 400;
	real_max = 1.0;
	real_min = -2.0;
	img_max = 1.5;
	img_min = -1.5;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);

	data_tag = 42;
	result_tag = 43;
	terminator_tag = 44;

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_coor_type);
	MPI_Type_commit(&mpi_coor_type);

	if (np < 2) {
		fprintf(stderr, "Requires at least two processes\n");
		exit(-1);
	}

	scale_real = (real_max - real_min) / disp_width;
	scale_img = (img_max - img_min) / disp_height;
	list = (coop*)malloc(sizeof(coop) * 400 * 10);

	if (me == 0) {
		count = 0;
		col = 0;
		for (k = 0; k < np - 1; k++) {		//initial send
			MPI_Send(&col, 1, MPI_INT, k + 1, data_tag, MPI_COMM_WORLD);
			count++;
			col += 10;
		}

		do {
			MPI_Recv(list, 400*10, mpi_coor_type, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD, &status);
			count--;
			proc_id = status.MPI_SOURCE;
			for (i = 0; i < 400 * 10; i++)
				map[list[i].y][list[i].x] = list[i].color;

			if (col < disp_width) {
				MPI_Send(&col, 1, MPI_INT, proc_id, data_tag, MPI_COMM_WORLD);
				col += 10;
				count++;
			}
			else
				MPI_Send(&col, 1, MPI_INT, proc_id, terminator_tag, MPI_COMM_WORLD);
		} while (count>0);
	}

	else {
		MPI_Recv(&col, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);	//receive first row
		source_tag = status.MPI_TAG;
		//receive column no. 
		while (source_tag == data_tag) {
			//using column no. make list(400x10) and send it to root
			for (i = 0; i < disp_height; i++) {
				c.real = real_min + ((float)i * scale_real);
				for (j = col; j < col + 10; j++) {
					c.imag = img_min + ((float)j * scale_img);
					list[10 * i + j - col].x = j;
					list[10 * i + j - col].y = i;
					list[10 * i + j - col].color = cal_pixel(c);
				}
			}
			MPI_Send(list, 400 * 10, mpi_coor_type, 0, result_tag, MPI_COMM_WORLD);
			MPI_Recv(&col, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			source_tag = status.MPI_TAG;
		}
	}
	if (me == 0) {
		for (i = 0; i < 400; i++) {
			for (j = 0; j < 400; j++)
				printf("%d ", map[i][j]);
		}
		printf("\n");
	}
	MPI_Type_free(&mpi_coor_type);
	MPI_Finalize();
	free(list);
	exit(0);
}

#endif // 
