#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <gsalt/gsalt.h>
#include "qslim/MxQSlim.h"
#include "qslim/MxPropSlim.h"


gsalt_verbose verbose_level = gsalt_verbose_warning;
char const* verbose_string[] = {"None", "Error", "Warning", "Debug", "All"};

#define GSALT_ALLFLAGS GSALT_VERTEX|GSALT_COLOR|GSALT_NORMAL|GSALT_TEXCOORD|GSALT_EDGE|GSALT_FACE


int gsalt_inited = 0;

typedef struct {
	float* ptr;
	int size;
	int stride;
	int local;
} fpointer;

typedef	union {
		uint16_t *ui16;
		uint32_t *ui32;
		void	 *ptr;
} multiptr;

typedef struct {
	multiptr ptr;
	int size;
	int stride;
	int local;
	int type;
} spointer;

#define SIGN 0x72730103

typedef struct {
	int signature;
	int num_vertex;
	int num_triangles;
	int decimed_vertex;
	int decimed_triangles;
	int faces_defined;
	unsigned int flags;

	MxStdModel *model;

	fpointer vertex;
	fpointer color;
	fpointer normal;
	fpointer texcoord;

	spointer indexes;

} GSalt_t, *PGSalt;

inline void init_pointer(fpointer* p, float* ptr, int size, int stride, int local)
{
	p->ptr = ptr;
	p->size = size;
	if (stride)
		p->stride = stride;
	else
		p->stride = size;
	p->local = local;
}
inline void init_pointer(spointer* p, void* ptr, int size, int stride, int local, int type)
{
	if(type)
		p->ptr.ui16 = (uint16_t*)ptr;
	else
		p->ptr.ui32 = (uint32_t*)ptr;
	p->size = size;
	if (stride)
		p->stride = stride;
	else
		p->stride = size;
	p->local = local;
	p->type = type;
}

void gsalt_log(gsalt_verbose level, const char *fmt, ...)
{
	if (level <= verbose_level) {
		va_list argptr;
		va_start(argptr, fmt);
    	vprintf(fmt, argptr);
    	va_end(argptr);
	}
}

gslat_return gsalt_init() {
	if (gsalt_inited)
		return GSALT_OK;

	gsalt_log(gsalt_verbose_none, "GSalt, the Geometry Simplification At Load Time library, version %d.%d by ptitSeb\n", GSALT_MAJOR, GSALT_MINOR);

	verbose_level = gsalt_verbose_warning;

	const char* env = getenv("GSALT_DEBUG");
	if (env) {
		if (env[0]=='1')
			verbose_level = gsalt_verbose_debug;
		else if (env[0]=='2')
			verbose_level = gsalt_verbose_all;
		else if (env[0]=='0')
			verbose_level = gsalt_verbose_none;
	}

	gsalt_inited = 1;

	return GSALT_OK;
}

gsalt_verbose gsalt_get_verbose() {
	gsalt_log(gsalt_verbose_debug, "GSalt: Verbose level = %s", verbose_string[verbose_level]);
	return verbose_level;
}

gsalt_verbose gsalt_set_verbose(gsalt_verbose new_level) {
	gsalt_verbose old = verbose_level;
	verbose_level = new_level;
	gsalt_log(gsalt_verbose_debug, "GSalt: Change Verbose level from %s to %s", verbose_string[old], verbose_string[verbose_level]);
	return old;
}

