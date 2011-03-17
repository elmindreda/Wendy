// Adapted from the RenderMonkey HDR sample

uniform sampler2D image;
uniform vec2 scale;

in vec2 mapping;

vec2 gaussFilter[3];

void main()
{
  vec4 color = vec4(0.0);
  
  gaussFilter[0] = vec2(-1.5, 15.0 / 50.0);
  gaussFilter[1] = vec2( 0.0, 20.0 / 50.0);
  gaussFilter[2] = vec2( 1.5, 15.0 / 50.0);
  
  for (int i = 0;  i < 3;  i++)
  {
    color += texture2D(image, vec2(mapping.s + gaussFilter[i].s * scale.s, mapping.t)) * 
             gaussFilter[i].t;
  }
  
  gl_FragColor = color;
}

