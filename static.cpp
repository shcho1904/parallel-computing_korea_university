#ifndef __static__project_one__
#define __static__project_one__

#pragma warning(disable:4996)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "mpi.h"
#include "project_sub.h"

int main(int argc, char* argv[])
{
	int np, me, i, col, j, m, n;
	int map[400][400];
	int blocklengths[] = { 1,1,1 };
	int tag = 42;
	coop temp_co;
	float scale_real, scale_img, real_min, real_max, disp_width, disp_height, img_max, img_min;
	complex c;
	const int nitems = 3;
	MPI_Datatype types[3] = { MPI_INT, MPI_INT, MPI_INT };
	MPI_Datatype mpi_coor_type;
	MPI_Aint	 offsets[3];
	coop* list;
	FILE* fp;
	const char* filename = "new1.ppm";
	static unsigned char rgb_color[3];
	const char* comment = "# ";

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

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_coor_type);
	MPI_Type_commit(&mpi_coor_type);

	if (np < 2) {
		fprintf(stderr, "Requires at least two processes\n");
		exit(-1);
	}

	scale_real = (real_max - real_min) / disp_width;
	scale_img = (img_max - img_min) / disp_height;
	list = (coop*)malloc(sizeof(coop) * 400 * 10);

	for (i = 0; i < 40 / (np - 1); i++)
	{
		if (me == 0) {	//master
			for (j = 0, col = (np - 1) * i * 10; j < (np - 1); j++, col = col + 10) {
				MPI_Send(&col, 1, MPI_INT, j + 1, tag, MPI_COMM_WORLD);
				MPI_Recv(list, 400 * 10, mpi_coor_type, j + 1, tag, MPI_COMM_WORLD, &status);
				for (n = 0; n < 400; n++) {
					for (m = 0; m < 10; m++) {
						coop temp_cood = list[10 * n + m];
						map[temp_cood.y][temp_cood.x] = temp_cood.color;
					}
				}
			}
		}

		else {		//slave
			MPI_Recv(&col, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

			for (n = 0; n < 400; n++) {
				for (m = col; m < col + 10; m++) {
					c.real = real_min + ((float)n * scale_real);
					c.imag = img_min + ((float)m * scale_img);
					temp_co.color = cal_pixel(c);
					temp_co.x = m;
					temp_co.y = n;
					list[10 * n + m - col] = temp_co;
				}
			}
			MPI_Send(list, 400 * 10, mpi_coor_type, 0, tag, MPI_COMM_WORLD);
		}

	}
	if (me == 0) {
		fp = fopen(filename, "wb");
		fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, 400, 400, 255);
		for (i = 0; i < 400; i++) {
			for (j = 0; j < 400; j++) {
				if (map[i][j] == 255) {
					rgb_color[0] = 0;
					rgb_color[1] = 0;
					rgb_color[2] = 0;
				}
				else {
					rgb_color[0] = 255;
					rgb_color[1] = 255;
					rgb_color[2] = 255;
				}
				fwrite(rgb_color, 1, 3, fp);
			}
		}
		fclose(fp);
	}
	MPI_Type_free(&mpi_coor_type);
	MPI_Finalize();
	exit(0);
}

#endif // 