GSalt gsalt_new(int num_vertex, int num_triangles, unsigned int flags) {
	if (!gsalt_inited) gsalt_init();
	gsalt_log(gsalt_verbose_debug, "GSalt: New Gsalt object, %d vertex, %d triangles, flags = %x\n", num_vertex, num_triangles, flags);
	if (num_vertex<0) {
		gsalt_log(gsalt_verbose_error, "GSalt: negative number of vertex (%d)\n", num_vertex);
		return NULL;
	}
	if (num_triangles<0) {
		gsalt_log(gsalt_verbose_error, "GSalt: negative number of triangles (%d)\n", num_triangles);
		return NULL;
	}
	if (flags&~(GSALT_ALLFLAGS)) {
		gsalt_log(gsalt_verbose_warning, "GSalt: flags = %x contains unknown flag\n", num_vertex, num_triangles, flags);
		flags&=(GSALT_ALLFLAGS);
	}
	PGSalt pgsalt = (PGSalt)malloc(sizeof(GSalt_t));

	pgsalt->num_vertex = num_vertex;
	pgsalt->num_triangles = num_triangles;
	pgsalt->flags = flags;

	pgsalt->faces_defined = 0;
	pgsalt->decimed_vertex = 0;
	pgsalt->decimed_triangles = 0;

	init_pointer(&pgsalt->vertex, NULL, 4, 0, 1);
	init_pointer(&pgsalt->normal, NULL, 3, 0, 1);
	init_pointer(&pgsalt->color, NULL, 4, 0, 1);
	init_pointer(&pgsalt->texcoord, NULL, 4, 0, 1);

	init_pointer(&pgsalt->indexes, NULL, 1, 0, 1, GSALT_UINT32);

	pgsalt->model = new MxStdModel(num_vertex, num_triangles);

	pgsalt->model->color_binding((pgsalt->flags&GSALT_COLOR)?MX_PERVERTEX:MX_UNBOUND);
	pgsalt->model->normal_binding((pgsalt->flags&GSALT_NORMAL)?MX_PERVERTEX:MX_UNBOUND);
	pgsalt->model->texcoord_binding((pgsalt->flags&GSALT_TEXCOORD)?MX_PERVERTEX:MX_UNBOUND);

	pgsalt->signature = SIGN;

	return (GSalt)pgsalt;
}

#define check_gsalt \
		PGSalt pgsalt = (PGSalt)gsalt;  \
		if(pgsalt->signature!=SIGN) {gsalt_log(gsalt_verbose_error, "GSalt: GSalt object is not valid\n"); return GSALT_ERROR;}

gslat_return gsalt_delete(GSalt gsalt) {
	gsalt_log(gsalt_verbose_debug, "GSalt: Delete GSalt object\n");

	check_gsalt;

	if(pgsalt->vertex.local) free(pgsalt->vertex.ptr);
	if(pgsalt->color.local) free(pgsalt->color.ptr);
	if(pgsalt->normal.local) free(pgsalt->normal.ptr);
	if(pgsalt->texcoord.local) free(pgsalt->texcoord.ptr);

	if(pgsalt->indexes.local) free(pgsalt->indexes.ptr.ui32);

	if(pgsalt->model) delete pgsalt->model;

	pgsalt->signature = 0x0;

	free(pgsalt);

	return GSALT_OK;
}

gslat_return gsalt_add_color(GSalt gsalt, float r, float g, float b, float a) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: add a color vertex(%f, %f, %f, %f)\n", r, g, b, a);

	pgsalt->model->add_color(r, g, b, a);

	return GSALT_OK;
}
gslat_return gsalt_add_normal(GSalt gsalt, float x, float y, float z) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: add a normal(%f, %f, %f)\n", x, y, z);

	pgsalt->model->add_normal(x, y, z);

	return GSALT_OK;
}
gslat_return gsalt_add_texcoord(GSalt gsalt, float s, float t, float r, float q) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: add a texcoord(%f, %f)\n", s, t);

	pgsalt->model->add_texcoord(s, t);

	return GSALT_OK;
}
gslat_return gsalt_add_vertex(GSalt gsalt, float x, float y, float z, float w) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: add a vertex(%f, %f, %f)\n", x, y, z);

	pgsalt->model->add_vertex(x, y, z);

	return GSALT_OK;
}

