#version 330 core
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;

out vec4 outColor;
out vec2 TexCoord;

void main()
{
  //vec2 ndcCoords;
  //ndcCoords.x = (2.0 * inPosition.x / screenSize.x) - 1;
  //ndcCoords.y = 1.0 - (2.0 * inPosition.y / screenSize.y);
  //gl_Position = vec4(ndcCoords, 0.0, 1.0);
  
  // convert the 2d screen position to homogeneous coordinates (vec4)
  vec4 screenPos = vec4(inPosition, 0.0, 1.0);

  // transform the screen position to NDC using the orthographic projection matrix
  vec4 ndcPos = projection  * view * screenPos;

  gl_Position = ndcPos;

  outColor = inColor;
  TexCoord = inTexCoord;
}