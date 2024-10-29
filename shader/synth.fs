#version 330 core
#define P  ( sqrt(3.14159265359)/2. )
#define scale 8 //number of repetitions

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D srcText;
// uniform vec2 R;//screen resolution
uniform vec2 R = vec2(1024.0, 576.0);
vec2 U = (TexCoords * R) / R.y * scale; 

// float erf(float x) {        // very good approx https://en.wikipedia.org/wiki/Error_function
//     float e = exp(-x*x); // ( Bürmann series )
//     return sign(x)/P * sqrt( 1.0 - e ) * ( P + 31.0/200.0*e - 341.0/8000. *e*e );
// }

// Approximate error function (erf) in GLSL
float erf(float x) {
    // Abramowitz & Stegun approximation coefficients
    float p = 0.278393;
    float q = 0.230389;
    float r = 0.000972;
    float s = 0.078108;

    // Absolute value of x (erf is an odd function)
    float t = abs(x);

    // The approximation formula
    float approximation = 1.0 - (1.0 / pow((1.0 + p * t + q * t * t + r * t * t * t + s * t * t * t * t), 4.0));

    // Adjust sign based on the sign of the input
    if (x < 0.0) {
        return -approximation;
    } else {
        return approximation;
    }
}

vec3 erf(vec3 x) {
    return vec3(erf(x.x), erf(x.y), erf(x.z));
}

//gamma correction, unused
vec3 srgb2rgb(vec3 V) {
    return pow( max(V,0.), vec3( 2.2 )  );
}

vec3 rgb2srgb(vec3 V) {
    return pow( max(V,0.), vec3(1./2.2) );
}

vec2 hash( vec2 p ) {
    return fract(sin((p)*mat2(127.1,311.7,269.5,183.3))*43758.5453);
}

vec2 duvdx = dFdx(U/scale);
vec2 duvdy = dFdy(U/scale);

vec3 fetch(vec2 uv) {
    // return srgb2rgb(textureGrad(srcText, U/scale + hash(uv), duvdx, duvdy).rgb);
    return textureGrad(srcText, U/scale + hash(uv), duvdx, duvdy).rgb;
}

void main() {
    //uv space <-> tiled space
    mat2 M0 = mat2( 1,0, 0.5,sqrt(3.0)/2.0 );
    mat2 M = inverse( M0 );    

    vec2 V = M * U,                                    // pre-hexa tilted coordinates
    I = floor(V);                                // hexa-tile id
    vec3 F = vec3(fract(V),0);

    //weights for interpolation
    vec3 W;
    F.z = 1.0 - F.x - F.y; // local hexa coordinates
    vec3 upper;
    vec3 avg = vec3(0.5);
    // vec3 avg = textureLod(srcText, TexCoords, 1000.f).rgb;//average color from mipmap

    if ( F.z > 0.0 )
        upper = ( W.x=   F.z ) * fetch(I)                      // smart interpolation
            + ( W.y=   F.y ) * fetch(I+vec2(0,1))            // of hexagonal texture patch
            + ( W.z=   F.x ) * fetch(I+vec2(1,0));           // centered at vertex
    else                                               
        upper = ( W.x=  -F.z ) * fetch(I + 1.) 
            + ( W.y=1. - F.y ) * fetch(I+vec2(1,0)) 
            + ( W.z=1. - F.x ) * fetch(I+vec2(0,1));

    //vec3 G_cov = (upper - avg)/length(W) + avg;
    vec3 G_cov = (upper-avg)/length(W) + avg;

    //inverse gaussian transformation
    vec3 U = vec3(0.5) + 0.5*erf((G_cov - vec3(0.5))/(6*sqrt(2.0)));
    FragColor = vec4(clamp(G_cov,0.0,1.0), 1.0);

    FragColor = vec4((G_cov), 1.0);

    // FragColor = texture(srcText, TexCoords);
}