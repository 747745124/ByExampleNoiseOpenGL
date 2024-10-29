#version 330 core

uniform sampler2D srcText;

in vec2 TexCoords;

out vec3 out_color;

vec3 srgb2rgb(vec3 V) {
    return pow( max(V,0.), vec3( 2.2 )  );
}

vec3 rgb2srgb(vec3 V) {
    return pow( max(V,0.), vec3(1./2.2) );
}

vec2 permute(vec2 uv){
  return mod(((uv*34.00)+1.0)*uv, 289.0f);
}

//https://en.wikipedia.org/wiki/Simplex_noise
vec2 skew (vec2 coord) {
    float n = 2;
    float F = (sqrt(n+1)-1)/n;
    return coord + (coord.x+coord.y)*F;
}

//https://thebookofshaders.com/11/
//http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
vec3 simplexGrid (vec2 st, float div, out vec2 uv1, out vec2 uv2, out vec2 uv3, out bool up) {
    vec3 w = vec3(0.0);

    vec2 sp = skew(st*div);
    vec2 p = fract(sp);
    if (p.x > p.y) { // upper triangle
        w.xy = 1-vec2(p.x,p.y-p.x);
        w.z = p.y;
        uv1 = floor(sp)/div;
        uv2 = uv1 + (vec2(1,0)/div);
        uv3 = uv1 + (vec2(1,1)/div);
        up = true;
    } else { // lower triangle
        w.yz = 1.0-vec2(p.x-p.y,p.y);
        w.x = p.x;
        uv3 = floor(sp)/div;
        uv2 = uv3 + (vec2(0,1)/div);
        uv1 = uv3 + (vec2(1,1)/div);
        up = false;
    }

    return fract(w);
}

void main() {
  vec2 uv1, uv2, uv3;
  bool up;
  vec3 w = simplexGrid(TexCoords, 8.0, uv1, uv2, uv3, up);
  
  vec3 upper;
  uv1 = TexCoords + permute(uv1);
  uv2 = TexCoords + permute(uv2);
  uv3 = TexCoords + permute(uv3);

  vec2 duvdx = dFdx(TexCoords);
  vec2 duvdy = dFdy(TexCoords);

  vec3 avg = textureLod(srcText, TexCoords, 1000.f).rgb;//average color from mipmap
  vec3 g1 = srgb2rgb(textureGrad(srcText, uv1, duvdx, duvdy).rgb);
  vec3 g2 = srgb2rgb(textureGrad(srcText, uv2, duvdx, duvdy).rgb);
  vec3 g3 = srgb2rgb(textureGrad(srcText, uv3, duvdx, duvdy).rgb);

  out_color = (g1*w.r + g2*w.g + g3*w.b) - vec3(0.5);
  out_color /= sqrt(w.r*w.r + w.g*w.g + w.b*w.b);
  out_color += vec3(0.5);

  upper = (g1*w.r + g2*w.g + g3*w.b);
  out_color = rgb2srgb(out_color);
}
