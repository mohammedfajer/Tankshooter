#include "text.h"


bool text_init()
{
  if (TTF_Init() != 0)
  {
    printf("TTF_Init Error: %s\n", TTF_GetError());
    return false;
  }
  return true;
}

bool text_setup_font(Text *text, const char *path, int fontSize)
{
  text->fontSize = fontSize;
  text->font = TTF_OpenFont(path, text->fontSize);
  if (!text->font)
  {
    printf("error openning font file: %s\n", SDL_GetError());
    return false;
  }
  return true;
}


// TODO(mo): its not working at the momenet.
void text_draw_gl(Text *text, GLuint shaderProgram,
               int xPos, int yPos,
               const char *txt,
               bool center,
               SDL_Color color, Mat4 *proj)
{
    // Create surface
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(text->font, txt, color);
    if (!surfaceMessage)
    {
        printf("Failed to create a surface for text message: %s\n", TTF_GetError());
        exit(0);
        

    }

    printf("Surface created: %dx%d, %d bpp\n", surfaceMessage->w, surfaceMessage->h, surfaceMessage->format->BytesPerPixel);

    // Generate an OpenGL texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Specify the texture format and upload the pixel data
    GLenum textureFormat;
    // if (surfaceMessage->format->BytesPerPixel == 4) // 32-bit
    // {
    //     if (surfaceMessage->format->Rmask == 0x000000ff)
    //         textureFormat = GL_RGBA;
    //     else
    //         textureFormat = GL_BGRA;
    // }
    // else // 24-bit
    // {
    //     if (surfaceMessage->format->Rmask == 0x000000ff)
    //         textureFormat = GL_RGB;
    //     else
    //         textureFormat = GL_BGR;
    // }

    switch (surfaceMessage->format->BytesPerPixel)
    {
        case 1: // Grayscale
            textureFormat = GL_RED;
            break;
        case 3: // RGB
            textureFormat = GL_RGB;
            break;
        case 4: // RGBA
            textureFormat = GL_RGBA;
            break;
        default:
            printf("Unsupported pixel format\n");
            return;
    }

    printf("Uploading texture data: format=%d, width=%d, height=%d\n", textureFormat, surfaceMessage->w, surfaceMessage->h);

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, surfaceMessage->w, surfaceMessage->h,
                 0, textureFormat, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
				 
	// Query the texture to get the width and height of the rendered text
    int textWidth = surfaceMessage->w;
    int textHeight = surfaceMessage->h;

    // Free the surface since we now have the texture
    SDL_FreeSurface(surfaceMessage);

  

    // Calculate the position based on the center flag
    int drawX = xPos;
    int drawY = yPos;

    if (center)
    {
        drawX -= textWidth / 2;
        drawY -= textHeight / 2;
    }

    // Set up vertex data and attribute pointers
    GLfloat vertices[] = {
        drawX,         drawY,                   0.0f, 1.0f,
        drawX,         drawY + textHeight,      0.0f, 0.0f,
        drawX + textWidth, drawY + textHeight,  1.0f, 0.0f,
        drawX + textWidth, drawY,               1.0f, 1.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Render the text
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);


    // Set the texture uniform in the shader
    glUniform1i(glGetUniformLocation(shaderProgram, "textTexture"), 0);

    // Set projection and model matrices
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

    Mat4 model = createTranslationMatrix2((Vec3){drawX, drawY, 0.0f});
   
	
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat *)(*proj).data); 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *) model.data) ;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Clean up
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texture);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        printf("OpenGL error: %d\n", error);
    }
}

void text_draw(Text *text,
              SDL_Renderer *renderer,
              int xPos, int yPos,
              const char* txt,
              bool center,
              SDL_Color color)
{
    // Create surface
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(text->font, txt, color);
    if (!surfaceMessage)
    {
        printf("Failed to create a surface for text message: %s\n", TTF_GetError());
        return; // Early return on error
    }
    // Create texture from surface
    SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_FreeSurface(surfaceMessage); 
    if (!Message)
    {
        printf("Failed to create a texture for text message: %s\n", SDL_GetError());
        return; // Early return on error
    }

    // Query the texture to get the width and height of the rendered text
    int textWidth, textHeight;
    SDL_QueryTexture(Message, NULL, NULL, &textWidth, &textHeight);

    // Calculate the position based on the center flag
    int drawX = xPos;
    int drawY = yPos;

    if (center)
    {
        drawX -= textWidth / 2;
        drawY -= textHeight / 2;
    }

    SDL_Rect MessageRect = {drawX, drawY, textWidth, textHeight};

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, Message, NULL, &MessageRect);
    // Destroy the texture to free up memory
    SDL_DestroyTexture(Message);
}