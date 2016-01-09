#ifndef _GSALT_H_
#define _GSALT_H_

#define GSALT_MAJOR 0
#define GSALT_MINOR 1

#include <stdint.h>

typedef enum {
	gsalt_verbose_none = 0,
	gsalt_verbose_error,
	gsalt_verbose_warning,
	gsalt_verbose_debug,
	gsalt_verbose_all
} gsalt_verbose;

typedef int gslat_return;

#define GSALT_OK 0
#define GSALT_ERROR -1

#define GSALT_ENABLE 1
#define GSALT_DISABLE 0

#define GSALT_VERTEX 0
#define GSALT_COLOR 1
#define GSALT_NORMAL 2
#define GSALT_TEXCOORD 4

typedef void* GSalt;

#ifdef __cplusplus
extern "C" {
#endif
// Init of the library
gslat_return gsalt_init();

// Get / set verbose level (default to error)
gsalt_verbose gsalt_get_verbose();
gsalt_verbose gsalt_set_verbose(gsalt_verbose new_level);

// To simplify a strucutre, here is a workflow:
// 1. create a gsalt object with gsalt_start_simplify with number of vertex and num triangles 
//		(only triangles are supported for now, no strip or fans, no quads)
//  	define also if color, normal and texcoord will be passed along (only 1 texture for now, no multitex)
// 2. Either send vertex ( plus color / normal / texcoord) one by one or using GL-style pointer
// 3. Define the triangles (index) or just use automatic one.
// 4. Simplify, giving the number of triangles objective. Gives back the effective number of triangles (or -1 if error)
// 5. read back the triangles (and number of vertex) or just get the number of triangles if using GL-style arrays
// 6. End the simplification to free the memory

GSalt gsalt_new(int num_vertex, int num_triangles, unsigned int flags);
gslat_return gsalt_add_color(GSalt gsalt, float r, float g, float b, float a=1.0f);
gslat_return gsalt_add_normal(GSalt gsalt, float x, float y, float z);
gslat_return gsalt_add_texcoord(GSalt gsalt, float s, float t, float r=0.0f, float q=1.0f);
gslat_return gsalt_add_vertex(GSalt gsalt, float x, float y, float z=0.0f, float w=1.0f);

gslat_return gsalt_add_triangle(GSalt gsalt, int idx1, int idx2, int idx3);

int gsalt_simplify(GSalt gsalt, int objective);

int gsalt_query_numvertex(GSalt gsalt);
int gsalt_query_numtriangles(GSalt gsalt);

gslat_return gsalt_query_color(GSalt gsalt, int index, float *r, float *g, float *b, float *a=NULL);
gslat_return gsalt_query_normal(GSalt gsalt, int index, float *x, float *y, float *z);
gslat_return gsalt_query_texcoord(GSalt gsalt, int index, float *s, float *t, float *r=NULL, float *q=NULL);
gslat_return gsalt_query_vertex(GSalt gsalt, int index, float *x, float *y, float *z=NULL, float *w=NULL);

gslat_return gsalt_query_triangle_uint32(GSalt gsalt, int index, uint32_t *idx1, uint32_t *idx2, uint32_t *idx3);
gslat_return gsalt_query_triangle_uint16(GSalt gsalt, int index, uint16_t *idx1, uint16_t *idx2, uint16_t *idx3);

gslat_return gsalt_delete(GSalt gsalt);
#ifdef __cplusplus
}
#endif

#endif //_GSALT_H_