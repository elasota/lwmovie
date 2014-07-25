/*
 * Copyright (C) 2007 Vitor <vitor1001@gmail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file cbook_gen.c
 * Codebook Generator using the ELBG algorithm
 */

#include <string.h>
#include <stdlib.h>

#include <vector>

#include "elbg.h"
#include "sb4random.h"
#include "sb4.h"

typedef __int64 I64_t;


#define DELTA_ERR_MAX 0.1  ///< Precision of the ELBG algorithm (as percentual error)

/**
 * In the ELBG jargon, a cell is the set of points that are closest to a
 * codebook entry. Not to be confused with a RoQ Video cell. */
typedef struct cell_s {
	int index;
	struct cell_s *next;
} cell;

typedef cell *cell_ptr;

/**
 * ELBG internal data
 */
typedef struct{
	I64_t error;
	int dim;
	int numCB;
	int *codebook;
	int *nearest_cb;
	int *points;

	std::vector<cell *> cells;
	std::vector<int> utility;
	std::vector<int> utility_inc;

	AVRandomState *rand_state;
} elbg_data;

static inline int distance_limited(int *a, int *b, int dim, int limit)
{
	int i, dist=0;
	for (i=0; i<dim; i++) {
		// Riot - Integer overflow fix
		int addition = (a[i] - b[i])*(a[i] - b[i]);
		if(limit - addition <= dist)
			return INT_MAX;
		dist += addition;
	}

	return dist;
}

static inline void vect_division(int *res, int *vect, int div, int dim)
{
	int i;
	if (div > 1)
		for (i=0; i<dim; i++)
			res[i] = ROUNDED_DIV(vect[i],div);
	else if (res != vect)
		memcpy(res, vect, dim*sizeof(int));

}

static int eval_error_cell(elbg_data *elbg, int *centroid, cell *cells)
{
	int error=0;
	for (; cells; cells=cells->next)
		error += distance_limited(centroid, elbg->points + cells->index*elbg->dim, elbg->dim, INT_MAX);

	return error;
}

static int get_closest_codebook(elbg_data *elbg, int index)
{
	int i, pick=0, diff, diff_min = INT_MAX;
	for (i=0; i<elbg->numCB; i++)
		if (i != index) {
			diff = distance_limited(elbg->codebook + i*elbg->dim, elbg->codebook + index*elbg->dim, elbg->dim, diff_min);
			if (diff < diff_min) {
				pick = i;
				diff_min = diff;
			}
		}
	return pick;
}

static int get_high_utility_cell(elbg_data *elbg)
{
	int i=0;
	/* Using linear search, do binary if it ever turns to be speed critical */
	int rbase = av_random(elbg->rand_state);
	int rdiv = elbg->utility_inc[elbg->numCB-1];
	int r = rbase%rdiv;
	while (elbg->utility_inc[i] < r)
		i++;
	return i;
}

/**
 * Implementation of the simple LBG algorithm for just two codebooks
 */
static int simple_lbg(int dim,
					  int *centroid[3],
					  int newutility[3],
					  int *points,
					  cell *cells)
{
	int i, idx;
	int numpoints[2] = {0,0};
	int newcentroid[2][ELBG_MAX_DIM];
	cell *tempcell;

	memset(newcentroid, 0, sizeof(newcentroid));

	newutility[0] =
	newutility[1] = 0;

	for (tempcell = cells; tempcell; tempcell=tempcell->next) {
		idx = distance_limited(centroid[0], points + tempcell->index*dim, dim, INT_MAX)>=
			  distance_limited(centroid[1], points + tempcell->index*dim, dim, INT_MAX);
		numpoints[idx]++;
		for (i=0; i<dim; i++)
			newcentroid[idx][i] += points[tempcell->index*dim + i];
	}

	vect_division(centroid[0], newcentroid[0], numpoints[0], dim);
	vect_division(centroid[1], newcentroid[1], numpoints[1], dim);

	for (tempcell = cells; tempcell; tempcell=tempcell->next) {
		int dist[2] = {distance_limited(centroid[0], points + tempcell->index*dim, dim, INT_MAX),
					   distance_limited(centroid[1], points + tempcell->index*dim, dim, INT_MAX)};
		int idx = dist[0] > dist[1];
		newutility[idx] += dist[idx];
	}

	return newutility[0] + newutility[1];
}

