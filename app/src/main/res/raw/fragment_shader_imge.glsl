precision mediump float;
varying vec2 v_texPo;//纹理位置  接收于vertex_shader
uniform sampler2D sTexture;//纹理    uniform用于application向 vertex和fragment中传值
uniform vec4 af_Color;
void main(){
    //gl_FragColor = af_Color;
    gl_FragColor=texture2D(sTexture, v_texPo);
}
