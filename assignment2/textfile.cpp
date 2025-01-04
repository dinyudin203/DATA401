// textfile.cpp
//
// simple reading and writing for text files
//
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//
// Modified by Won-Ki Jeong 
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>

#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

char *textFileRead(char *fn) {


	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

int textFileWrite(char *fn, char *s) {

	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}


GLuint loadShader(GLenum shadertype, char *c)
{
	GLuint s = glCreateShader( shadertype );
	char *ss = textFileRead( c );
	const char *css = ss;
	glShaderSource(s, 1, &css, NULL);
	free( ss );
	glCompileShader( s );

	// validation
	int status, sizeLog;
	glGetShaderiv(s, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &sizeLog);
		char *log = new char[sizeLog + 1];
		glGetShaderInfoLog(s, sizeLog, NULL, log);
		std::cout << "Shader Compilation Error: " << log << std::endl;
		delete [] log;
		assert( false );
	}

	return s;
}

GLuint createGLSLProgram(char *vs, char *gs, char *fs) 
{
	GLuint v, g, f, p;
	
	if( !vs && !gs && !fs ) return 0;
		
	p = glCreateProgram();
	
	if( vs ) 
	{
		v = loadShader( GL_VERTEX_SHADER, vs );
		glAttachShader(p,v);
	}
	if( gs )
	{
		g = loadShader( GL_GEOMETRY_SHADER_EXT, gs );
		glAttachShader(p,g);
	}
	if( fs )
	{
		f = loadShader( GL_FRAGMENT_SHADER, fs );
		glAttachShader(p,f);
	}

	glLinkProgram(p);

	// validating program
	GLint status;
	glGetProgramiv(p, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint sizeLog;
		glGetProgramiv(p, GL_INFO_LOG_LENGTH, &sizeLog);
		char *log = new char[sizeLog + 1];
		glGetProgramInfoLog(p, sizeLog, NULL, log);
		std::cout << "Program Link Error: " << log << std::endl;
		delete [] log;
		assert( false );
	}
	
	glValidateProgram(p);
	glGetProgramiv(p, GL_VALIDATE_STATUS, &status);

	if (status == GL_FALSE)
	{
		std::cerr << "Error validating program: "<< p << std::endl;
		assert( false );
	}

	// validation passed.. therefore, we will use this program
	glUseProgram(p);

	return p;
}