gslat_return gsalt_add_triangle(GSalt gsalt, int idx1, int idx2, int idx3) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: add a triangle(%d, %d, %d)\n", idx1, idx2, idx3);

	pgsalt->model->add_face(idx1, idx2, idx3);

	pgsalt->faces_defined++;

	return GSALT_OK;
}

int gsalt_simplify(GSalt gsalt, int objective) {
	check_gsalt;
	gsalt_log(gsalt_verbose_debug, "GSalt: Simplify, objective=%d\n", objective);

	if(objective<3) {
		gsalt_log(gsalt_verbose_warning, "GSalt: Simplify, objective too low(%d) !\n", objective);		
		return GSALT_ERROR;
	}

	if(pgsalt->faces_defined==0) {
		gsalt_log(gsalt_verbose_debug, "GSalt: create a dummy triangle list\n");
		for (int i=0; i<pgsalt->num_triangles; i++)
			pgsalt->model->add_face(i*3+0, i*3+1, i*3+2);
	}
	MxStdSlim *slim;
	if (pgsalt->flags&GSALT_FACE) {
		gsalt_log(gsalt_verbose_debug, "GSalt: Simplify using %s strategy\n", "Face");
		slim = new MxFaceQSlim(*pgsalt->model);
	} else if (pgsalt->flags&GSALT_EDGE) {
		gsalt_log(gsalt_verbose_debug, "GSalt: Simplify using %s strategy\n", "Edge");
		slim = new MxEdgeQSlim(*pgsalt->model);
	} else {
		gsalt_log(gsalt_verbose_debug, "GSalt: Simplify using %s strategy\n", "Prop");
		slim = new MxPropSlim(*pgsalt->model);
	}
	slim->initialize();
	slim->decimate(objective);
	// now, get back the values in the arrays
#define alloc_ptr(A) pgsalt->A.ptr = (float*)realloc(pgsalt->A.ptr, sizeof(float)*pgsalt->num_vertex*pgsalt->A.size);
	if(pgsalt->vertex.local) {
		alloc_ptr(vertex);
	}
	if((pgsalt->flags&GSALT_COLOR) && (pgsalt->color.local)) {
		alloc_ptr(color);		
	}
	if((pgsalt->flags&GSALT_NORMAL) && (pgsalt->normal.local)) {
		alloc_ptr(normal);		
	}
	if((pgsalt->flags&GSALT_TEXCOORD) && (pgsalt->texcoord.local)) {
		alloc_ptr(texcoord);		
	}
#undef alloc_ptr
	if(pgsalt->indexes.local) {
		if(pgsalt->indexes.type)
			pgsalt->indexes.ptr.ui16 = (uint16_t*)realloc(pgsalt->indexes.ptr.ui16, sizeof(uint16_t)*pgsalt->num_triangles*3*pgsalt->indexes.size);
		else
			pgsalt->indexes.ptr.ui32 = (uint32_t*)realloc(pgsalt->indexes.ptr.ui32, sizeof(uint32_t)*pgsalt->num_triangles*3*pgsalt->indexes.size);
	}

	// now, count the new number of vertex and triangles
	unsigned int max_vertex = pgsalt->model->vert_count();
	unsigned int max_faces = pgsalt->model->face_count();

	int *match;
	match = (int*)malloc(sizeof(int)*max_vertex);

	float *vertex, *color, *normal, *texcoord;
	uint16_t *ind16;
	uint32_t *ind32;

	vertex = pgsalt->vertex.ptr;
	color = (pgsalt->flags&GSALT_COLOR)?pgsalt->color.ptr:NULL;
	normal = (pgsalt->flags&GSALT_NORMAL)?pgsalt->normal.ptr:NULL;
	texcoord = 	(pgsalt->flags&GSALT_TEXCOORD)?pgsalt->texcoord.ptr:NULL;

	pgsalt->decimed_vertex = 0;
	pgsalt->decimed_triangles = 0;

	for (unsigned int i=0; i<max_vertex; i++) {
		if (pgsalt->model->vertex_is_valid(i) && (pgsalt->decimed_vertex<pgsalt->num_vertex)) {
			vertex[0] = pgsalt->model->vertex(i).as.pos[0]; vertex[1] = pgsalt->model->vertex(i).as.pos[1]; vertex[2] = pgsalt->model->vertex(i).as.pos[2];
			if(pgsalt->vertex.size>3) vertex[3] = 1.0f;
			vertex += pgsalt->vertex.stride;
			if(color) {
				color[0] = pgsalt->model->color(i).R();
				color[1] = pgsalt->model->color(i).G();
				color[2] = pgsalt->model->color(i).B();
				if(pgsalt->color.size>3)
					color[3] = pgsalt->model->color(i).A();
				color += pgsalt->color.stride;
			}
			if(normal) {
				normal[0] = pgsalt->model->normal(i)[0]; normal[1] = pgsalt->model->normal(i)[1]; normal[2] = pgsalt->model->normal(i)[2];
				normal += pgsalt->normal.stride;
			}
			if(texcoord) {
				texcoord[0] = pgsalt->model->texcoord(i).u[0]; texcoord[1] = pgsalt->model->texcoord(i).u[1];
				if(pgsalt->texcoord.size>2) texcoord[2] = 0.0f;
				if(pgsalt->texcoord.size>3) texcoord[3] = 1.0f;
				texcoord += pgsalt->texcoord.stride;
			}
			match[i]=pgsalt->decimed_vertex++;
		}
	}
	int newFaces = 0;
	if(pgsalt->indexes.type) {
		ind16 = pgsalt->indexes.ptr.ui16;
		for (int i=0; i<max_faces; i++) {
			if (pgsalt->model->face_is_valid(i)) {
				*(ind16++)=match[pgsalt->model->face(i).v[0]];
				*(ind16++)=match[pgsalt->model->face(i).v[1]];
				*(ind16++)=match[pgsalt->model->face(i).v[2]];
				newFaces++;
			}
		}
	} else {
		ind32 = pgsalt->indexes.ptr.ui32;
		for (int i=0; i<pgsalt->num_triangles; i++) {
			if (pgsalt->model->face_is_valid(i)) {
				*(ind32++)=match[pgsalt->model->face(i).v[0]];
				*(ind32++)=match[pgsalt->model->face(i).v[1]];
				*(ind32++)=match[pgsalt->model->face(i).v[2]];
				newFaces++;
			}
		}
	}

	pgsalt->decimed_triangles=newFaces;

	free(match);

	if(!pgsalt->faces_defined) {
		gsalt_log(gsalt_verbose_debug, "GSalt: no indexes, flatening the vertex list\n");
		// indexes is not used here, so put the vertex "flat" and discard indexes array
#define alloc(A,B) if((pgsalt->flags & B)==B) { \
			A=(float*)malloc(sizeof(float)*pgsalt->num_vertex*pgsalt->A.stride);	\
			memcpy(A, pgsalt->A.ptr, sizeof(float)*pgsalt->num_vertex*pgsalt->A.stride);	\
		} else A=NULL
		alloc(vertex, GSALT_VERTEX);
		alloc(color, GSALT_COLOR);
		alloc(texcoord, GSALT_TEXCOORD);
		alloc(normal, GSALT_NORMAL);
#undef alloc
		for (int i=0; i<pgsalt->decimed_triangles*3; i++) {
			int idx = (pgsalt->indexes.type)?pgsalt->indexes.ptr.ui16[i]:pgsalt->indexes.ptr.ui32[i];
			#define copy(A) if(A) memcpy(pgsalt->A.ptr+i*pgsalt->A.stride, A+idx*pgsalt->A.stride, sizeof(float)*pgsalt->A.size);
			copy(vertex);
			copy(normal);
			copy(texcoord);
			copy(color);
			#undef copy
		}
		free(vertex);
		free(color);
		free(normal);
		free(texcoord);
		pgsalt->decimed_vertex = pgsalt->decimed_triangles*3;
		free(pgsalt->indexes.ptr.ptr);
		pgsalt->indexes.ptr.ptr=NULL;
	}
	gsalt_log(gsalt_verbose_warning, "GSalt: Simplified from %d(%d) to %d(%d)\n", 
		pgsalt->num_vertex, pgsalt->num_triangles, pgsalt->decimed_vertex, pgsalt->decimed_triangles);

	delete slim;

	if (pgsalt->decimed_vertex > pgsalt->num_vertex) {
		gsalt_log(gsalt_verbose_error, "GSalt: Simplified failed, number of vertex increased\n");
		pgsalt->decimed_vertex = 0;
		pgsalt->decimed_triangles = 0;
		free(match);
		return GSALT_OK;
	}
	if (pgsalt->decimed_triangles > pgsalt->num_triangles) {
		gsalt_log(gsalt_verbose_error, "GSalt: Simplified failed, number of triangles increased\n");
		pgsalt->decimed_vertex = 0;
		pgsalt->decimed_triangles = 0;
		free(match);
		return GSALT_OK;
	}
	if (!(pgsalt->faces_defined) && (pgsalt->decimed_triangles*3 > pgsalt->num_vertex)) {
		gsalt_log(gsalt_verbose_error, "GSalt: Simplified failed, number of vertex increased\n");
		pgsalt->decimed_vertex = 0;
		pgsalt->decimed_triangles = 0;
		free(match);
		return GSALT_OK;
	}

	return newFaces;
}

