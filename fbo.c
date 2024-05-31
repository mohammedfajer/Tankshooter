GLuint gFbo;
GLuint gTextureColorBuffer;

float gQuadVertices [] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};

unsigned int gQuadIndices[] = {
  0,1,2,
  0,2,3
};

GLuint gQuadVAO, gQuadVBO, gQuadEBO;
GLuint gScreenShaderProgram;

// Shader sources for screen quad
const char* gScreenVertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    TexCoord = aTexCoord;\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\0";

const char* gScreenFragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D screenTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(screenTexture, TexCoord );\n"
    "}\n\0";

// const char* gScreenFragmentShaderSource = "#version 330 core\n"
//     "out vec4 FragColor;\n"
//     "in vec2 TexCoord;\n"
//     "uniform sampler2D screenTexture;\n"
//     "uniform float time;\n"
//     "uniform vec2 resolution;\n"

//     "void main()\n"
//     "{\n"
//     "    // Apply a wave distortion to the texture coordinates\n"
//     "    float wave = sin(TexCoord.y * 10.0 + time) * 0.01;\n"
//     "    vec2 distortedTexCoord = vec2(TexCoord.x + wave, TexCoord.y);\n"
    
//     "    // Sample the texture\n"
//     "    vec4 color = texture(screenTexture, distortedTexCoord);\n"

//     "    // Color shifting effect\n"
//     "    float red = color.r * (0.5 + 0.5 * sin(time));\n"
//     "    float green = color.g * (0.5 + 0.5 * sin(time + 2.0));\n"
//     "    float blue = color.b * (0.5 + 0.5 * sin(time + 4.0));\n"
//     "    vec4 colorShifted = vec4(red, green, blue, 1.0);\n"

//     "    // Vignette effect\n"
//     "    vec2 position = (TexCoord - 0.5) * resolution;\n"
//     "    float vignette = smoothstep(0.5, 0.9, length(position) / max(resolution.x, resolution.y));\n"
//     "    vec4 finalColor = mix(colorShifted * cos(vignette), colorShifted * vignette, 0.5);\n"

//     "    FragColor = finalColor;\n"
//     "}\n";


void init_quad()
{
  glGenVertexArrays(1, &gQuadVAO);
  glGenBuffers(1, &gQuadVBO);
  glGenBuffers(1, &gQuadEBO);
  glBindVertexArray(gQuadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gQuadVertices), gQuadVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gQuadIndices), gQuadIndices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindVertexArray(0);
}


void render_quad()
{
  //glDisable(GL_DEPTH_TEST);

  //glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gScreenShaderProgram);

   // Set the time uniform
  float time = SDL_GetTicks() / 1000.0f; // Convert milliseconds to seconds
  glUniform1f(glGetUniformLocation(gScreenShaderProgram, "time"), time);

  // Set the resolution uniform
   int windowWidth, windowHeight;
  SDL_GetWindowSize(gWindow, &windowWidth, &windowHeight);
  glUniform2f(glGetUniformLocation(gScreenShaderProgram, "resolution"), (float)windowWidth , (float)windowHeight );

  glBindVertexArray(gQuadVAO);
  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}




void init_framebuffer(int width, int height)
{
  glGenFramebuffers(1, &gFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gFbo);

  glGenTextures(1, &gTextureColorBuffer);
  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width , height , 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  // Filtering
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gTextureColorBuffer, 0);


  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width , height );
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "Framebuffer is not complete!\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}