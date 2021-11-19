#version 450

layout(binding=0)uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
}ubo;

layout(location=0)in vec2 inPosition;
layout(location=1)in vec3 inColor;
layout(location=2)in vec2 inTexCoord;

layout(location=0)out vec3 fragColor;
layout(location=1)out vec2 fragTexCoord;
layout(location=2)out vec4 uboColor;

void main(){
    gl_Position=ubo.proj*ubo.view*ubo.model*vec4(inPosition,0.,1.);
    fragColor=inColor;
    fragTexCoord=inTexCoord;//vec2(1-inTexCoord.x,1.-inTexCoord.y);
    uboColor=ubo.color;
    
}