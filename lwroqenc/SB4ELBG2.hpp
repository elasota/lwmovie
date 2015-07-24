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

#ifndef __SB4ELBG2_HPP__
#define __SB4ELBG2_HPP__

#include "SB4Random.hpp"

#define SB4_ELBG_MAX_DIM	256

/**
* Implementation of the Enhanced LBG Algorithm
* Based on the paper "Neural Networks 14:1219-1237" that can be found in
* http://citeseer.ist.psu.edu/patan01enhanced.html .
*
* @param points Input points.
* @param dim Dimension of the points.
* @param numpoints Num of points in **points.
* @param codebook Pointer to the output codebook. Must be allocated.
* @param numCB Number of points in the codebook.
* @param num_steps The maximum number of steps. One step is already a good compromise between time and quality.
* @param closest_cb Return the closest codebook to each point. Must be allocated.
* @param rand_state A random number generator state. Should be already initialised by av_init_random.
*/
void ff_do_elbg(int *points, int dim, int numpoints, int *codebook,
	int numCB, int num_steps, int *closest_cb,
	SB4RandomState *rand_state);

/**
* Initialize the **codebook vector for the elbg algorithm. If you have already
* a codebook and you want to refine it, you shouldn't call this function.
* If numpoints < 8*numCB this function fills **codebook with random numbers.
* If not, it calls ff_do_elbg for a (smaller) random sample of the points in
* **points. Get the same parameters as ff_do_elbg.
*/
void ff_init_elbg(int *points, int dim, int numpoints, int *codebook,
	int numCB, int num_steps, int *closest_cb,
	SB4RandomState *rand_state);


#endif