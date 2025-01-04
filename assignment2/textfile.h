// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//
// Modified by Won-Ki Jeong
//
//////////////////////////////////////////////////////////////////////

char *textFileRead(char *fn);
int textFileWrite(char *fn, char *s);
GLuint createGLSLProgram(char *vs=NULL, char *gs=NULL, char *fs=NULL);