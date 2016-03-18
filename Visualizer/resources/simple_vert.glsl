#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 MV;
out vec3 fragmentColor;

void main()
{
	gl_Position = P * MV * vec4(vertPos, 1.0);
    
    vec3 color = vec3(0.0, 1.0, 1.0);
    
    if (vertPos.y > 0.6) {
        color = vec3(1.0, 0.0, 0.0);
    }
    else if (vertPos.y > 0.3)
    {
        color = vec3(0.0, 1.0, 0.0);
    }
    
    // The color of each vertex will be interpolated
    // to produce the color of each fragment
    fragmentColor = (2/gl_Position.z) * color;
}