static void get_new_centroids(elbg_data *elbg, int huc, int *newcentroid_i,
							  int *newcentroid_p)
{
	cell *tempcell;
	int min[ELBG_MAX_DIM];
	int max[ELBG_MAX_DIM];
	int i;

	for (i=0; i< elbg->dim; i++) {
		min[i]=INT_MAX;
		max[i]=0;
	}

	for (tempcell = elbg->cells[huc]; tempcell; tempcell = tempcell->next)
		for(i=0; i<elbg->dim; i++) {
			min[i]=FFMIN(min[i], elbg->points[tempcell->index*elbg->dim + i]);
			max[i]=FFMAX(max[i], elbg->points[tempcell->index*elbg->dim + i]);
		}

	for (i=0; i<elbg->dim; i++) {
		newcentroid_i[i] = min[i] + (max[i] - min[i])/3;
		newcentroid_p[i] = min[i] + (2*(max[i] - min[i]))/3;
	}
}

/**
 * Add the points in the low utility cell to its closest cell. Split the high
 * utility cell, putting the separed points in the (now empty) low utility
 * cell.
 *
 * @param elbg		 Internal elbg data
 * @param indexes	  {luc, huc, cluc}
 * @param newcentroid  A vector with the position of the new centroids
 */
static void shift_codebook(elbg_data *elbg, int *indexes,
						   int *newcentroid[3])
{
	cell *tempdata;
	cell **pp = &elbg->cells[indexes[2]];

	while(*pp)
		pp= &(*pp)->next;

	*pp = elbg->cells[indexes[0]];

	elbg->cells[indexes[0]] = NULL;
	tempdata = elbg->cells[indexes[1]];
	elbg->cells[indexes[1]] = NULL;

	while(tempdata) {
		cell *tempcell2 = tempdata->next;
		int idx = distance_limited(elbg->points + tempdata->index*elbg->dim,
						   newcentroid[0], elbg->dim, INT_MAX) >
				  distance_limited(elbg->points + tempdata->index*elbg->dim,
						   newcentroid[1], elbg->dim, INT_MAX);

		tempdata->next = elbg->cells[indexes[idx]];
		elbg->cells[indexes[idx]] = tempdata;
		tempdata = tempcell2;
	}
}

static void evaluate_utility_inc(elbg_data *elbg)
{
	int i, inc=0;

	if(elbg->error < 0)
		printf("Error overflow?\n");

	for (i=0; i < elbg->numCB; i++) {
		if (elbg->numCB*elbg->utility[i] > elbg->error)
			inc += elbg->utility[i];
		elbg->utility_inc[i] = inc;
		if(inc < 0)
			printf("Uh oh\n");
	}
}


static void update_utility_and_n_cb(elbg_data *elbg, int idx, int newutility)
{
	cell *tempcell;

	elbg->utility[idx] = newutility;
	for (tempcell=elbg->cells[idx]; tempcell; tempcell=tempcell->next)
		elbg->nearest_cb[tempcell->index] = idx;
}

/**
 * Evaluate if a shift lower the error. If it does, call shift_codebooks
 * and update elbg->error, elbg->utility and elbg->nearest_cb.
 *
 * @param elbg  Internal elbg data
 * @param indexes	  {luc (low utility cell, huc (high utility cell), cluc (closest cell to low utility cell)}
 */
static void try_shift_candidate(elbg_data *elbg, int idx[3])
{
	int j, k, cont=0;
	I64_t olderror=0;
	I64_t newerror;
	int newutility[3];
	int newcentroid[3][ELBG_MAX_DIM];
	int *newcentroid_ptrs[3] = { newcentroid[0], newcentroid[1], newcentroid[2] };
	cell *tempcell;

	for (j=0; j<3; j++)
		olderror += elbg->utility[idx[j]];

	memset(newcentroid[2], 0, elbg->dim*sizeof(int));

	for (k=0; k<2; k++)
		for (tempcell=elbg->cells[idx[2*k]]; tempcell; tempcell=tempcell->next) {
			cont++;
			for (j=0; j<elbg->dim; j++)
				newcentroid[2][j] += elbg->points[tempcell->index*elbg->dim + j];
		}

	vect_division(newcentroid[2], newcentroid[2], cont, elbg->dim);

	get_new_centroids(elbg, idx[1], newcentroid[0], newcentroid[1]);

	newutility[2]  = eval_error_cell(elbg, newcentroid[2], elbg->cells[idx[0]]);
	newutility[2] += eval_error_cell(elbg, newcentroid[2], elbg->cells[idx[2]]);

	newerror = newutility[2];

	newerror += simple_lbg(elbg->dim, newcentroid_ptrs, newutility, elbg->points,
						   elbg->cells[idx[1]]);

	if (olderror > newerror) {
		shift_codebook(elbg, idx, newcentroid_ptrs);

		elbg->error += newerror - olderror;
		if(elbg->error < 0)
			printf("Error overflowed\n");

		for (j=0; j<3; j++)
			update_utility_and_n_cb(elbg, idx[j], newutility[j]);

		evaluate_utility_inc(elbg);
	}
 }

/**
 * Implementation of the ELBG block
 */
