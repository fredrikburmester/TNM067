#include "utils/structs.glsl"

uniform sampler2D vfColor;
uniform ImageParameters vfParameters;
in vec3 texCoord_;

float passThrough(vec2 coord){
    return texture(vfColor,coord).x;
}

float magnitude(vec2 coord){
    //TASK 1: find the magnitude of the vectorfield at the position coords
    vec2 velo = texture(vfColor, coord).xy;
    return sqrt(pow(velo.x,2) + pow(velo.y,2));
}

float divergence(vec2 coord){
    //TASK 2: find the divergence of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;
        
    vec2 v1 = texture(vfColor, vec2(coord.x + pixelSize.x, coord.y)).xy;
    vec2 v2 = texture(vfColor, vec2(coord.x - pixelSize.x, coord.y)).xy;
    vec2 v3 = texture(vfColor, vec2(coord.x, coord.y + pixelSize.y)).xy;
    vec2 v4 = texture(vfColor, vec2(coord.x, coord.y - pixelSize.y)).xy;

    vec2 dVdx = (v1 - v2) / (2 * pixelSize.x);
    vec2 dVdy = (v3 - v4) / (2 * pixelSize.y);
    
    return dVdx.x + dVdy.y;
}

float rotation(vec2 coord){
    //TASK 3: find the curl of the vectorfield at the position coords
    vec2 pixelSize = vfParameters.reciprocalDimensions;
    
    vec2 v1 = texture(vfColor, vec2(coord.x + pixelSize.x, coord.y)).xy;
    vec2 v2 = texture(vfColor, vec2(coord.x - pixelSize.x, coord.y)).xy;
    vec2 v3 = texture(vfColor, vec2(coord.x, coord.y + pixelSize.y)).xy;
    vec2 v4 = texture(vfColor, vec2(coord.x, coord.y - pixelSize.y)).xy;

    vec2 dVdx = (v1 - v2) / (2 * pixelSize.x);
    vec2 dVdy = (v3 - v4) / (2 * pixelSize.y);
    
    return dVdx.y - dVdy.x;
}

void main(void) {
    float v = OUTPUT(texCoord_.xy);
    FragData0 = vec4(v);
}