int gsalt_query_numvertex(GSalt gsalt) {
	check_gsalt;
	gsalt_log(gsalt_verbose_debug, "GSalt: query numvertex\n");

	gsalt_log(gsalt_verbose_debug, "GSalt: num_vertex = %d, decimed_vertex = %d\n", pgsalt->num_vertex, pgsalt->decimed_vertex);

	return (pgsalt->decimed_vertex)?pgsalt->decimed_vertex:pgsalt->num_vertex;
}

int gsalt_query_numtriangles(GSalt gsalt) {
	check_gsalt;
	gsalt_log(gsalt_verbose_debug, "GSalt: query numtriangles\n");

	gsalt_log(gsalt_verbose_debug, "GSalt: num_triangles = %d, decimed_triangles = %d\n", pgsalt->num_triangles, pgsalt->decimed_triangles);

	return (pgsalt->decimed_triangles)?pgsalt->decimed_triangles:pgsalt->num_triangles;
}

gslat_return gsalt_query_color(GSalt gsalt, int index, float *r, float *g, float *b, float *a) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: query color(%d)\n", index);

	if(!(pgsalt->flags&GSALT_COLOR)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query color but color is not activated\n");		
		return GSALT_ERROR;
	}
	if (index<0 || index>((pgsalt->decimed_vertex)?pgsalt->decimed_vertex:pgsalt->num_vertex)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query color index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_vertex) {
		float* color = pgsalt->color.ptr+index*pgsalt->color.stride;
		if(r) *r=color[0];
		if(g) *g=color[1];
		if(b) *b=color[2];
		if(a) *a=color[3];
	} else {
		if(r) *r=pgsalt->model->color(index).R();
		if(g) *g=pgsalt->model->color(index).G();
		if(b) *b=pgsalt->model->color(index).B();
		if(a) *a=pgsalt->model->color(index).A();
	}
	return GSALT_OK;
}
gslat_return gsalt_query_normal(GSalt gsalt, int index, float *x, float *y, float *z)  {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: query normal(%d)\n", index);

	if(!(pgsalt->flags&GSALT_NORMAL)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query normal but normal is not activated\n");		
		return GSALT_ERROR;
	}
	if (index<0 || index>((pgsalt->decimed_vertex)?pgsalt->decimed_vertex:pgsalt->num_vertex)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query normal index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_vertex) {
		float* normal = pgsalt->normal.ptr+index*pgsalt->normal.stride;
		if(x) *x=normal[0];
		if(y) *y=normal[1];
		if(z) *z=normal[2];
	} else {
		if(x) *x=pgsalt->model->normal(index)[0];
		if(y) *y=pgsalt->model->normal(index)[1];
		if(z) *z=pgsalt->model->normal(index)[2];
	}
	return GSALT_OK;
}
gslat_return gsalt_query_texcoord(GSalt gsalt, int index, float *s, float *t, float *r, float *q) {
	check_gsalt;
	gsalt_log(gsalt_verbose_all, "GSalt: query texcoord(%d)\n", index);

	if(!(pgsalt->flags&GSALT_TEXCOORD)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query texcoord but texcoord is not activated\n");		
		return GSALT_ERROR;
	}
	if (index<0 || index>((pgsalt->decimed_vertex)?pgsalt->decimed_vertex:pgsalt->num_vertex)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query texcoord index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_vertex) {
		float* texcoord = pgsalt->texcoord.ptr+index*pgsalt->texcoord.stride;
		if(s) *s=texcoord[0];
		if(t) *t=texcoord[1];
		if(r) *r=0.0f;
		if(q) *q=1.0f;
	} else {
		if(s) *s=pgsalt->model->texcoord(index).u[0];
		if(t) *t=pgsalt->model->texcoord(index).u[1];
		if(r) *r=0.0f;
		if(q) *q=1.0f;
	}
	return GSALT_OK;
}
gslat_return gsalt_query_vertex(GSalt gsalt, int index, float *x, float *y, float *z, float *w) {
	check_gsalt;

	if (index<0 || index>((pgsalt->decimed_vertex)?pgsalt->decimed_vertex:pgsalt->num_vertex)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query vertex index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_vertex) {
		float* vertex = pgsalt->vertex.ptr+index*pgsalt->vertex.stride;
		if(x) *x=vertex[0];
		if(y) *y=vertex[1];
		if(z) *z=vertex[2];
		if(w) *w=1.0f;
	} else {
		if(x) *x=pgsalt->model->vertex(index).as.pos[0];
		if(y) *y=pgsalt->model->vertex(index).as.pos[1];
		if(z) *z=pgsalt->model->vertex(index).as.pos[2];
		if(w) *w=1.0f;
	}
	gsalt_log(gsalt_verbose_all, "GSalt: query vertex(%d) ->(%f, %f, %f)\n", index, *x, *y, *z);
	return GSALT_OK;
}

