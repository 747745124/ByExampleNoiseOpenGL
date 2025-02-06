#version 400 core

out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D src_texture;
uniform sampler2D gauss_texture;
uniform sampler2D inv_lut_texture;
// inverse transformation LUT

uniform float aspect_ratio = 1.0f;
uniform int blendMode = 0;
// Decorrelated color space vectors and origin
uniform vec3 _colorSpaceVec1;
uniform vec3 _colorSpaceVec2;
uniform vec3 _colorSpaceVec3;
uniform vec3 _colorSpaceOrigin;

vec3 ReturnToOriginalColorSpace(vec3 color)
{
	vec3 result = 
		_colorSpaceOrigin +
		_colorSpaceVec1 * color.r +
		_colorSpaceVec2 * color.g +
		_colorSpaceVec3 * color.b;
	return result;
}

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

vec2 hash( vec2 p ) {
    return fract(sin((p)*mat2(127.1,311.7,269.5,183.3))*43758.5453);
}

// Compute local triangle barycentric coordinates and vertex IDs
void TriangleGrid(vec2 uv,
	out float w1, out float w2, out float w3,
	out ivec2 vertex1, out ivec2 vertex2, out ivec2 vertex3)
{
	// Scaling of the input
	uv *= 3.464; // 2 * sqrt(3)

	// Skew input space into simplex triangle grid
	const mat2 gridToSkewedGrid = mat2(1.0, 0.0, -0.57735027, 1.15470054);
	vec2 skewedCoord = gridToSkewedGrid * uv;

	// Compute local triangle vertex IDs and local barycentric coordinates
	ivec2 baseId = ivec2(floor(skewedCoord));
	vec3 temp = vec3(fract(skewedCoord), 0);
	temp.z = 1.0 - temp.x - temp.y;
	if (temp.z > 0.0)
	{
		w1 = temp.z;
		w2 = temp.y;
		w3 = temp.x;
		vertex1 = baseId;
		vertex2 = baseId + ivec2(0, 1);
		vertex3 = baseId + ivec2(1, 0);
	}
	else
	{
		w1 = -temp.z;
		w2 = 1.0 - temp.y;
		w3 = 1.0 - temp.x;
		vertex1 = baseId + ivec2(1, 1);
		vertex2 = baseId + ivec2(1, 0);
		vertex3 = baseId + ivec2(0, 1);
	}
}


vec3 fetch(vec2 uv, vec2 duvdx, vec2 duvdy) {
	// without OT, direct apply interpolation
    if(blendMode==0||blendMode==1)
		return (textureGrad(src_texture, uv, duvdx, duvdy).rgb);

    return (textureGrad(gauss_texture, uv, duvdx, duvdy).rgb);
}

void main() {

	vec2 uv = TexCoord;
	uv.x *= aspect_ratio;

	//source picture
	if(blendMode==3)
	{	
		FragColor = vec4(texture(src_texture, uv).rgb, 1.0);
		return;
	}

	//gaussian picture
	if(blendMode==4)
	{
		FragColor = vec4(texture(gauss_texture, uv).rgb, 1.0);
		return;
	}

    float w1, w2, w3;
	ivec2 vertex1, vertex2, vertex3;
    TriangleGrid(uv, w1, w2, w3, vertex1, vertex2, vertex3);

    // Assign random offset to each triangle vertex
	vec2 uv1 = uv + hash(vertex1);
	vec2 uv2 = uv + hash(vertex2);
	vec2 uv3 = uv + hash(vertex3);

	// Precompute UV derivatives 
	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);

	// Fetch Gaussian input
	vec3 G1 = fetch(uv1, duvdx, duvdy).rgb;
	vec3 G2 = fetch(uv2, duvdx, duvdy).rgb;
	vec3 G3 = fetch(uv3, duvdx, duvdy).rgb;

	vec3 avg = vec3(0.5);
    vec3 G_upper = w1*G1 + w2*G2 + w3*G3;
	vec3 G_cov = G_upper - avg;
	G_cov = G_cov * inversesqrt(w1*w1 + w2*w2 + w3*w3);
	G_cov = G_cov + avg;
    G_cov = clamp(G_cov,0.0,1.0);

	//linear blend
    if(blendMode==0)
	{
		FragColor = vec4(G_upper, 1.0);
		return;
	}

	//variance blend or gaussian blend
	if(blendMode==1 || blendMode==5)
	{
		FragColor = vec4(G_cov, 1.0);
		return;
	}


	//inverse LUT
	vec3 color;
	float LOD = textureQueryLod(gauss_texture, uv).y / float(textureSize(inv_lut_texture, 0).y);

	color.r = texture(inv_lut_texture, vec2(G_cov.r, LOD)).r;
	color.g	= texture(inv_lut_texture, vec2(G_cov.g, LOD)).g;
	color.b	= texture(inv_lut_texture, vec2(G_cov.b, LOD)).b;
    FragColor = vec4(ReturnToOriginalColorSpace(color), 1.0);
}