
// Quad Draw Test
GLuint gSpriteQuadVBO, gSpriteQuadVAO, gSpriteQuadEBO;
GLuint gSpriteShaderProgram;


const char* g_sprite_vertex_shader_source = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
  
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    
    "void main() {\n"
    "    gl_Position =  projection * view * model * vec4(position, 1.0);\n"
    
    "}\n";

const char* g_sprite_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    
    "void main() {\n"
    "    FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n" // Orange color
    "}\n";



Mat4 gProjectionMatrix;
Mat4 gViewMatrix;

Vec3 gSpritePos;

void init_sprite_quad() {
    // Define vertices for a simple quad
 

    float vertices[] = {
        -0.5f, 0.5f, 0.0f,  // top right
        0.5f, 0.5f, 0.0f,  // bottom right
        0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  -0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 2,   // first triangle
        3, 0, 2    // second triangle
    };  


    // Create Vertex Array Object and Vertex Buffer Object
    glGenVertexArrays(1, &gSpriteQuadVAO);
    glGenBuffers(1, &gSpriteQuadVBO);
    glGenBuffers(1, &gSpriteQuadEBO); // Add EBO for indices

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s)
    glBindVertexArray(gSpriteQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gSpriteQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSpriteQuadEBO); // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Pass indices data

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);
}



void render_sprite_quad(float posX, float posY, float scaleX, float scaleY, float rotationZ) {
    // Bind VAO and draw quad
    glUseProgram(gSpriteShaderProgram);
    glBindVertexArray(gSpriteQuadVAO);
    // Set transformation matrices
    Mat4 translationMatrix = createTranslationMatrix2((Vec3) {posX, posY, 0.0});
    Mat4 scaleMatrix = createScaleMatrix((Vec3){scaleX, scaleY, 1.0});
    Mat4 rotationMatrix = createRotationZMatrix(rotationZ);
    // Calculate model matrix by multiplying translation, rotation, and scale matrices
    Mat4 modelMatrix = createIdentityMatrix();
    
    modelMatrix = multiplyMatrices(modelMatrix, scaleMatrix);
    modelMatrix = multiplyMatrices(modelMatrix, rotationMatrix);
    modelMatrix = multiplyMatrices(modelMatrix, translationMatrix);

    // ((T * R) * S)
    int MatrixID = glGetUniformLocation(gSpriteShaderProgram, "model");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix.data[0][0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