gslat_return gsalt_query_triangle_uint32(GSalt gsalt, int index, uint32_t *idx1, uint32_t *idx2, uint32_t *idx3) {
	check_gsalt;
	if(!(pgsalt->faces_defined)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query triangle index but texcoord is not activated\n");		
		return GSALT_ERROR;
	}

	if (index<0 || index>((pgsalt->decimed_triangles)?pgsalt->decimed_triangles:pgsalt->num_triangles)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query triangle index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_triangles) {
		if(pgsalt->indexes.type) {
			uint16_t* triangle = pgsalt->indexes.ptr.ui16+index*3*pgsalt->indexes.stride;
			if(idx1) *idx1=triangle[0];
			if(idx2) *idx2=triangle[1];
			if(idx3) *idx3=triangle[2];
		} else {
			uint32_t* triangle = pgsalt->indexes.ptr.ui32+index*3*pgsalt->indexes.stride;
			if(idx1) *idx1=triangle[0];
			if(idx2) *idx2=triangle[1];
			if(idx3) *idx3=triangle[2];
		}
	} else {
		if(idx1) *idx1=pgsalt->model->face(index).v[0];
		if(idx2) *idx2=pgsalt->model->face(index).v[1];
		if(idx3) *idx3=pgsalt->model->face(index).v[2];
	}
	gsalt_log(gsalt_verbose_all, "GSalt: query triangle uint32_t (%d) -> (%d, %d, %d)\n", index, *idx1, *idx2, *idx3);
	return GSALT_OK;
}

