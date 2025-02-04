
/**
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */
#include "functions.h"


void interp3D(Grid *n, Grid *o)
{
	myfloat tx[n->imax+2][o->jmax+2][o->kmax+2];
	myfloat txy[n->imax+2][n->jmax+2][o->kmax+2];

	//start the grid loop
	for(int j = 1; j < o->jmax+1; j++)
	{
		for(int k = 1; k < o->kmax+1; k++)
		{
			myfloat temp[o->imax+2],temp2[o->imax+2];
			for(int i = 1; i < o->imax+1; i++)
			{
				temp[i] = o->T[idx_T(i,j,k,o->imax,o->jmax)];
			}
			spline(o->x, temp, o->imax, 1.e30, 1.0e30, temp2 );
			for(int i = 1; i < n->imax+1; i++)
			{
					tx[i][j][k] = splint(o->x,temp,temp2,o->imax,n->x[i]);
			}
		}
	}

	 
	for(int i = 1; i < n->imax+1; i++)
	{
		for(int k = 1; k < o->kmax+1; k++)
		{
			myfloat temp[o->jmax+2],temp2[o->jmax+2];
			for(int j = 1; j < o->jmax+1; j++)
			{
				temp[j] = tx[i][j][k];
			}
			spline(o->y, temp, o->jmax, 1e30, 1e30, temp2);
			for(int j = 1; j < n->jmax+1; j++)
			{
				txy[i][j][k] = splint(o->y,temp,temp2,o->jmax,n->y[j]);
			}
		}
	}


	for(int i = 1; i < n->imax+1; i++)
	{
		for(int j = 1; j < n->jmax+1; j++)
		{
			myfloat temp[o->kmax+2],temp2[o->kmax+2];
			for(int k = 1; k < o->kmax+1; k++)
			{
				temp[k] = txy[i][j][k];
			}
			spline(o->z, temp, o->kmax, 1e30, 1e30, temp2);
			for(int k = 1; k < n->kmax+1; k++)
			{
				n->T[idx_T(i,j,k,n->imax,n->jmax)] = splint(o->z,temp,temp2,o->kmax,n->z[k]);
			}
		}
	}


	return;
}


void spline(myfloat x[], myfloat y[], int n, myfloat yp1, myfloat ypn, myfloat y2[])
{
	int i,j,k;
	myfloat p,qn,sig,un,*u;
	u=vector12(1,n-1);
	if (yp1 > 0.99e30)
		y2[1]=u[1]=0.0;
	else {
		y2[1] = -0.5;
		u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
	}
	for (i=2;i<=n-1;i++) {
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30)
		qn=un=0.0;
	else {
		qn=0.5;
		un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
	}
	y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
	for (k=n-1;k>=1;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
	free_vector(u,1,n-1);
}

myfloat splint(myfloat xa[], myfloat ya[], myfloat y2a[], int n, myfloat x)
{
	int klo,khi,k;
	myfloat h,b,a,y;
	klo=1;
	khi=n;
	while (khi-klo > 1) {
		k=(khi+klo) >> 1;
		if (xa[k] > x) khi=k;
		else klo=k;
	}
	h=xa[khi]-xa[klo];
	if (h == 0.0){
		fprintf(stderr,"Bad xa input to routine splint");
		fprintf(stderr,"...now exiting to system...\n");
		exit(1);
	}
	a=(xa[khi]-x)/h;
	b=(x-xa[klo])/h;
	y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
	return y;
}

myfloat *vector12(long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh] */
{
	myfloat *v;

	v=(myfloat *)malloc((size_t) ((nh-nl+1+1)*sizeof(myfloat)));
	if (!v){
		fprintf(stderr,"allocation failure in vector()");
		fprintf(stderr,"...now exiting to system...\n");
		exit(1);
	}
	return v-nl+1;
}
void free_vector(myfloat *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
	free((char*) (v+nl-1));
}
