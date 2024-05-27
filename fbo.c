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
  glDisable(GL_DEPTH_TEST);

  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gScreenShaderProgram);
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

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  // Filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gTextureColorBuffer, 0);


  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "Framebuffer is not complete!\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}