gslat_return gsalt_query_triangle_uint16(GSalt gsalt, int index, uint16_t *idx1, uint16_t *idx2, uint16_t *idx3)  {
	check_gsalt;
	if(!(pgsalt->faces_defined)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query triangle index but texcoord is not activated\n");		
		return GSALT_ERROR;
	}

	if (index<0 || index>((pgsalt->decimed_triangles)?pgsalt->decimed_triangles:pgsalt->num_triangles)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: query triangle index out of range\n");		
		return GSALT_ERROR;
	}

	if(pgsalt->decimed_triangles) {
		if(pgsalt->indexes.type) {
			uint16_t* triangle = pgsalt->indexes.ptr.ui16+index*3*pgsalt->indexes.stride;
			if(idx1) *idx1=triangle[0];
			if(idx2) *idx2=triangle[1];
			if(idx3) *idx3=triangle[2];
		} else {
			uint32_t* triangle = pgsalt->indexes.ptr.ui32+index*3*pgsalt->indexes.stride;
			if(idx1) *idx1=triangle[0];
			if(idx2) *idx2=triangle[1];
			if(idx3) *idx3=triangle[2];
		}
	} else {
		if(idx1) *idx1=pgsalt->model->face(index).v[0];
		if(idx2) *idx2=pgsalt->model->face(index).v[1];
		if(idx3) *idx3=pgsalt->model->face(index).v[2];
	}
	gsalt_log(gsalt_verbose_all, "GSalt: query triangle uint16_t (%d) -> (%d, %d, %d)\n", index, *idx1, *idx2, *idx3);
	return GSALT_OK;
}

