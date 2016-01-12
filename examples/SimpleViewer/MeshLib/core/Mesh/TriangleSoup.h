#ifndef _TRIANGLE_SOUP_H_
#define _TRIANGLE_SOUP_H_

#include <vector>
#include <map>
#include <string.h>
#include <iostream>
#include <fstream>

#include "../Geometry/Point.h"
#include "../Geometry/Point2.h"
#include "../Parser/parser.h"
#include "../Parser/StrUtil.h"

namespace MeshLib
{
namespace TriangleSoup
{

class CVertex
{
	public:
		CPoint    m_point;
		CPoint2   m_uv;
		CPoint    m_rgb;
		CPoint    m_normal;
		int       m_id;
		int 	  m_cnt;
		double    m_area;
};

class CFace
{
	public:
		int m_id;
		CVertex * m_v[3];
		CPoint    m_normal;
		double    m_area;
};



class CTriangleMesh
{
  public:

	CTriangleMesh()
	{
		m_has_uv     = false;
		m_has_rgb    = false;
		m_has_normal = false;
	};

	~CTriangleMesh(){
		for( size_t i = 0; i < m_vertices.size(); i ++ )
		{
			CVertex * pV = m_vertices[i];
			delete pV;
		}
		for( size_t i = 0; i < m_faces.size(); i ++ )
		{
			CFace * pF = m_faces[i];
			delete pF;
		}
		m_vertices.clear();
		m_faces.clear();
	};
	
	bool m_has_uv;
	bool m_has_rgb;
	bool m_has_normal;


	std::vector<CFace*> 	& faces() { return	m_faces;};
	std::vector<CVertex*> 	& vertices() { return	m_vertices;};

   protected:

	std::vector<CVertex*>   m_vertices;
	std::vector<CFace*> 	m_faces;
	std::map<int,CVertex*>  m_map;
	// for obj reading
	std::vector<CPoint>     m_points;
	std::vector<CPoint2>    m_uvs;
	std::vector<CPoint>     m_normals;
	std::map<std::string,CVertex*> m_map2;


   public:

	void read_vertex_m( char * line )
	{
		char * str = strtok(line, " \r\n");
		if( strcmp( str, "Vertex" ) != 0 )
		{
			std::cerr << "Error in reading vertex" << std::endl;
			return;
		}
		CVertex * pV = new CVertex;
		pV->m_cnt = m_vertices.size();
        	pV->m_rgb = CPoint(1,1,1);

		str = strtok(NULL, " \r\n");

		pV->m_id = atoi( str );

		CPoint p;
		for( int i = 0; i < 3; i ++ )
		{
			str = strtok(NULL, " \r\n");
			p[i] = atof( str );
		}
		pV->m_point = p;

		str = strtok( NULL, "\r\n" );

		if( str == NULL ) return;

		std::string s(str);

		size_t pos0 = s.find_first_of('{');
		size_t pos1 = s.find_last_not_of('}');

		s = s.substr( pos0+1, pos1);

		CParser parser(s);

		for( std::list<CToken*>::iterator iter = parser.tokens().begin(); iter != parser.tokens().end(); iter ++ )
		{
			CToken * pT = *iter;
			if( pT->m_key == "rgb" )
			{
			   CPoint rgb;
			   sscanf(pT->m_value.c_str(), "(%lf %lf %lf)", &rgb[0], &rgb[1], &rgb[2]);
			   pV->m_rgb = rgb;
			   m_has_rgb = true;
			}
			if( pT->m_key == "uv" )
			{
			   CPoint2 uv;
			   sscanf(pT->m_value.c_str(), "(%lf %lf)", &uv[0], &uv[1]);
			   pV->m_uv = uv;
			   m_has_uv = true;
			}
		}

		m_vertices.push_back( pV );
		m_map[pV->m_id] = pV;
	};

	void read_face_m( char * line )
	{
		char * str = strtok(line, " \r\n");
		if( strcmp( str, "Face" ) != 0 )
		{
			std::cerr << "Error in reading face" << std::endl;
			return;
		}
		CFace * pF = new CFace;

		str = strtok(NULL, " \r\n");
		pF->m_id = atoi( str );

		for( int i = 0; i < 3; i ++ )
		{
			str = strtok(NULL, " \r\n");
			int id = atoi( str );
			//pF->m_v[i] = m_vertices[id-1];
			pF->m_v[i] = m_map[id];
		}
		m_faces.push_back( pF );
	};

