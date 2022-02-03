void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    fragColor = 2.*texelFetch(iChannel0,ivec2(fragCoord),0);
}