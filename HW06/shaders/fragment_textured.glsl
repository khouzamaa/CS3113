uniform sampler2D diffuse;
uniform int is_dead;

varying vec2 texCoordVar;


void main()
{
    if (is_dead == 1)
    {
        vec4 color = texture2D(diffuse, texCoordVar);
        float c = (color.r + color.g + color.b)/ 3.0;
        gl_FragColor = vec4(c, c, c, color.a);
    }
    else
    {
        gl_FragColor = texture2D(diffuse, texCoordVar);
    }
}