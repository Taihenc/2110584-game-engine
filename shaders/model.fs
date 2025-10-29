#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform bool hasTexture;

void main()
{    
    if (hasTexture) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        FragColor = texColor * 1.5; // Brighten textures
    } else {
        // Brighter default airplane color
        FragColor = vec4(0.95, 0.95, 1.0, 1.0);
    }
}