#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform highp vec4 color;

void main(void)
{
    gl_FragColor = color;
}
