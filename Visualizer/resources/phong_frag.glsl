#version 330 core 

in vec3 fragNor;
in vec3 fragPos;

uniform vec3 lightPos;

out vec4 color;

void main()
{
    vec3 lightNor = normalize(fragNor); 
    //normal = (MV * vec4(vertNor, 0.0)).xyz;
    
    //lighting hack replace with real lighting!!!
    vec3 lightVec = normalize(lightPos - fragPos);
    
    color = vec4(lightNor + lightVec, 1.0);
}
