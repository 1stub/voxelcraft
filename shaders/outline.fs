#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.90, 0.90, 0.90, 1.0);
    FragColor.a = 0.5;
}