// v0.2 api
gslat_return gsalt_array_vertex(GSalt gsalt, int type, int size, int stride, void* pointer)
{
	if (type!=GSALT_FLOAT) {
		gsalt_log(gsalt_verbose_error, "GSalt: Array vertex only support FLOAT (type=%d)\n", type);
		return GSALT_ERROR;
	}
	check_gsalt;

	gsalt_log(gsalt_verbose_debug, "GSalt: Array vertex defined (%s, %d, %d)\n", "FLOAT", size, stride);
	init_pointer(&pgsalt->vertex, (float*)pointer, size, stride, 0);

	float x=0, y=0, z=0/*, w=1*/;
	int sz = pgsalt->vertex.size;
	int width = pgsalt->vertex.stride;
	float* array = pgsalt->vertex.ptr;
	for (int i=0; i<pgsalt->num_vertex; i++) {
		if (sz>0) x = array[0];
		if (sz>1) y = array[1];
		if (sz>2) z = array[2];
		//if (sz>3) w = array[3];
		pgsalt->model->add_vertex(x, y, z);
		array+=width;
	}
}

gslat_return gsalt_array_normal(GSalt gsalt, int type, int stride, void* pointer) {
	if (type!=GSALT_FLOAT) {
		gsalt_log(gsalt_verbose_error, "GSalt: Array normal only support FLOAT (type=%d)\n", type);
		return GSALT_ERROR;
	}
	check_gsalt;
	if(!(pgsalt->flags&GSALT_NORMAL)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: Setting Array normal but normal is not activated\n");		
		return GSALT_ERROR;
	}

	gsalt_log(gsalt_verbose_debug, "GSalt: Array normal defined (%s, %d)\n", "FLOAT", stride);
	init_pointer(&pgsalt->normal, (float*)pointer, 3, stride, 0);

	float x=0, y=0, z=0;
	int sz = pgsalt->normal.size;
	int width = pgsalt->normal.stride;
	float* array = pgsalt->normal.ptr;
	for (int i=0; i<pgsalt->num_vertex; i++) {
		if (sz>0) x = array[0];
		if (sz>1) y = array[1];
		if (sz>2) z = array[2];
		pgsalt->model->add_normal(x, y, z);
		array+=width;
	}
}

