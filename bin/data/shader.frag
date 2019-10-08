precision highp float;
uniform sampler2D backbuffer;   // previus buffer
uniform sampler2D tex0;         // actual buffer

uniform float damping;
uniform vec2 resolution;

vec2 offset[4];
varying vec2 texCoordVarying;

void main(){

    //float stepX = 1.0/float(resolution.x);
    //float stepY = 1.0/float(resolution.y);
    vec2 step = vec2(1.0/resolution);

    offset[0] = vec2(-step.x, 0.0);
    offset[1] = vec2(step.x, 0.0);
    offset[2] = vec2(0.0, step.y);
    offset[3] = vec2(0.0, -step.y);
    //  Grab the information arround the active pixel
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]



    vec2 st = vec2(gl_FragCoord.xy/resolution);
    st.x = st.x + 0.5685/float(resolution.x);
    st.y = st.y + 0.568/float(resolution.y);
    vec3 sum = texture2D(tex0, st + offset[0]).rgb;
    sum += texture2D(tex0, st + offset[1]).rgb;
    sum += texture2D(tex0, st + offset[2]).rgb;
    sum += texture2D(tex0, st + offset[3]).rgb;

    //  make an average and substract the center value
    //
    sum = ((sum / 2.0) - texture2D(backbuffer, st).rgb);
    sum *= 0.99;
    
    gl_FragColor = vec4(sum, 1.0);

}
