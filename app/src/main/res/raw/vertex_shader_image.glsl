attribute vec4 av_Position; //attribute 只能在顶点shader中使用
attribute vec2 af_Position;//纹理位置
varying vec2 v_texPo;//纹理位置  与fragment_shader交互，fragment_shader中需要声明同样的变量名
void main(){
    v_texPo = af_Position;
    gl_Position = av_Position;
}
