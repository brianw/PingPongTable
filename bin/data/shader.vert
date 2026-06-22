#ifdef GL_ES
attribute vec4 position;
uniform mat4 modelViewProjectionMatrix;
#endif

void main(){
#ifdef GL_ES
	gl_Position = modelViewProjectionMatrix * position;
#else
	gl_Position = ftransform();
#endif
}
