#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform highp mat4 mvp_matrix;
attribute highp vec4 vertex;

void main(void)
{
    gl_Position = mvp_matrix * vertex;
}