	void read_m( const char * name )
	{
		FILE * fp=fopen(name,"r");
		char line[1024];

		while( !feof( fp ) )
		{
			if( fgets(line, 1024, fp) == NULL ) break;

			if( strncmp( line, "Vertex", 6 ) == 0 )
			{
				read_vertex_m(line);
				continue;
			}
			if( strncmp( line, "Face", 4 ) == 0 )
			{
				read_face_m(line);
				continue;
			}
		}
		fclose( fp );
		m_map.clear();
	}

	void read_vertex_obj( char * line )
	{
		if( line[1]=='t' )
		{
			CPoint2 uv;
			sscanf(line,"vt %lf %lf", &uv[0], &uv[1]);
			m_uvs.push_back( uv );
			return;
		}
		
		if( line[1] == 'n')
		{
			CPoint n;
			sscanf(line,"vn %lf %lf %lf", &n[0], &n[1], &n[2]);
			m_normals.push_back( n );
			return;
		}

		CPoint p;
		sscanf(line,"v %lf %lf %lf", &p[0], &p[1], &p[2]);
		m_points.push_back( p );
	}
	CVertex* add_vertex_obj(int point, int uv, int normal)
	{
		CVertex *c;
		char buff[100];
		sprintf(buff,"%d", point);
		if (uv!=-1) sprintf(buff, "%s/%d", buff, uv);
		if (normal!=-1) sprintf(buff, "%s/%d", buff, normal);
		std::string s=buff;

		std::map<std::string,CVertex*>::iterator search = m_map2.find(buff);
		if (search==m_map2.end()) {
			// create the vertex
			c = new CVertex;
			c->m_id = m_vertices.size();
			c->m_cnt = m_vertices.size();
			c->m_point = m_points[point-1];
			if(uv!=-1) c->m_uv = m_uvs[uv-1];
			if(normal!=-1) c->m_normal = m_normals[uv-1];
			c->m_rgb = CPoint(1,1,1);
			m_map2[s]=c;
			m_vertices.push_back(c);
		} else {
			c = search->second;
		}
		return c;
	}

	void read_face_obj( char * line )
	{
		int faces[100][3];
		int f = 0;
		char* str = strtok(line, " \n");
		while( str = strtok(NULL, " \n") )
		{
			if((str[0]>='0') && (str[0]<='9')) {
				faces[f][0]=-1; faces[f][1]=-1; faces[f][2]=-1;
				char *str_uv=NULL, *str_norm=NULL;
				str_uv=strchr(str,'/'); 
				if(str_uv) {
					str_uv[0]='\0'; str_uv++;
					str_norm=strchr(str_uv,'/');
					if(str_norm) {
						str_norm[0]='\0'; str_norm++;
					}
				}
				faces[f][0] = atoi(str);
				if(str_uv && str_uv[0]!='\0') faces[f][1] = atoi(str_uv);
				if(str_norm && str_norm[0]!='\0') faces[f][2] = atoi(str_norm);
				f++;
			}
		}
		if(f<2) return;
		// add the faces and vertex
		CVertex *c1 = add_vertex_obj(faces[0][0], faces[0][1], faces[0][2]);
		CVertex *c2 = add_vertex_obj(faces[1][0], faces[1][1], faces[1][2]);
		for (int i=2; i<f; i++) {
			CVertex *c3 = add_vertex_obj(faces[i][0], faces[i][1], faces[i][2]);
			CFace *f = new CFace;
			f->m_id = m_faces.size();
			f->m_v[0] = c1; f->m_v[1] = c2; f->m_v[2] = c3;
			m_faces.push_back(f);
			c2 = c3;
		}
		if (faces[0][1]!=-1) m_has_uv = true;
		if (faces[0][2]!=-1) m_has_normal = true;
	}

	void read_obj( const char * name )
	{

		FILE * fp=fopen(name,"r");
		char line[1024];

		while( !feof( fp ) )
		{
			if( fgets(line, 1024, fp) == NULL ) break;
			
			if(line[0]=='v') 
				read_vertex_obj(line);

			if(line[0]=='f')
				read_face_obj(line);
		}
		fclose( fp );
		m_has_rgb = false;
		m_points.clear();
		m_uvs.clear();
		m_normals.clear();
		m_map.clear();
	}
};

}	//namspace TriangleSoup

}	//namespace Mesh
#endif //_TRIANGLE_SOUP_H_