gslat_return gsalt_array_color(GSalt gsalt, int type, int size, int stride, void* pointer) {
	if (type!=GSALT_FLOAT) {
		gsalt_log(gsalt_verbose_error, "GSalt: Array color only support FLOAT (type=%d)\n", type);
		return GSALT_ERROR;
	}
	check_gsalt;
	if(!(pgsalt->flags&GSALT_COLOR)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: Setting Array color but color is not activated\n");		
		return GSALT_ERROR;
	}

	gsalt_log(gsalt_verbose_debug, "GSalt: Array color defined (%s, %d, %d)\n", "FLOAT", size, stride);
	init_pointer(&pgsalt->color, (float*)pointer, size, stride, 0);

	float r=0, g=0, b=0, a=1;
	int sz = pgsalt->color.size;
	int width = pgsalt->color.stride;
	float* array = pgsalt->color.ptr;
	for (int i=0; i<pgsalt->num_vertex; i++) {
		if (sz>0) r = array[0];
		if (sz>1) g = array[1];
		if (sz>2) b = array[2];
		if (sz>3) a = array[3];
		pgsalt->model->add_color(r, g, b, a);
		array+=width;
	}
}

gslat_return gsalt_array_texcoord(GSalt gsalt, int type, int size, int stride, void* pointer) {
	if (type!=GSALT_FLOAT) {
		gsalt_log(gsalt_verbose_error, "GSalt: Array texcoord only support FLOAT (type=%d)\n", type);
		return GSALT_ERROR;
	}
	check_gsalt;
	if(!(pgsalt->flags&GSALT_TEXCOORD)) {
		gsalt_log(gsalt_verbose_debug, "GSalt: Setting Array texcoord but texcoord is not activated\n");		
		return GSALT_ERROR;
	}

	gsalt_log(gsalt_verbose_debug, "GSalt: Array texcoord defined (%s, %d, %d)\n", "FLOAT", size, stride);
	init_pointer(&pgsalt->texcoord, (float*)pointer, size, stride, 0);

	float s=0, t=0/*, r=0, q=1*/;
	int sz = pgsalt->texcoord.size;
	int width = pgsalt->texcoord.stride;
	float* array = pgsalt->texcoord.ptr;
	for (int i=0; i<pgsalt->num_vertex; i++) {
		if (sz>0) s = array[0];
		if (sz>1) t = array[1];
		/*if (sz>2) r = array[2];
		if (sz>3) q = array[3];*/
		pgsalt->model->add_texcoord(s, t);
		array+=width;
	}
}

gslat_return gsalt_array_indexes(GSalt gsalt, int type, void* pointer) {
	if ((type!=GSALT_UINT16) && (type!=GSALT_UINT32)) {
		gsalt_log(gsalt_verbose_error, "GSalt: Array indexes only support UINT16 and UINT32 (type=%d)\n", type);
		return GSALT_ERROR;
	}
	check_gsalt;

	gsalt_log(gsalt_verbose_debug, "GSalt: Array indexes defined (%s)\n", (type)?"UINT16":"UINT32");
	init_pointer(&pgsalt->indexes, pointer, 1, 0, 0, type);

	pgsalt->faces_defined=pgsalt->num_triangles;

	int idx1, idx2, idx3;
	multiptr array = pgsalt->indexes.ptr;
	int tp = pgsalt->indexes.type;
	for (int i=0; i<pgsalt->num_triangles; i++) {
		idx1 = (tp)?array.ui16[0]:array.ui32[0];
		idx2 = (tp)?array.ui16[1]:array.ui32[1];
		idx3 = (tp)?array.ui16[2]:array.ui32[2];
		pgsalt->model->add_face(idx1, idx2, idx3);
		if(tp)
			array.ui16+=3;
		else
			array.ui32+=3;
	}
}
