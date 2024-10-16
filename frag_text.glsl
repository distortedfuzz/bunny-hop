#version 330 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them
// here for simplicity.

vec3 I = vec3(1, 1, 1);          // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(0.0, 0.0, 1.0);     // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.0, 0.0, 0.0);   // specular reflectance coefficient
vec3 lightPos = vec3(5, 5, 5);   // light position in world coordinates
uniform float speed;

uniform vec3 eyePos;

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
    // Compute lighting. We assume lightPos and eyePos are in world
    // coordinates. fragWorldPos and fragWorldNor are the interpolated
    // coordinates by the rasterizer.

    vec3 L = normalize(lightPos - vec3(fragWorldPos));
    vec3 V = normalize(eyePos - vec3(fragWorldPos));
    vec3 H = normalize(L + V);
    vec3 N = normalize(fragWorldNor);

    float NdotL = dot(N, L); // for diffuse component
    float NdotH = dot(N, H); // for specular component

    vec3 diffuseColor = I * kd * max(0, NdotL);
    vec3 specularColor = I * ks * pow(max(0, NdotH), 100);
    vec3 ambientColor = Iamb * ka;

    bool x = false;
    int x_comparer = int(fragWorldPos.x + 7)/2;
    if((x_comparer) % 2 == 0){
        x = true;
    }
    bool y = false;
    int y_comparer = int(fragWorldPos.y + 7)/2;
    if((y_comparer) % 2 == 0){
        y = true;
    }
    bool z = false;
    int z_comparer = int(fragWorldPos.z -speed-7)/2;
    if((z_comparer) % 2 == 0){
        z = true;
    }

    bool xorXY = x != y;
    if (xorXY != z){

        vec3 blue = vec3(0.2,0.2,0.8);
        fragColor =  vec4(blue,1);

    }else{
        vec3 grey = vec3(0.4,0.4,0.4);
        fragColor =  vec4(grey,1);
    }
    //fragColor = vec4(specularColor + ambientColor, 1); //or try this, don't forget to change the active program in main.cpp
}