static void do_shiftings(elbg_data *elbg)
{
	int idx[3];

	evaluate_utility_inc(elbg);

	for (idx[0]=0; idx[0] < elbg->numCB; idx[0]++)
		if (elbg->numCB*elbg->utility[idx[0]] < elbg->error) {
			if (elbg->utility_inc[elbg->numCB-1] == 0)
				return;

			int highestUtil = elbg->utility_inc[elbg->numCB-1];

			idx[1] = get_high_utility_cell(elbg);
			idx[2] = get_closest_codebook(elbg, idx[0]);

			try_shift_candidate(elbg, idx);
		}
}

#define BIG_PRIME 433494437LL

static int numInits = 0;

void ff_init_elbg(int *points, int dim, int numpoints, int *codebook,
				  int numCB, int max_steps, int *closest_cb,
				  AVRandomState *rand_state)
{
	int i, k;

	numInits++;

	if (numpoints > 24*numCB) {
		/* ELBG is very costly for a big number of points. So if we have a lot
		   of them, get a good initial codebook to save on iterations	   */
		std::vector<int> temp_points;
		temp_points.resize(dim*(numpoints/8));

		for (i=0; i<numpoints/8; i++) {
			k = (i*BIG_PRIME) % numpoints;
			memcpy(&temp_points[i*dim], points + k*dim, dim*sizeof(int));
		}

		ff_init_elbg(&temp_points[0], dim, numpoints/8, codebook, numCB, 2*max_steps, closest_cb, rand_state);
		ff_do_elbg(&temp_points[0], dim, numpoints/8, codebook, numCB, 2*max_steps, closest_cb, rand_state);
	} else  // If not, initialize the codebook with random positions
		for (i=0; i < numCB; i++)
			memcpy(codebook + i*dim, points + ((i*BIG_PRIME)%numpoints)*dim,
				   dim*sizeof(int));

}

void ff_do_elbg(int *points, int dim, int numpoints, int *codebook,
				int numCB, int max_steps, int *closest_cb,
				AVRandomState *rand_state)
{
	int dist;
	elbg_data elbg_d;
	elbg_data *elbg = &elbg_d;
	int i, j, k, steps=0;
	I64_t last_error;
	cell *free_cells;
	std::vector<int> dist_cb;
	std::vector<int> size_part;
	std::vector<cell> list_buffer;

	dist_cb.resize(numpoints);
	size_part.resize(numCB);
	list_buffer.resize(numpoints);


	elbg->error = INT_MAX;
	elbg->dim = dim;
	elbg->numCB = numCB;
	elbg->codebook = codebook;
	elbg->cells.resize(numCB);
	elbg->utility.resize(numCB);
	elbg->nearest_cb = closest_cb;
	elbg->points = points;
	elbg->utility_inc.resize(numCB);

	elbg->rand_state = rand_state;

	do {
		free_cells = &list_buffer[0];
		last_error = elbg->error;
		steps++;
		memset(&elbg->utility[0], 0, numCB*sizeof(int));
		memset(&elbg->cells[0], 0, numCB*sizeof(cell *));

		elbg->error = 0;

		/* This loop evaluate the actual Voronoi partition. It is the most
		   costly part of the algorithm. */
		for (i=0; i < numpoints; i++) {
			dist_cb[i] = INT_MAX;
			for (k=0; k < elbg->numCB; k++) {
				dist = distance_limited(elbg->points + i*elbg->dim, elbg->codebook + k*elbg->dim, dim, dist_cb[i]);
				if (dist < dist_cb[i]) {
					dist_cb[i] = dist;
					elbg->nearest_cb[i] = k;
				}
			}
			I64_t prevError = elbg->error;
			elbg->error += dist_cb[i];
			if(elbg->error < 0)
			{
				int dcb = dist_cb[i];
				printf("Error overflowed\n");
			}
			elbg->utility[elbg->nearest_cb[i]] += dist_cb[i];
			free_cells->index = i;
			free_cells->next = elbg->cells[elbg->nearest_cb[i]];
			elbg->cells[elbg->nearest_cb[i]] = free_cells;
			free_cells++;
		}

		do_shiftings(elbg);

		memset(&size_part[0], 0, numCB*sizeof(int));

		memset(elbg->codebook, 0, elbg->numCB*dim*sizeof(int));

		for (i=0; i < numpoints; i++) {
			size_part[elbg->nearest_cb[i]]++;
			for (j=0; j < elbg->dim; j++)
				elbg->codebook[elbg->nearest_cb[i]*elbg->dim + j] +=
					elbg->points[i*elbg->dim + j];
		}

		for (i=0; i < elbg->numCB; i++)
			vect_division(elbg->codebook + i*elbg->dim,
						  elbg->codebook + i*elbg->dim, size_part[i], elbg->dim);

	} while(((last_error - elbg->error) > DELTA_ERR_MAX*elbg->error) &&
			(steps < max_steps));
}
