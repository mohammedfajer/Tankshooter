
#define MAX_QUADS 2000
#define MAX_VERTICES (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)

typedef struct {
  Vec2 position;
  Vec4 color;
  Vec2 texCoord;
  float texID;
} Vertex;

typedef struct {
  
  Vertex vertices[MAX_VERTICES];
  int indices[MAX_INDICES];
  int quad_count;

  // Add OpenGL render objects
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;

  GLuint shaderProgram;

  Mat4 viewMatrix;
  Mat4 projectionMatrix;

  GLuint globalTexId;

} QuadBatch;

// Function to initialize quad batch
void init_quad_batch(QuadBatch *q);

// Function to generate indices for the quad batch
void generate_indices(QuadBatch *q);

// Function to generate VAO, EBO, VBO for rendering
void generate_render_objects(QuadBatch *q);


// Generate indices for the quad batch
void generate_indices(QuadBatch *q) {
  for(int i=0; i< MAX_INDICES; i+=6) {
    int base_index = i / 6 * 4;
    // First Triangles
    q->indices[i + 0] = base_index; 
    q->indices[i + 1] = base_index + 1;
    q->indices[i + 2] = base_index + 3;
    // Second Triangles
    q->indices[i + 3] = base_index + 1;
    q->indices[i + 4] = base_index + 2;
    q->indices[i + 5] = base_index + 3;
  }
}

void init_quad_batch(QuadBatch *q) {
  q->quad_count = 0;

  // Load and compile shaders
  char *vs = readFile("./data/shaders/texturedquad.vs");
  char *fs = readFile("./data/shaders/texturedquad.fs");

  printf("VS %s\n", vs);
  printf("VS %s\n", fs);

  q->shaderProgram = createShaderProgram(vs, fs);

  // Set the shader program
  glUseProgram(q->shaderProgram);

  // Set uniform values for screen size and color
  glUniform2f(glGetUniformLocation(q->shaderProgram, "screenSize"), WINDOW_WIDTH, WINDOW_HEIGHT);
  glUniform4f(glGetUniformLocation(q->shaderProgram, "color"),1, 0, 0, 1);

  q->projectionMatrix = createOrthographicMatrix(0, WINDOW_WIDTH,  WINDOW_HEIGHT, 0, -1.0F, 1.0f);
  q->viewMatrix = createIdentityMatrix();

  glUniformMatrix4fv(glGetUniformLocation(q->shaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)q->projectionMatrix.data);
  glUniformMatrix4fv(glGetUniformLocation(q->shaderProgram, "view"), 1, GL_FALSE, (const GLfloat *)q->viewMatrix.data);
  
  generate_indices(q);
  generate_render_objects(q);
}

void add_quad(QuadBatch *q, Vec2 position, Vec2 size)
{
  int x = position.x;
  int y = position.y;
  int w = size.x;
  int h = size.y;

  // Calculate vertices
  Vertex v0 = { {x, y + h} };
  Vertex v1 = { {x + w, y + h} };
  Vertex v2 = { {x  + w, y} };
  Vertex v3 = { {x , y } };

  // Add vertices to the array
  int base_index = q->quad_count * 4; // Calculate the base index for the new quad
  q->vertices[base_index + 0] = v0;
  q->vertices[base_index + 1] = v1;
  q->vertices[base_index + 2] = v2;
  q->vertices[base_index + 3] = v3;

  // Update quad count
  q->quad_count++;
}

void add_quad_color(QuadBatch *q, Vec2 position, Vec2 size, Vec4 color)
{
  int x = position.x;
  int y = position.y;
  int w = size.x;
  int h = size.y;
  int r = color.x;
  int g = color.y;
  int b = color.z;
  int a = color.w;

  // Calculate vertices
  Vertex v0 = { {x, y + h}, {r,g,b,a} };
  Vertex v1 = { {x + w, y + h}, {r,g,b,a} };
  Vertex v2 = { {x  + w, y}, {r,g,b,a}};
  Vertex v3 = { {x , y }, {r,g,b,a} };

  // Add vertices to the array
  int base_index = q->quad_count * 4; // Calculate the base index for the new quad
  q->vertices[base_index + 0] = v0;
  q->vertices[base_index + 1] = v1;
  q->vertices[base_index + 2] = v2;
  q->vertices[base_index + 3] = v3;

  // Update quad count
  q->quad_count++;
}

void add_quad_texture(QuadBatch *q, Vec2 position, Vec2 size, Vec4 color, float textureId)
{
  int x = position.x;
  int y = position.y;
  int w = size.x;
  int h = size.y;
  int r = color.x;
  int g = color.y;
  int b = color.z;
  int a = color.w;

  // Calculate vertices
  Vertex v0 = { {x, y + h}, {r,g,b,a}, {0.0f, 0.0f}, textureId };
  Vertex v1 = { {x + w, y + h}, {r,g,b,a}, {1.0f, 0.0f}, textureId };
  Vertex v2 = { {x  + w, y}, {r,g,b,a}, {1.0f, 1.0f}, textureId};
  Vertex v3 = { {x , y }, {r,g,b,a} , {0.0f, 1.0f}, textureId};

  // Add vertices to the array
  int base_index = q->quad_count * 4; // Calculate the base index for the new quad
  q->vertices[base_index + 0] = v0;
  q->vertices[base_index + 1] = v1;
  q->vertices[base_index + 2] = v2;
  q->vertices[base_index + 3] = v3;

  // Update quad count
  q->quad_count++;
}


// Generate VAO, VBO, EBO for rendering
void generate_render_objects(QuadBatch *q) {
    // Generate and bind VAO
    glGenVertexArrays(1, &q->VAO);
    glBindVertexArray(q->VAO);
    
    // Generate VBO and bind data
    glGenBuffers(1, &q->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, q->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(q->vertices), q->vertices, GL_DYNAMIC_DRAW); // Note: Using dynamic draw as vertices will be updated frequently
    
    // Generate EBO and bind data
    glGenBuffers(1, &q->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(q->indices), q->indices, GL_STATIC_DRAW); // Note: Using static draw as indices will not be updated frequently
    
    // Set vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // POSITION
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float))); // Color
    glEnableVertexAttribArray(2);

    
    // Unbind VAO
    glBindVertexArray(0);
}

// Function to render the quad batch
void render_quad_batch(const QuadBatch *q) {
    // Bind the VAO
    glBindVertexArray(q->VAO);

    // Set the shader program
    glUseProgram(q->shaderProgram);

    // Bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, q->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(q->vertices), q->vertices); // Update vertex buffer data

    // NOTE(mo): Not passing the view matrix in loop causes it to miss behave as if its reset every frame.
    glUniformMatrix4fv(glGetUniformLocation(q->shaderProgram, "view"), 1, GL_TRUE, (const float*)q->viewMatrix.data);

    // Bind the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->EBO);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Bind texture to a texture unit (e.g., GL_TEXTURE0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, q->globalTexId);


    // Set the texture uniform in the shader to correspond to the texture unit
    glUseProgram(q->shaderProgram);
    glUniform1i(glGetUniformLocation(q->shaderProgram, "texture1"), 0); // 0 corresponds to GL_TEXTURE0


    // Draw the quads
    glDrawElements(GL_TRIANGLES, q->quad_count * 6, GL_UNSIGNED_INT, 0);

    // Unbind the VAO
    glBindVertexArray(0);
}