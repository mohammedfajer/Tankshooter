#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  GLuint textureId;   // ID handle of the glyph texture
  Vec2i size;          // size of glyph
  Vec2i bearing;       // offset from baseline to left/top of glyph
  GLuint advance;     // offset to advance to next glyph
} Character;


typedef struct {
  char key;
  Character value;
} KeyValuePair;


// sturct .. when empy was causing warning later. since struct Node was not defined prior to that line.
typedef struct Node {
  KeyValuePair data;
  struct Node *next;
} Node;

typedef struct {
  GLuint VAO;
  GLuint VBO;
  GLuint shaderProgram;
  Mat4 projection;
  int fontSize;
} TextRenderState;  



const char* gVertexShaderSourceText = "#version 330 core\n"
    "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
    "out vec2 TexCoords;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
    "    TexCoords = vertex.zw;\n"
    "}\n";

// const char* gFragmentShaderSourceText = "#version 330 core\n"
//     "in vec2 TexCoords;\n"
//     "out vec4 color;\n"
//     "uniform sampler2D text;\n"
//     "uniform vec3 textColor;\n"
//     "void main()\n"
//     "{    \n"
//     "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, vec2()).r);\n"
//     "    color = vec4(textColor, 1.0) * sampled;\n"
//     "}\n";

const char* gFragmentShaderSourceText = "#version 330 core\n"
    "in vec2 TexCoords;\n"
    "out vec4 color;\n"
    "uniform sampler2D text;\n"
    "uniform vec3 textColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, vec2(TexCoords.x, 1.0 - TexCoords.y)).r);\n"
    "    color = vec4(textColor, 1.0) * sampled;\n"
    "}\n";



Node *gCharacterMap = NULL;



void free_map(Node *head)
{
  while(head != NULL)
  {
    Node *temp = head;
    head = head->next;
    free(temp);
  }
}


Node *create_node(char key, Character value)
{
  Node *newNode = (Node *) malloc(sizeof(Node));
  if (newNode == NULL) {
    printf("Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  newNode->data.key = key;
  newNode->data.value = value;
  newNode->next = NULL;
  return newNode;
}

void insert(Node **head, char key, Character value)
{
  if(*head == NULL)
  {
    *head = create_node(key, value);
    return;
  }

  Node *current = *head;
  while(current->next != NULL)
  {
    current = current->next;
  }

  current->next = create_node(key, value);
}

Character *get(Node *head, char key)
{
  Node *current = head;
  while(current != NULL)
  {
    if(current->data.key == key)
    {
      return &(current->data.value);
    }
    current = current->next;
  }

  return NULL;
}

int calculate_text_width(const char* text, int font_size) {
    int total_width = 0;
    for (const char* c = text; *c != '\0'; c++) {
        Character* ch = get(gCharacterMap, *c);
        if (ch != NULL) {
            total_width += ch->size.x;
        }
    }
    return total_width ;
}

int calculate_text_height(const char *text, int font_size) {
    int max_height = 0;
    for (const char* c = text; *c != '\0'; c++) {
        Character* ch = get(gCharacterMap, *c);
        if (ch != NULL) {
            if (ch->size.y > max_height) {
                max_height = ch->size.y;
            }
        }
    }
    return max_height ;
}

void init_freetype(TextRenderState* s, const char *path, int fontSize)
{
  // Setup Shaders
  s->fontSize = fontSize;
  s->shaderProgram = createShaderProgram(gVertexShaderSourceText, gFragmentShaderSourceText);
  s->projection = createOrthographicMatrix(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1.0f, 100.0f);
    
  FT_Library ft;
  // Initialize FreeType library
  if (FT_Init_FreeType(&ft)) {
      printf("ERROR::FREETYPE: Could not init FreeType Library\n");
      exit(-1);
  }

  FT_Face face;
  // Load font as face
  FT_Error error = FT_New_Face(ft, path, 0, &face);
  if (error) {
      printf("ERROR::FREETYPE: Failed to load font: %s\n", FT_Error_String(error));
        FT_Done_FreeType(ft); // Don't forget to clean up FreeType resources
      exit(-1);
  }
  else
  {
    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);
  
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            .textureId = texture,
            .size = (Vec2i){(int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows},
            .bearing = (Vec2i){(int)face->glyph->bitmap_left, (int)face->glyph->bitmap_top },
            .advance = (unsigned int)(face->glyph->advance.x)
        };

       
        insert(&gCharacterMap, c, character);
        
      }
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);   


    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &s->VAO);
    glGenBuffers(1, &s->VBO);
    glBindVertexArray(s->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, s->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 
}


void render_text(TextRenderState *s, const char *text, float x, float y, float scale, SDL_Color color, bool center) {
    // Activate corresponding render state
    glUseProgram(s->shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(s->shaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)s->projection.data);

    glUniform3f(glGetUniformLocation(s->shaderProgram, "textColor"), color.r, color.g, color.b);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(s->VAO);

    // Calculate text width and height based on the font size
    int text_width = calculate_text_width(text, s->fontSize);
    int text_height = calculate_text_height(text, s->fontSize);

    // Adjust the position if centering is required
    if (center) {
        x -= text_width / 2;
        y -= text_height / 2;
    }

    // Iterate through all characters
    for (const char* c = text; *c != '\0'; c++) {
        Character* ch = get(gCharacterMap, *c);
        if (ch == NULL) {
            // Character not found in the map
            continue;
        }

        float xpos = x + ch->bearing.x * scale;
        // float ypos = y - (ch->size.y - ch->bearing.y) * scale;
float ypos = y - ch->bearing.y * scale;
        float w = ch->size.x * scale;
        float h = ch->size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch->textureId);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, s->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch->advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}